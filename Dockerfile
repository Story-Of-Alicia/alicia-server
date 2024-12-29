FROM ubuntu:latest AS builder
RUN apt-get update
RUN apt-get install -y --no-install-recommends build-essential git cmake libboost-dev libpq-dev
RUN apt-get clean

WORKDIR /builder/alicia-server
ADD . .
RUN git submodule update --init --recursive

RUN mkdir build && \
    cd build && \
    cmake .. && \
    cmake --build . --parallel

FROM ubuntu:latest
WORKDIR /server
COPY --from=builder /builder/alicia-server/build/alicia-server .

WORKDIR /game/resources
COPY --from=builder /builder/alicia-server/resources/* .
WORKDIR /game

VOLUME [ "/game" ]
ENTRYPOINT ["/server/alicia-server"]
