FROM ubuntu:18.04
LABEL "com.rti.vendor"="Real-Time Innovations" \
    version="3.1.0" \
    maintainer="jmorales@rti.com" \
    description="Run RTI Perftest from a Docker container"

ADD https://github.com/rticommunity/rtiperftest/releases/download/3.1.0/rti_perftest-3.0.1_connext_dds_pro_6.0.1_x64Linux.tar.gz /tmp
RUN tar -xvzf tmp/rti_perftest-3.1.0_connext_dds_pro_6.0.0_x64Linux.tar.gz
WORKDIR rti_perftest-3.1.0_x64Linux3gcc4.8.2
RUN ln -s bin/x64Linux3gcc4.8.2/release/perftest_cpp perftest
ENTRYPOINT ["./perftest"]

# How to use?
# 1. Build container image:
#       $ docker build -t rti/perftest .
# 2. Run docker container
#       $ docker run rti/perftest <perftest arguments>
