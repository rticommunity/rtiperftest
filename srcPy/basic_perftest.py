# (c) 2005-2023 Copyright, Real-Time Innovations, Inc. All rights reserved.
# Subject to Eclipse Public License v1.0; see LICENSE.md for details.

# This is a simple performance test for RTI Connext DDS for the Python API.
# It is based on the C++ version of the test, and it is intended to be used
# experimentally at this point. It is not intended to be a full-featured
# performance test, but just cover the basic use-cases.

import time
from dataclasses import field
from typing import Sequence

import rti.connextdds as dds
import rti.types as idl
import argparse
import signal
import threading
import sys
import os
from datetime import datetime

BURST_SIZE = 100
EXIT_SIZE = 10
BYTES_OVERHEAD = 28
LATENCY_COUNT = 100000

class ShellColors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def error_print(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

class Perftest:

    def handler(self, signum, frame):
        error_print(f"\n{ShellColors.FAIL}[Ctrl-c was pressed. Finishing Test]{ShellColors.ENDC}", flush=True)
        sys.exit(-1)

    def __init__(self,
                sequence_size: int,
                bound: idl.annotations.BoundAnnotation,
                nic: str,
                be: bool,
                timeout_us: int,
                batch_size: int,
                latency_test: bool,
                no_print: bool,
                no_headers: bool,
                domain: int,
                transport: str):

        signal.signal(signal.SIGINT, self.handler)

        # Eventually we might want to move this to a generated code in a different file
        # (Generated by RTIDDSGEN :D).
        @idl.struct(
            type_annotations = [idl.final],
            member_annotations = {'bin_data': [idl.bound(sequence_size)]}
        )
        class PerftestType:
            seq_num: idl.int32 = 0
            timestamp_nsec: idl.uint64 = 0
            latency_ping: idl.int32 = 0
            bin_data: Sequence[idl.int8] = field(default_factory = idl.array_factory(idl.int8))

        # Object level variables for Connext DDS.
        self.participant = None
        self.publisher = None
        self.subscriber = None

        # Object level variables taken from input parameters.
        self.type = PerftestType
        self.sample = PerftestType(0,0,0,dds.Int8Seq(sequence_size))
        self.nic = nic
        self.timeout_us = timeout_us
        self.batch_size = batch_size
        self.latency_test = latency_test
        self.no_print = no_print
        self.no_headers = no_headers
        self.domain = domain
        self.be = be

        type_support = idl.get_type_support(self.type)

        # This value will be overwriten in the case of a reader (since we receive the size)
        self.sample_size = type_support.get_serialized_sample_size(self.sample)

        # Get base folder for Perftest:
        python_script_folder = os.path.dirname(os.path.realpath(__file__))
        base_folder = os.path.abspath(os.path.join(python_script_folder, ".."))

        self.qos_provider_base = dds.QosProvider(str(os.path.join(base_folder, "perftest_qos_profiles.xml")), "PerftestQosLibrary::BaseProfileQos")
        self.qos_provider_ping = dds.QosProvider(str(os.path.join(base_folder, "perftest_qos_profiles.xml")), "PerftestQosLibrary::ThroughputQos")
        self.qos_provider_pong = dds.QosProvider(str(os.path.join(base_folder, "perftest_qos_profiles.xml")), "PerftestQosLibrary::LatencyQos")

        # In case we want to print the type.
        # error_print("Data type is:\n\n", type_support.dynamic_type)

    def configure_participant_qos(self):
        participant_qos = self.qos_provider_base.participant_qos
        participant_qos.transport_builtin.mask = dds.TransportBuiltinMask.UDPv4
        if self.nic is not None:
            participant_qos.property['dds.transport.UDPv4.builtin.parent.allow_interfaces_list'] = self.nic
        return participant_qos

    def configure_writer_qos(self, topic_name):

        writer_qos = None

        if topic_name == "Ping":

            writer_qos = self.qos_provider_ping.datawriter_qos

            if self.latency_test:
                self.batch_size = 0

            if self.batch_size != 0:
                writer_qos.batch.enable = True
                writer_qos.batch.max_data_bytes = self.batch_size

        else: # topic_name = "Pong"

            writer_qos = self.qos_provider_pong.datawriter_qos

        if writer_qos is not None:

            if self.be:
                writer_qos.reliability = writer_qos.reliability.best_effort

        return writer_qos


    def configure_reader_qos(self, topic_name):

        reader_qos = None

        if topic_name == "Ping":

            reader_qos = self.qos_provider_ping.datareader_qos

        else: # topic_name = "Pong"

            reader_qos = self.qos_provider_pong.datareader_qos

        if reader_qos is not None:

            if self.be:
                reader_qos.reliability = reader_qos.reliability.best_effort

        return reader_qos

    def initialize_dds_entities(self):

        # Certain QoS need to be modified in the participant_qos
        participant_qos = self.configure_participant_qos()
        self.participant = dds.DomainParticipant(self.domain, qos=participant_qos)

        # Nothing needs to be configured for the pub and sub entities.
        self.publisher = dds.Publisher(self.participant, qos=self.qos_provider_base.publisher_qos)
        self.subscriber = dds.Subscriber(self.participant, qos=self.qos_provider_base.subscriber_qos)

    def run_publisher(self):

        class LatencyInfo():
            def __init__(self):
                self.latency_array = []
                self.sum = 0.0
                self.average = 0.0
                self.max_value = 0
                self.min_value = 999999999999
                self.count = 0
                self.last_latency_sample = 0
                self.std = 0
                self.sample_size = -1
                self.sorted = False

            def update(self, new_value):
                if new_value <= 0:
                    error_print(f"{new_value} IS NEGATIVE!!!!")
                self.latency_array.append(new_value)
                self.sum = self.sum + new_value
                self.count = self.count + 1
                self.average = sum(self.latency_array) / self.count
                if new_value > self.max_value: self.max_value = new_value
                if new_value < self.min_value: self.min_value = new_value
                self.last_latency_sample = new_value

            def get_percentile(self, percentile):
                if not self.sorted:
                    self.latency_array.sort()
                index = int(len(self.latency_array) * percentile / 100)
                if index > len(self.latency_array) - 1:
                    error_print(f"[WARNING]: length latency array is: {len(self.latency_array)}, index is {index}.")
                    index = len(self.latency_array) -1
                return self.latency_array[index]


        class PongDataListener(dds.NoOpDataReaderListener):
            def __init__(self, sample_size, latency_test: bool, latency_lock, latency_info: LatencyInfo, no_print: bool):
                super().__init__()
                self.sample_size = sample_size
                self.latency_test = latency_test
                self.latency_lock = latency_lock
                self.latency_info = latency_info
                self.discard_data_value = True
                self.exit_burst = False
                self.no_print = no_print

            def on_data_available(self, reader):
                samples = reader.take_data()
                for sample in samples:
                    if self.latency_test:
                        try:
                            self.latency_lock.release()
                        except Exception:
                            pass
                    latency_us = (time.perf_counter_ns() - sample.timestamp_nsec) / (2 * 1000)
                    if not self.discard_data_value: # and not self.exit_burst:
                        latency_info.update(latency_us)
                        if not self.no_print:
                            print('{:>19d},{:>13.0f},{:>9.0f},{:>9.1f},{:>9.0f},{:>9.0f}'.format(
                                self.sample_size,
                                latency_info.last_latency_sample,
                                latency_info.average,
                                latency_info.std,
                                latency_info.min_value,
                                latency_info.max_value))

            def discard_data(self, value):
                self.discard_data_value = value


        error_print(f"{ShellColors.BOLD}Perftest (Python) -- {ShellColors.HEADER}PUBLISHER{ShellColors.ENDC}\n")

        # This initializes participant, Publisher and Subscriber
        self.initialize_dds_entities()

        latency_lock = threading.Lock()
        latency_info = LatencyInfo()

        if self.latency_test:
            error_print("-- Latency Test")

        error_print(f"-- Data size: {self.sample_size}")
        error_print(f"-- Batch Size: {self.batch_size}")
        error_print(f"-- Best Effort: {self.be}")

        # This is the reader used to send all the Throughput data.
        writer_qos = self.configure_writer_qos("Ping")
        ping_writer = dds.DataWriter(self.publisher,
                                    dds.Topic(self.participant, 'Ping', self.type),
                                    qos=writer_qos)

        # This is the reader used to receive the Pong messages (latency Samples).
        pong_reader_listener = PongDataListener(self.sample_size, self.latency_test, latency_lock, latency_info, self.no_print)
        reader_qos = self.configure_reader_qos("Pong")
        pong_reader = dds.DataReader(self.subscriber,
                                    dds.Topic(self.participant, 'Pong', self.type),
                                    reader_qos,
                                    pong_reader_listener,
                                    dds.StatusMask.DATA_AVAILABLE,)

        error_print("\nWaiting for subscribers... ", end="" , flush=True)
        while len(ping_writer.matched_subscriptions) == 0 and len(pong_reader.matched_publications) == 0:
            time.sleep(0.5)
        error_print(f"{ShellColors.BOLD}{ShellColors.OKGREEN}[OK]{ShellColors.ENDC}")


        # Initial Burst, these samples will not be taken into account:
        error_print("Sending initial Burst... ", end="" , flush=True)
        count = 0
        while count < BURST_SIZE:
            self.sample.seq_num = -1
            self.sample.latency_ping = 1
            if self.latency_test:
                latency_lock.acquire()
            ping_writer.write(self.sample)
            ping_writer.flush()
            count += 1
        error_print(f"{ShellColors.BOLD}{ShellColors.OKGREEN}[OK]{ShellColors.ENDC}\n")
        time.sleep(1)
        pong_reader_listener.discard_data(False)

        error_print("Starting test...")
        if not self.no_headers and not self.no_print:
            print("Sample Size (Bytes), Latency (us), Avg (μs), Std (μs), Min (μs), Max (μs)")

        count = 0
        total_time_us = 0.0
        current_time = 1.0 * time.time_ns()
        sample = self.sample
        latency_test = self.latency_test
        while total_time_us < self.timeout_us:
            sample.seq_num = count
            if latency_test:
                latency_lock.acquire()
                sample.latency_ping = 1
            else:
                if count % LATENCY_COUNT == 0:
                    sample.latency_ping = 1
                else:
                    sample.latency_ping = 0
            sample.timestamp_nsec = time.perf_counter_ns()
            ping_writer.write(sample)
            count += 1
            total_time_us += 1.0 * (time.time_ns() - current_time) / 1000
            current_time = time.time_ns()

        pong_reader_listener.discard_data(True)

        error_print("\n")
        if not self.no_headers:
            print("Sample Size (Bytes), Ave (μs), Std (μs), Min (μs), Max (μs), 50% (μs), 90% (μs), 99% (μs), 99.99% (μs), 99.9999% (μs)")


        print('{:>19d},{:>9.0f},{:>9.1f},{:>9.0f},{:>9.0f},{:>9.0f},{:>9.0f},{:>9.0f},{:>12.0f},{:>14.0f}'.format(
                                self.sample_size,
                                latency_info.average,
                                latency_info.std,
                                latency_info.min_value,
                                latency_info.max_value,
                                latency_info.get_percentile(50),
                                latency_info.get_percentile(90),
                                latency_info.get_percentile(99),
                                latency_info.get_percentile(99.99),
                                latency_info.get_percentile(99.9999)
                                ))

        # Sending exit burst
        error_print("\nSending exit Burst... ", end="" , flush=True)
        count = 0
        while count < EXIT_SIZE:
            self.sample.seq_num = -2
            self.latency_test = False
            self.sample.latency_ping = 1
            ping_writer.write(self.sample)
            ping_writer.flush()
            time.sleep(0.1)
            count += 1
        error_print(f"{ShellColors.BOLD}{ShellColors.OKGREEN}[OK]{ShellColors.ENDC}\n")

        error_print(f"{ShellColors.BOLD}{ShellColors.OKGREEN}[Finishing Test]{ShellColors.ENDC}\n")

    def get_mbps(self, count, elapsed_time):
        mbps = 8 * (self.sample_size * count / elapsed_time * 1e9) / 1e6
        return mbps

    def run_subscriber(self):

        class PingDataListener(dds.NoOpDataReaderListener):
            def __init__(self, start_test_lock, pong_writer):
                super().__init__()
                self.count = 0
                self.time = 0
                self.data_size = -1
                self.test_started = False
                self.test_finished = False
                self.pong_writer = pong_writer

                self.start_test_lock = start_test_lock
                self.start_test_lock.acquire()

                self.previous_sequence_num = 0
                self.lost_samples = 0


            def on_data_available(self, reader):
                samples = reader.take_data()
                for sample in samples:
                    sample_seq_number = sample.seq_num
                    if sample_seq_number >= 0:
                        if not self.test_started:
                            self.test_started = True
                            self.start_test_lock.release()
                        self.count += 1
                        if sample_seq_number - self.previous_sequence_num -1 > 0:
                            self.lost_samples += sample_seq_number - self.previous_sequence_num
                        self.previous_sequence_num = sample_seq_number

                    elif sample_seq_number == -2:
                        self.test_finished = True
                    elif sample_seq_number == -1:
                        self.data_size = len(sample.bin_data) + BYTES_OVERHEAD

                    if sample.latency_ping == 1:
                        self.pong_writer.write(sample)

        error_print(f"{ShellColors.BOLD}Perftest (Python) -- {ShellColors.HEADER}SUBSCRIBER{ShellColors.ENDC}\n")


        # This initializes participant, Publisher and Subscriber
        self.initialize_dds_entities()

        # We need this lock to know when the test starts:
        start_test = threading.Lock()

        # Pong to answer the Pong samples (latency Samples)
        writer_qos = self.configure_writer_qos("Pong")
        pong_writer = dds.DataWriter(self.publisher,
                                dds.Topic(self.participant, 'Pong', self.type),
                                qos=writer_qos)

        # Reader to get the Ping data
        ping_reader_listener = PingDataListener(start_test, pong_writer)
        reader_qos = self.configure_reader_qos("Ping")
        ping_reader = dds.DataReader(self.subscriber,
                                dds.Topic(self.participant, 'Ping', self.type),
                                reader_qos,
                                ping_reader_listener,
                                dds.StatusMask.DATA_AVAILABLE)

        # Discovery Phase, we need the Ping and Pong channel to be ready before
        # starting.
        error_print("Waiting for publishers... ", end="" , flush=True)
        while len(ping_reader.matched_publications) == 0 and len(pong_writer.matched_subscriptions) == 0:
            time.sleep(0.5)
        error_print(f"{ShellColors.BOLD}{ShellColors.OKGREEN}[OK]{ShellColors.ENDC}")


        # This is the initial Burst, we do this to ensure we have already reserved
        # memory for incoming samples and that we have exercised all the paths.
        # It is considered done when the Listener says so.
        error_print("Receiving initial Burst... ", end="" , flush=True)
        start_test.acquire()
        error_print(f"{ShellColors.BOLD}{ShellColors.OKGREEN}[OK]{ShellColors.ENDC}\n")

        self.sample_size = ping_reader_listener.data_size

        error_print("Waiting for data...")
        initial_time = time.time_ns()
        current_time = initial_time
        last_period_time = initial_time
        last_sample_count = ping_reader_listener.count

        if not self.no_headers and not self.no_print:
            print("Length (Bytes), Total Samples,  Samples/s, Avg Samples/s,     Mbps,  Avg Mbps, Lost Samples, Lost Samples (%)")

        while not ping_reader_listener.test_finished:
            time.sleep(1)
            current_time = time.time_ns()
            samples_per_second = ping_reader_listener.count - last_sample_count
            interval_mbps = self.get_mbps(samples_per_second, current_time - last_period_time)
            last_sample_count = ping_reader_listener.count
            last_period_time = current_time
            avg_samples = ping_reader_listener.count * 1e9 / (current_time - initial_time)
            avg_mbps = self.get_mbps(ping_reader_listener.count, current_time - initial_time)

            if not self.no_print:
                print('{:>14d},{:>14},{:>11},{:>14.0f},{:>9.1f},{:>10.1f},{:>14},{:>16.2f}'.format(self.sample_size, last_sample_count, samples_per_second, avg_samples, interval_mbps, avg_mbps, ping_reader_listener.lost_samples, 0.0))

        error_print("\n")
        if not self.no_headers:
            print("Sample Size (Bytes), Total Samples, Avg Samples/s,    Avg Mbps, Lost Samples, Lost Samples (%)")
        print('{:>19},{:>14},{:>14.0f},{:>12.1f},{:>13},{:>17.2f}'.format(self.sample_size, ping_reader_listener.count, avg_samples, avg_mbps, ping_reader_listener.lost_samples, 0))
        error_print(f"{ShellColors.BOLD}{ShellColors.OKGREEN}[Finishing Test]{ShellColors.ENDC}")



def main():
    # Parse sub or pub argument:
    class CaseInsensitivePartialMatchAction(argparse.Action):
        def __call__(self, parser, namespace, values, option_string=None):
            setattr(namespace, self.dest, values.lower())

    parser = argparse.ArgumentParser()
    parser.register('action', 'partial_match', CaseInsensitivePartialMatchAction)
    parser.add_argument('-sub', action='store_true', help="Run subscriber")
    parser.add_argument('-pub', action='store_true', help="Run publisher")
    parser.add_argument('-bestEffort', action='store_true', help="Best Effort Mode")
    parser.add_argument('-dataLen', type=int, default=100, help="Size of the type")
    parser.add_argument('-executionTime', type=int, default=20, help="Test timeout in seconds")
    parser.add_argument('-nic', type=str, default=None, help="Allow interface list value")
    parser.add_argument('-bound', type=int, default=1000, help="Sequence bound (0 for unbounded)")
    parser.add_argument('-batchSize', type=int, default=8192, help="Sequence bound (0 for unbounded)")
    parser.add_argument('-latencyTest', action='store_true', help="Run a latency test")
    parser.add_argument('-noPrintIntervals', action='store_true', help="Do not print anything until the end of the test")
    parser.add_argument('-noOutputHeaders', action='store_true', help="Do not print headers")
    parser.add_argument('-domain', type=int, default=0, help="DDS Domain")
    parser.add_argument('-transport', type=str, default=None, help="Transport to use")
    args = parser.parse_args()

    # Run publisher or subscriber:
    perftest = Perftest(
        sequence_size=int(args.dataLen - BYTES_OVERHEAD),
        bound=idl.bound(args.bound) if args.bound != 0 else idl.unbounded,
        nic=args.nic,
        be = args.bestEffort,
        timeout_us=args.executionTime * 1000 * 1000,
        batch_size = args.batchSize,
        latency_test = args.latencyTest,
        no_print = args.noPrintIntervals,
        no_headers = args.noOutputHeaders,
        domain = args.domain,
        transport = args.transport)
    if args.pub:
        perftest.run_publisher()
    else:
        # By default we create a subscriber.
        perftest.run_subscriber()

if __name__ == '__main__':
    main()