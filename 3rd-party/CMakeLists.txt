add_subdirectory(json)
add_subdirectory(yaml-cpp)
add_subdirectory(spdlog)
add_subdirectory(zlib)

set(BUILD_SHARED_LIBS OFF)
set(PostgreSQL_ADDITIONAL_VERSIONS "17")

#add_subdirectory(libpqxx)