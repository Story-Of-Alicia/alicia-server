# syntax=docker/dockerfile:1
ARG BUILDER_REPO_PATH=/builder/alicia-server

FROM alpine:3 AS builder
RUN apk add --no-cache boost-dev build-base cmake git gnu-libiconv
RUN ls -a /usr/lib

ARG BUILDER_REPO_PATH
ARG BUILD_TYPE=RelWithDebInfo

WORKDIR ${BUILDER_REPO_PATH}
COPY . .
RUN git submodule update --init --recursive

RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_TESTS=True . -B ./build
RUN cmake --build ./build --parallel
RUN cmake --install ./build --prefix .

FROM alpine:3
ARG BUILDER_REPO_PATH

LABEL author="Serkan Sahin" maintainer="dev@storyofalicia.com"
LABEL org.opencontainers.image.source=https://github.com/Story-Of-Alicia/alicia-server
LABEL org.opencontainers.image.description="Dedicated server implementation for the Alicia game series"

RUN apk add --no-cache libstdc++ gnu-libiconv
WORKDIR /opt/alicia-server
COPY --from=builder ${BUILDER_REPO_PATH}/dist .

ENTRYPOINT ["/opt/alicia-server/alicia-server"]
