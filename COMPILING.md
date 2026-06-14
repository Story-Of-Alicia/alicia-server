# Compiling
## Requirements

- Git
- CMake
- A C++23 compatible compiler
- Boost
- libpq (PostgreSQL)

### On Windows

You can download PostgreSQL binaries as an archive and extract them to a folder and set `PostgreSQL_ROOT` to point to the extracted folder. This will let CMake know where to find the `/include` and `/lib` folders.

You'll also need to add the `/bin` folder to the `PATH` environment variable so that the `libpq.dll` can be found on runtime.

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


## Licensing
You agree to the license and you give up your individual rights to the code you contribute for the betterment of the community... right? Todo something more official.  
