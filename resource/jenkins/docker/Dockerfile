ARG PLATFORM=amd64
ARG TAG=7.3.0-20240812
ARG IMAGE_USER=jenkins
ARG IMAGE_GROUP=jenkins
ARG USER_ID=789
ARG GROUP_ID=789


FROM --platform=${PLATFORM} repo.rti.com/connext-containers/connext-sdk:${TAG}

ARG IMAGE_USER
ARG IMAGE_GROUP
ARG USER_ID
ARG GROUP_ID

RUN groupadd -g ${GROUP_ID} ${IMAGE_GROUP} && \
    useradd -u ${USER_ID} -g ${GROUP_ID} -ms /bin/bash ${IMAGE_USER}

RUN chown -R ${IMAGE_USER}:${IMAGE_GROUP} /root/.dotnet

USER ${IMAGE_USER}