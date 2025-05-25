# syntax=docker/dockerfile:1
ARG BUILDER_REPO_PATH=/builder/alicia-server

FROM alpine:3 AS builder
RUN apk add --no-cache boost-dev build-base cmake git

ARG BUILDER_REPO_PATH
ARG BUILD_TYPE=Release

WORKDIR ${BUILDER_REPO_PATH}
COPY . .
RUN git submodule update --init --recursive

RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_TESTS=False . -B ./build
RUN cmake --build ./build --parallel
RUN cmake --install ./build --prefix .

FROM alpine:3
ARG BUILDER_REPO_PATH

LABEL author="Serkan Sahin" maintainer="dev@storyofalicia.com"

RUN apk add --no-cache libstdc++
RUN addgroup -S alicia && adduser -S alicia -G alicia
USER alicia:alicia

WORKDIR /opt/alicia-server
COPY --from=builder --chown=alicia ${BUILDER_REPO_PATH}/dist .

VOLUME [ "/opt/alicia-server" ]
ENTRYPOINT ["/opt/alicia-server/alicia-server"]
