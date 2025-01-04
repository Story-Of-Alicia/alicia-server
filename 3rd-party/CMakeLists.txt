add_subdirectory(spdlog)
add_subdirectory(json)

set(BUILD_SHARED_LIBS OFF)
set(PostgreSQL_ADDITIONAL_VERSIONS "17")
add_subdirectory(libpqxx)