# syntax=docker/dockerfile:1
FROM alpine:3 AS build

# Setup the build environment
RUN apk add --no-cache cmake make git build-base boost-dev icu-dev

ARG SERVER_BUILD_TYPE=RelWithDebInfo

# Build the server
WORKDIR /build/alicia-server

# Add linker search path to /usr/local as that is where iconv is installed at.
ENV LD_LIBRARY_PATH=/usr/local/lib

# Prepare the source
COPY . .
RUN git submodule update --init --recursive
RUN cmake -DCMAKE_BUILD_TYPE=${SERVER_BUILD_TYPE} -DBUILD_TESTS=False . -B ./build
RUN cmake --build ./build --parallel

# Install the binary
RUN cmake --install ./build --prefix /usr/local

# Copy the resources
RUN mkdir /var/lib/alicia-server/
RUN cp -r ./resources/* /var/lib/alicia-server/

FROM alpine:3

LABEL author="Serkan Sahin" maintainer="dev@storyofalicia.com"
LABEL org.opencontainers.image.source=https://github.com/Story-Of-Alicia/alicia-server
LABEL org.opencontainers.image.description="Dedicated server implementation for the Alicia game series"

# Setup the runtime environent
RUN apk add --no-cache libstdc++ icu icu-data-full

WORKDIR /opt/alicia-server

COPY --from=build /usr/local /usr/local
COPY --from=build /var/lib/alicia-server/ /var/lib/alicia-server/

ENTRYPOINT ["/usr/local/bin/alicia-server", "/var/lib/alicia-server"]