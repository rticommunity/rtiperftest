# (c) 2022 Copyright, Real-Time Innovations, Inc.  All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or revisions thereof,
# must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.

FROM repo.rti.com:443/build-docker-prod-hq/x64linux4gcc7.3.0:20240429-bd4d2d5112ecb4a74dad52076d414f2403b26336

ENV PATH="/opt/tools/cmake/3.12/bin:/opt/tools/python/3.8/bin:${PATH}"

# Create a virtual environment writable for everyone. From now on, every python
# command will be run using this virtual environment. This includes pip
ENV VIRTUAL_ENV=/opt/venv
RUN rm -rf $VIRTUAL_ENV
RUN /opt/tools/python/3.8/bin/python3 -m venv $VIRTUAL_ENV && chmod a+w -R $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

ENV ENABLE_IPV6=1

RUN mkdir -p /tmp
ARG PIP_VERSION="20.3"
RUN python -m pip install --upgrade pip==${PIP_VERSION} --no-cache-dir
RUN python -m pip install -U pip

ARG TIMESTAMP
ARG AUTOMATION_VERSION="rti-doozer"
RUN echo "${TIMESTAMP}"
RUN echo "${AUTOMATION_VERSION}"
RUN pip install "${AUTOMATION_VERSION}" --index-url https://repo.rti.com/artifactory/api/pypi/pypi/simple --no-cache-dir

RUN chmod a+w -R $VIRTUAL_ENV

# SEC-2640: Install socat so that we can run the Security Plugins' test for
# OCSP through HTTPS. This is a short-term solution until we have BUILD-4757.
# We can use apt tool because the image for this Dockerfile is based on ubuntu.
#
# RUN apt install socat
#
# The version from apt is too old and was compiled with an OpenSSL ciphersuite
# that is not supported by the OCSP server. We need to compile socat from
# source and rely on the newer OpenSSL version that comes with this image.
RUN wget https://repo.rti.com/artifactory/thirdparty-local/rti/socat/1.8.0.2/socat-1.8.0.2.tar.bz2 -P /tmp/socat \
    && [ $(sha256sum /tmp/socat/socat-1.8.0.2.tar.bz2 | awk '{print $1}') = "adc07a9c2723527cf6568d2fb96559794cf9c254a4bc2edd36f7f3789e9f7625" ] \
    && tar -xjvf /tmp/socat/socat-1.8.0.2.tar.bz2 -C /tmp/socat \
    && (cd /tmp/socat/socat-1.8.0.2 \
    && ./configure \
    && make \
    && make install)

ENTRYPOINT ["/entrypoint.sh"]
