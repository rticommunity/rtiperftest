# (c) 2022 Copyright, Real-Time Innovations, Inc.  All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or revisions thereof,
# must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.

FROM repo.rti.com:443/build-docker-prod-hq/x86_64leelfgcc7.3.0-linux4:20231019-f3411dc14fdd31ab4c6747793231c81803377a58

RUN apt-get install -y net-tools

ENV PATH="/opt/tools/cmake/3.18/bin:/opt/tools/python/3.8/bin:${PATH}"

# Create a virtual environment writable for everyone. From now on, every python
# command will be run using this virtual environment. This includes pip
ENV VIRTUAL_ENV=/opt/venv
RUN /opt/tools/python/3.8/bin/python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
RUN python3 -m pip install pip==23.2.1

ARG TIMESTAMP
RUN echo "${TIMESTAMP}" 

RUN chmod a+w -R $VIRTUAL_ENV

RUN mkdir -p /opt/toolchains/$CONNEXT_ARCH
RUN touch /opt/toolchains/$CONNEXT_ARCH/environment.sh
RUN chmod a+rx /opt/toolchains/$CONNEXT_ARCH/environment.sh

ENTRYPOINT ["/entrypoint.sh"]