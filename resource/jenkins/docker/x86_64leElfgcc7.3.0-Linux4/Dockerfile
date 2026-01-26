# (c) 2022 Copyright, Real-Time Innovations, Inc.  All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or revisions thereof,
# must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.

FROM repo.rti.com:443/build-docker-prod-hq/x64linux4gcc7.3.0:20231019-f3411dc14fdd31ab4c6747793231c81803377a58

ENV PATH="/opt/tools/cmake/3.18/bin:${PATH}"

RUN apt-get update \
    && apt-get install -y libxrender1 libfontconfig1

RUN ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime
ENV TZ=America/Los_Angeles
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends tzdata
RUN apt-get install -y net-tools
RUN apt-get clean

RUN mkdir -p /opt/tools/bk/1.0.0
RUN curl https://repo.rti.com:443/artifactory/thirdparty-local/rti/bk-cli/1.0.0/linux64CERT/bk-show-unknown.bin -o /opt/tools/bk/1.0.0/bk-show-unknown.bin
RUN chmod a+x /opt/tools/bk/1.0.0/bk-show-unknown.bin

RUN curl https://repo.rti.com:443/artifactory/thirdparty-local/rti/openjdk/17.0.10/OpenJDK17U-jre_x64_linux_hotspot_17.0.10_7.tar.gz | tar -zxv -C /opt/
ENV JAVA_HOME="/opt/jdk-17.0.10+7-jre"
ENV JREHOME=$JAVA_HOME

ENV VIRTUAL_ENV=/opt/venv
RUN /opt/tools/python/3.8/bin/python3 -m venv $VIRTUAL_ENV && chmod a+w -R $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
RUN python3 -m pip install pip==23.2.1

ARG TIMESTAMP
RUN echo "${TIMESTAMP}" 

RUN chmod -R a+w $VIRTUAL_ENV

ENV CONNEXT_ARCH "x86_64leElfgcc7.3.0-Linux4"
RUN mkdir -p /opt/toolchains/$CONNEXT_ARCH
RUN touch /opt/toolchains/$CONNEXT_ARCH/environment.sh
RUN chmod a+rx /opt/toolchains/$CONNEXT_ARCH/environment.sh

ENTRYPOINT ["/entrypoint.sh"]