ARG BUILDER_REPO_PATH=/builder/alicia-server

FROM ubuntu:latest AS builder
RUN apt-get update
RUN apt-get install -y --no-install-recommends build-essential git cmake libboost-dev libpq-dev
RUN apt-get clean

ARG BUILDER_REPO_PATH
ARG BUILD_TYPE=Release

WORKDIR ${BUILDER_REPO_PATH}
ADD . .
RUN git submodule update --init --recursive

RUN cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -S ${BUILDER_REPO_PATH} -B ${BUILDER_REPO_PATH}/build && \
    cmake --build ${BUILDER_REPO_PATH}/build --parallel

FROM ubuntu:latest
ARG BUILDER_REPO_PATH
WORKDIR /server
COPY --from=builder ${BUILDER_REPO_PATH}/build/alicia-server .

WORKDIR /game/resources
COPY --from=builder ${BUILDER_REPO_PATH}/resources/* .
WORKDIR /game

VOLUME [ "/game" ]
ENTRYPOINT ["/server/alicia-server"]
