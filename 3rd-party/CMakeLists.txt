add_subdirectory(json)

set(YAML_CPP_BUILD_TOOLS OFF)
set(YAML_CPP_BUILD_TESTS OFF)
add_subdirectory(yaml-cpp)

add_subdirectory(spdlog)
set(ZLIB_BUILD_TESTING OFF)
set(ZLIB_BUILD_SHARED OFF)
set(ZLIB_BUILD_MINIZIP OFF)

add_subdirectory(zlib)

add_subdirectory(tinyxml2)

set(BUILD_SHARED_LIBS OFF)
set(PostgreSQL_ADDITIONAL_VERSIONS "17")
find_package(PostgreSQL REQUIRED)
add_subdirectory(libpqxx)
