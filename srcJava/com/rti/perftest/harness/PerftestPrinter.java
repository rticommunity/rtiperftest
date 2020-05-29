package com.rti.perftest.harness;

enum PerftestOutputFormat
    {
        LEGACY,
        JSON,
        CSV
    }

public class PerftestPrinter {
    private int _dataLength;
        private boolean _printHeaders;
        private boolean _showCPU;
        private boolean _printIntervals;
        private boolean _printSummaryHeaders;
        private boolean _isJsonInitialized;
        private boolean _controlJsonIntervals;
        private PerftestOutputFormat _outputFormat;
        public final String PERFT_TIME_UNIT = "us";

        public PerftestPrinter()
        {
            _dataLength = 100;
            _printSummaryHeaders = true;
            _outputFormat = PerftestOutputFormat.CSV;
        }
        public void initialize(
                boolean printIntervals,
                String outputFormat,
                boolean printHeaders,
                boolean showCpu)
        {
            _printIntervals = printIntervals;
            _printHeaders = printHeaders;
            _showCPU = showCpu;
            if (outputFormat.equals("csv")) {
                _outputFormat = PerftestOutputFormat.CSV;
            } else if (outputFormat.equals("json")) {
                _outputFormat = PerftestOutputFormat.JSON;
                _isJsonInitialized = false;
            } else if (outputFormat.equals("legacy")) {
                _outputFormat = PerftestOutputFormat.LEGACY;
            }
        }
        public void set_data_length(int dataLength)
        {
            _dataLength = dataLength;
        }
        public void set_header_printed(boolean printHeaders)
        {
            _printHeaders = printHeaders;
        }
        public void print_latency_header()
        {
            switch (_outputFormat) {
                case CSV:
                    if(_printHeaders && _printIntervals) {
                        System.out.printf("\nIntervals One-way Latency for %1d " +
                                      "Bytes:\n", _dataLength);
                        System.out.printf("Length (Bytes)" +
                                      ", Latency (" + PERFT_TIME_UNIT +
                                      "), Ave (" + PERFT_TIME_UNIT +
                                      "), Std (" + PERFT_TIME_UNIT +
                                      "), Min (" + PERFT_TIME_UNIT +
                                      "), Max (" + PERFT_TIME_UNIT +
                                      ")");
                        if (_showCPU) {
                            System.out.print(", CPU (%%)");
                        }
                    }
                    System.out.print("\n");
                    System.out.flush();
                    break;
                case JSON:
                    if (_isJsonInitialized) {
                        System.out.print(",\n\t\t{\n");
                    } else {
                        _isJsonInitialized = true;
                    }
                    System.out.printf("\t\t\t\"length\":%1d,\n", _dataLength);
                    if (_printIntervals) {
                        System.out.print("\t\t\t\"intervals\":[\n");
                        _controlJsonIntervals = true;
                    }
                    System.out.flush();
                    break;
                case LEGACY:
                    if(_printHeaders && _printIntervals) {
                        System.out.printf("\n\n********** New data length is %1d\n",
                                      _dataLength);
                        System.out.flush();
                    }
                    break;
            }
        }
        public void print_throughput_header()
        {
            switch (_outputFormat) {
                case CSV:
                    if(_printHeaders && _printIntervals) {
                        System.out.printf("\nIntervals Throughput for %1d " +
                                      "Bytes:\n", _dataLength);
                        System.out.printf("Length (Bytes), Total Samples,  " +
                                      "Samples/s, Ave Samples/s,     " +
                                      "Mbps,  Ave Mbps" +
                                      ", Lost Samples, Lost Samples (%%)");
                        if (_showCPU) {
                            System.out.printf(", CPU (%%)");
                        }
                    }
                    System.out.printf("\n");
                    System.out.flush();
                    break;
                case JSON:
                    if (_isJsonInitialized) {
                        System.out.printf(",\n\t\t{\n");
                    } else {
                        _isJsonInitialized = true;
                    }
                    System.out.printf("\t\t\t\"length\":%1d,\n", _dataLength);
                    if (_printIntervals) {
                        System.out.printf("\t\t\t\"intervals\":[\n");
                        _controlJsonIntervals = true;
                    }
                    System.out.flush();
                    break;
                case LEGACY:
                    if(_printHeaders && _printIntervals) {
                        System.out.printf("\n\n********** New data length is %1d\n",
                                      _dataLength);
                        System.out.flush();
                    }
                    break;
            }
        }
        public void print_latency_interval(
            int latency,
            double latencyAve,
            double latencyStd,
            int latencyMin,
            int latencyMax,
            double outputCpu)
        {
            switch (_outputFormat) {
                case CSV:
                    System.out.printf("%1$14d,%2$13d,%3$9.0f,%4$9.1f,%5$9d" +
                                      ",%6$9d",
                                      _dataLength,
                                      latency,
                                      latencyAve,
                                      latencyStd,
                                      latencyMin,
                                      latencyMax);
                    if (_showCPU) {
                        System.out.printf(",%1$8.2f", outputCpu);
                    }
                    System.out.printf("\n");
                    System.out.flush();
                    break;
                case JSON:
                    if (_controlJsonIntervals) {
                        _controlJsonIntervals = false;
                    } else {
                        System.out.printf(",");
                    }
                    System.out.printf("\n\t\t\t\t{\n" +
                                  "\t\t\t\t\t\"latency\": %1d,\n" +
                                  "\t\t\t\t\t\"latency_ave\": %2$1.0f,\n" +
                                  "\t\t\t\t\t\"latency_std\": %3$1.1f,\n" +
                                  "\t\t\t\t\t\"latency_min\": %4$1d,\n" +
                                  "\t\t\t\t\t\"latency_max\": %5$1d",
                                  latency,
                                  latencyAve,
                                  latencyStd,
                                  latencyMin,
                                  latencyMax);
                    if (_showCPU) {
                        System.out.printf(",\n\t\t\t\t\t\"cpu\": %1$1.2f",
                                      outputCpu);
                    }
                    System.out.printf("\n\t\t\t\t}");
                    break;
                case LEGACY:
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
                    break;
            }
        }
        public void print_latency_summary(
            double latencyAve,
            double latencyStd,
            long latencyMin,
            long latencyMax,
            int[] latencyHistory,
            long count,
            double outputCpu)
        {
            switch (_outputFormat) {
                case CSV:
                    if (_printSummaryHeaders && _printHeaders) {
                        if (!_printIntervals && _printSummaryHeaders) {
                            _printSummaryHeaders = _printIntervals;
                        }
                        System.out.printf("\nOne-way Latency Summary:\n");
                        System.out.printf("Length (Bytes)" +
                                          ", Ave (" + PERFT_TIME_UNIT +
                                          "), Std (" + PERFT_TIME_UNIT +
                                          "), Min (" + PERFT_TIME_UNIT +
                                          "), Max (" + PERFT_TIME_UNIT +
                                          "), 50%% (" + PERFT_TIME_UNIT +
                                          "), 90%% (" + PERFT_TIME_UNIT +
                                          "), 99%% (" + PERFT_TIME_UNIT +
                                          "), 99.99%% (" + PERFT_TIME_UNIT +
                                          "), 99.9999%% (" +PERFT_TIME_UNIT +
                                          ")");
                        if (_showCPU) {
                            System.out.printf(", CPU (%%)");
                        }
                        System.out.printf("\n");
                    }
                    System.out.printf("%1$14d,%2$9.0f,%3$9.1f,%4$9d,%5$9d," +
                                  "%6$9d,%7$9d,%8$9d,%9$12d,%10$14d",
                                  _dataLength,
                                  latencyAve,
                                  latencyStd,
                                  latencyMin,
                                  latencyMax,
                                  latencyHistory[(int) (count * 50 / 100)],
                                  latencyHistory[(int) (count * 90 / 100)],
                                  latencyHistory[(int) (count * 99 / 100)],
                                  latencyHistory[(int) (count *
                                        (9999 / 10000))],
                                  latencyHistory[(int) (count *
                                        (999999 / 1000000))]);
                    if (_showCPU) {
                        System.out.printf(",%1$8.2f", outputCpu);
                    }
                    System.out.printf("\n");
                    System.out.flush();
                    break;
                case JSON:
                    if (_printIntervals) {
                        System.out.printf("\n\t\t\t],\n");
                    }
                    System.out.printf("\t\t\t\"summary\":{\n" +
                                  "\t\t\t\t\"latency_ave\": %1$1.1f,\n" +
                                  "\t\t\t\t\"latency_std\": %2$1.1f,\n" +
                                  "\t\t\t\t\"latency_min\": %3$1d,\n" +
                                  "\t\t\t\t\"latency_max\": %4$1d,\n" +
                                  "\t\t\t\t\"latency_50\": %5$1d,\n" +
                                  "\t\t\t\t\"latency_90\": %6$1d,\n" +
                                  "\t\t\t\t\"latency_99\": %7$1d,\n" +
                                  "\t\t\t\t\"latency_99.99\": %8$1d,\n" +
                                  "\t\t\t\t\"latency_99.9999\": %9$1d",
                                  latencyAve,
                                  latencyStd,
                                  latencyMin,
                                  latencyMax,
                                  latencyHistory[(int) (count * 50 / 100)],
                                  latencyHistory[(int) (count * 90 / 100)],
                                  latencyHistory[(int) (count * 99 / 100)],
                                  latencyHistory[(int) (count *
                                        (9999 / 10000))],
                                  latencyHistory[(int) (count *
                                        (999999 / 1000000))]);
                    if (_showCPU) {
                        System.out.printf(",\n\t\t\t\t\"cpu\": %1$1.1f", outputCpu);
                    }
                    System.out.printf("\n\t\t\t}\n\t\t}");
                    break;
                case LEGACY:
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
                    break;
            }
        }
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
            switch (_outputFormat) {
                case CSV:
                    System.out.printf("%1$14d,%2$14d,%3$11d,%4$14.0f,%5$9.1f"+
                                      ",%6$10.1f,%7$13d,%8$17.2f",
                                      _dataLength,
                                      lastMsgs,
                                      mps,
                                      mpsAve,
                                      bps * 8.0 / 1000.0 / 1000.0,
                                      bpsAve * 8.0 / 1000.0 / 1000.0,
                                      missingPackets,
                                      missingPacketsPercent);
                    if (_showCPU) {
                        System.out.printf(",%1$8.2f", outputCpu);
                    }
                    System.out.printf("\n");
                    System.out.flush();
                    break;
                case JSON:
                    if (_controlJsonIntervals) {
                        _controlJsonIntervals = false;
                    } else {
                        System.out.printf(",");
                    }
                    System.out.printf("\n\t\t\t\t{\n" +
                                  "\t\t\t\t\t\"length\": %1d,\n" +
                                  "\t\t\t\t\t\"packets\": %2d,\n" +
                                  "\t\t\t\t\t\"packets/s\": %3d,\n" +
                                  "\t\t\t\t\t\"packets/s_ave\": %4$1.2f,\n" +
                                  "\t\t\t\t\t\"mbps\": %5$1.1f,\n" +
                                  "\t\t\t\t\t\"mbps_ave\": %6$1.1f,\n" +
                                  "\t\t\t\t\t\"lost\": %7$1d,\n" +
                                  "\t\t\t\t\t\"lost_percent\": %8$1.2f",
                                  _dataLength,
                                  lastMsgs,
                                  mps,
                                  mpsAve,
                                  bps * 8.0 / 1000.0 / 1000.0,
                                  bpsAve * 8.0 / 1000.0 / 1000.0,
                                  missingPackets,
                                  missingPacketsPercent);
                    if (_showCPU) {
                        System.out.printf(",\n\t\t\t\t\t\"cpu\": %1$1.2f",
                                      outputCpu);
                    }
                    System.out.printf("\n\t\t\t\t}");
                    break;
                case LEGACY:
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
                    break;
            }
        }
        public void print_throughput_summary(
            int length,
            long intervalPacketsReceived,
            long intervalTime,
            long intervalBytesReceived,
            long intervalMissingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            switch (_outputFormat) {
                case CSV:
                    if (_printSummaryHeaders && _printHeaders) {
                        if (!_printIntervals && _printSummaryHeaders) {
                            _printSummaryHeaders = _printIntervals;
                        }
                        System.out.printf("\nThroughput Summary:\n");
                        System.out.printf("Length (Bytes), Total Samples," +
                                      " Ave Samples/s,    Ave Mbps, " +
                                      "Lost Samples, Lost Samples (%%)");
                        if (_showCPU) {
                            System.out.printf(", CPU (%%)");
                        }
                        System.out.printf("\n");
                    }
                    System.out.printf("%1$14d,%2$14d,%3$14.0f,%4$12.1f," +
                                  "%5$13d,%6$17.2f",
                                  length,
                                  intervalPacketsReceived,
                                  intervalPacketsReceived * 1000000.0
                                         / intervalTime,
                                  intervalBytesReceived * 1000000.0
                                         / intervalTime * 8.0 / 1000.0 / 1000.0,
                                  intervalMissingPackets,
                                  missingPacketsPercent);
                    if (_showCPU) {
                        System.out.printf(",%1$8.2f", outputCpu);
                    }
                    System.out.printf("\n");
                    System.out.flush();
                    break;
                case JSON:
                    if (_printIntervals) {
                        System.out.printf("\n\t\t\t],\n");
                    }
                    System.out.printf("\t\t\t\"summary\":{\n" +
                                  "\t\t\t\t\"packets\": %1$1d,\n" +
                                  "\t\t\t\t\"packets/sAve\": %2$1.0f,\n" +
                                  "\t\t\t\t\"mbpsAve\": %3$1.1f,\n" +
                                  "\t\t\t\t\"lost\": %4$1d,\n" +
                                  "\t\t\t\t\"lostPercent\": %5$1.2f",
                                  intervalPacketsReceived,
                                  intervalPacketsReceived * 1000000.0
                                         / intervalTime,
                                  intervalBytesReceived * 1000000.0
                                         / intervalTime * 8.0 / 1000.0 / 1000.0,
                                  intervalMissingPackets,
                                  missingPacketsPercent);
                    if (_showCPU) {
                        System.out.printf(",\n\t\t\t\t\"cpu\": %1$1.2f", outputCpu);
                    }
                    System.out.printf("\n\t\t\t}\n\t\t}");
                    break;
                case LEGACY:
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
                    break;
            }
        }
        public void print_initial_output()
        {
            if (_outputFormat.equals(PerftestOutputFormat.JSON)) {
                System.out.printf("{\"perftest\":\n\t[\n\t\t{\n");
            }
        }
        public void print_final_output()
        {
            if (_outputFormat.equals(PerftestOutputFormat.JSON)) {
                System.out.printf("\n\t]\n}\n");
            }
        }
}
