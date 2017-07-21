FROM ubuntu:16.04
LABEL "com.rti.vendor"="Real-Time Innovations" \
    version="2.2" \
    maintainer="israel@rti.com" \
    description="Run RTI Perftest from a Docker container"

ADD https://github.com/rticommunity/rtiperftest/releases/download/v2.2/rti_perftest-2.2_x64Linux.tar.gz /tmp
RUN tar -xvzf /tmp/rti_perftest-2.2_x64Linux.tar.gz
WORKDIR rti_perftest-2.2_x64Linux3gcc4.8.2
RUN ln -s bin/x64Linux3gcc4.8.2/release/perftest_cpp perftest
ENTRYPOINT ["./perftest"]
CMD [""]
