# syntax=docker/dockerfile:1
FROM alpine:3 AS build

# Setup the build environment
RUN apk add --no-cache build-base git cmake boost-dev icu-dev libpq-dev

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
RUN cmake --build ./build --parallel $(nproc)

# Install the binary
RUN cmake --install ./build --prefix /usr/local

# Copy the resources
RUN mkdir /var/lib/alicia-server/
RUN cp -r ./resources/* /var/lib/alicia-server/

FROM alpine:3

LABEL author="Story of Alicia Developers" maintainer="dev@storyofalicia.com"
LABEL org.opencontainers.image.source="https://github.com/Story-Of-Alicia/alicia-server"
LABEL org.opencontainers.image.description="Dedicated server implementation for the Alicia game series"

# Setup the runtime environent
RUN apk add --no-cache icu icu-data-full libpq

WORKDIR /opt/alicia-server

COPY --from=build /usr/local/bin/alicia-server /usr/local/bin/alicia-server
COPY --from=build /var/lib/alicia-server/ /var/lib/alicia-server/

ENTRYPOINT ["/usr/local/bin/alicia-server", "/var/lib/alicia-server"]