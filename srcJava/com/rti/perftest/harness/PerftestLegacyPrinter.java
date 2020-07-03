package com.rti.perftest.harness;


public class PerftestLegacyPrinter extends PerftestPrinter {

        @Override
        public void print_latency_header()
        {
            if(_printHeaders && _printIntervals) {
                System.out.printf("\n\n********** New data length is %1d\n",
                                  _dataLength);
                System.out.flush();
            }
        }

        @Override
        public void print_throughput_header()
        {
            if(_printHeaders && _printIntervals) {
                System.out.printf("\n\n********** New data length is %1d\n",
                                  _dataLength);
                System.out.flush();
            }
        }

        @Override
        public void print_latency_interval(
            int latency,
            double latencyAve,
            double latencyStd,
            int latencyMin,
            int latencyMax,
            double outputCpu)
        {
            System.out.printf("One way Latency: %1$6d "
                              + PERFT_TIME_UNIT +
                              " Ave %2$6.0f " + PERFT_TIME_UNIT +
                              " Std %3$6.1f " + PERFT_TIME_UNIT +
                              " Min %4$6d " + PERFT_TIME_UNIT +
                              " Max %5$6d " + PERFT_TIME_UNIT,
                              latency,
                              latencyAve,
                              latencyStd,
                              latencyMin,
                              latencyMax);
            if (_showCPU) {
                System.out.printf(" CPU %1$1.2f (%%)", outputCpu);
            }
            System.out.printf("\n");
            System.out.flush();
        }

        @Override
        public void print_latency_summary(
            double latencyAve,
            double latencyStd,
            long latencyMin,
            long latencyMax,
            int[] latencyHistory,
            long count,
            double outputCpu)
        {
            System.out.printf("Length: %1$5d" +
                              " Latency: Ave %2$6.0f " + PERFT_TIME_UNIT +
                              " Std %3$6.1f " + PERFT_TIME_UNIT +
                              " Min %4$6d " + PERFT_TIME_UNIT +
                              " Max %5$6d " + PERFT_TIME_UNIT +
                              " 50%% %6$6d " + PERFT_TIME_UNIT +
                              " 90%% %7$6d " + PERFT_TIME_UNIT +
                              " 99%% %8$6d " + PERFT_TIME_UNIT +
                              " 99.99%% %9$6d " + PERFT_TIME_UNIT +
                              " 99.9999%% %10$6d " + PERFT_TIME_UNIT,
                              _dataLength,
                              latencyAve,
                              latencyStd,
                              latencyMin,
                              latencyMax,
                              latencyHistory[(int) (count * 50 / 100)],
                              latencyHistory[(int) (count * 90 / 100)],
                              latencyHistory[(int) (count * 99 / 100)],
                              latencyHistory[(int) (count
                                      * (9999 / 10000))],
                              latencyHistory[(int) (count
                                      * (999999 / 1000000))]);
            if (_showCPU) {
                System.out.printf(" CPU %1$1.2f (%%)", outputCpu);
            }
            System.out.printf("\n");
            System.out.flush();
        }

        @Override
        public void print_throughput_interval(
            long lastMsgs,
            long mps,
            double mpsAve,
            long bps,
            double bpsAve,
            long missingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            System.out.printf("Packets: %1$8d  Packets/s: %2$7d  " +
                              "Packets/s(ave): %3$7.0f  Mbps: %4$7.1f" +
                              "  Mbps(ave): %5$7.1f  Lost: %6$5d (%7$1.2f (%%))",
                              lastMsgs,
                              mps,
                              mpsAve,
                              bps * 8.0 / 1000.0 / 1000.0,
                              bpsAve * 8.0 / 1000.0 / 1000.0,
                              missingPackets,
                              missingPacketsPercent);
            if (_showCPU) {
                System.out.printf(" CPU %1$1.2f (%%)", outputCpu);
            }
            System.out.printf("\n");
            System.out.flush();
        }

        @Override
        public void print_throughput_summary(
            int length,
            long intervalPacketsReceived,
            long intervalTime,
            long intervalBytesReceived,
            long intervalMissingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            System.out.printf("Length: %1$5d  Packets: %2$8d  " +
                              "Packets/s(ave): %3$7.0f  " +
                              "Mbps(ave): %4$7.1f  Lost: %5$5d (%6$1.2f%%)",
                              length,
                              intervalPacketsReceived,
                              intervalPacketsReceived * 1000000.0 /
                                  intervalTime,
                              intervalBytesReceived * 1000000.0 /
                                  intervalTime *8.0/1000.0/1000.0,
                              intervalMissingPackets,
                              missingPacketsPercent);
            if (_showCPU) {
                System.out.printf(" CPU %1$1.2f (%%)", outputCpu);
            }
            System.out.printf("\n");
            System.out.flush();
        }
}
