# Alicia Server
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/Story-Of-Alicia/alicia-server/docker-publish.yml)

Dedicated server software for Alicia game series.

## How to run
[Read more on how to run the server and connect the client.](HOWTORUN.md)

## Community
If you're a user please feel free to join [the official community of Story Of Alicia](https://discord.gg/storyofalicia) and follow our progress there. 
If you're a programmer and want to talk about the code [join the dedicated developer](https://discord.gg/bP4pNWvVHM) community on Discord.

## Requirements

- Git
- CMake
- A C++20 compatible compiler
- Boost
- libpq

# macOS

When building on Mac you'll also need GCC and the ICU library. The following instructions assume you're using [Homebrew](https://brew.sh/) as your package manager.

1. `brew install gcc`
2. `export CC=gcc-15 CXX=g++-15` (replace `15` with the version of GCC you installed)
3. `brew install icu4c`
4. `export CMAKE_PREFIX_PATH=/opt/homebrew/opt/icu4c@77`
4. `export LDFLAGS="-L/opt/homebrew/opt/icu4c@77"`


## Building

1. Clone the repository
```bash
git clone https://github.com/Story-Of-Alicia/alicia-server.git
```
2. Initialize the repository's submodules 
```bash
git submodule update --init --recursive
```
3. Create the build directory 
```bash
mkdir build; cd build
```
4. Generate the build files
```bash
cmake ..
```
5. Build the project
```bash
cmake --build .
```

After building, the executable `alicia-server` or `alicia-server.exe` will be present in the `build/` directory
