# syntax=docker/dockerfile:1
ARG BUILDER_REPO_PATH=/builder/alicia-server

FROM ubuntu:latest AS builder
RUN apt-get update
RUN apt-get install -y --no-install-recommends build-essential git cmake libboost-dev libpq-dev ca-certificates
RUN apt-get clean

ARG BUILDER_REPO_PATH
ARG BUILD_TYPE=Release

WORKDIR ${BUILDER_REPO_PATH}
COPY . .
RUN git submodule update --init --recursive

RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_TESTS=False . -B ./build
RUN cmake --build ./build --parallel
RUN cmake --install ./build --prefix .

FROM ubuntu:latest
ARG BUILDER_REPO_PATH

LABEL author="Serkan Sahin" maintainer="dev@storyofalicia.com"

RUN groupadd -r alicia && useradd --no-log-init -r -g alicia alicia
USER alicia

WORKDIR /opt/alicia-server  
COPY --from=builder --chown=alicia ${BUILDER_REPO_PATH}/dist .

EXPOSE 10030/tcp 10031/tcp 10032/tcp
VOLUME [ "/opt/alicia-server" ]  
ENTRYPOINT ["/opt/alicia-server/alicia-server"]  
