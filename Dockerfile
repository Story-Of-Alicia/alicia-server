# syntax=docker/dockerfile:1
FROM gcc:15 AS build

# Setup the build environment
RUN apt-get update -y
RUN apt-get install git cmake libboost-dev libicu-dev -y --no-install-recommends

ARG SERVER_BUILD_TYPE=RelWithDebInfo

# Build the server
WORKDIR /build/alicia-server

# Prepare the source
COPY . .

# Use
ENV PATH="/usr/local/bin:${PATH}"

RUN git init
RUN git submodule update --init --recursive

RUN cmake -DCMAKE_BUILD_TYPE=${SERVER_BUILD_TYPE} -DBUILD_TESTS=False . -B ./build
RUN cmake --build ./build --parallel 8

# Install the binary
RUN cmake --install ./build --prefix /usr/local

# Copy the resources
RUN mkdir /var/lib/alicia-server/
RUN cp -r ./resources/* /var/lib/alicia-server/

FROM debian:forky-slim

LABEL author="Story of Alicia Developers" maintainer="dev@storyofalicia.com"
LABEL org.opencontainers.image.source=https://github.com/Story-Of-Alicia/alicia-server
LABEL org.opencontainers.image.description="Dedicated server implementation for the Alicia game series"

# Setup the runtime environent
RUN apt-get update -y
RUN apt-get install libicu76 -y --no-install-recommends

WORKDIR /opt/alicia-server

COPY --from=build /usr/local /usr/local
COPY --from=build /var/lib/alicia-server/ /var/lib/alicia-server/

ENTRYPOINT ["/usr/local/bin/alicia-server", "/var/lib/alicia-server"]