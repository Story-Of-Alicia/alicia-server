cmake_minimum_required(VERSION 3.20)

project(alicia-server
        VERSION 1.0.0)

option(BUILD_TESTS "Build tests" ON)

find_package(Boost 1.80.0 MODULE)

add_subdirectory(3rd-party SYSTEM)

# Project properties interface library
add_library(project-properties INTERFACE)
target_compile_features(project-properties
        INTERFACE cxx_std_23)

# Alicia server library
add_library(alicia-libserver STATIC
        include/libserver/data/DataDefinitions.hpp
        include/libserver/data/DataStorage.hpp
        src/libserver/Util.cpp
        src/libserver/Stream.cpp
        src/libserver/base/Server.cpp
        src/libserver/command/CommandProtocol.cpp
        src/libserver/command/CommandServer.cpp
        src/libserver/command/proto/DataDefines.cpp
        src/libserver/command/proto/LobbyMessageDefines.cpp
        src/libserver/command/proto/RaceMessageDefines.cpp
        src/libserver/command/proto/RanchMessageDefines.cpp
        src/libserver/data/DataDirector.cpp
        #src/libserver/data/pq/PqDataSource.cpp
        #src/libserver/data/pq/SqlUtils.cpp
        src/libserver/data/file/FileDataSource.cpp
        src/libserver/data/file/FileDataSource.cpp)

target_include_directories(alicia-libserver
        PUBLIC include/)
target_link_libraries(alicia-libserver
        PUBLIC project-properties Boost::headers spdlog::spdlog nlohmann_json::nlohmann_json)

# Alicia server executable
add_executable(alicia-server
        src/server/main.cpp
        src/server/Settings.cpp
        src/server/Scheduler.cpp
        src/server/tracker/WorldTracker.cpp
        src/server/lobby/LobbyDirector.cpp
        src/server/lobby/LoginHandler.cpp
        src/server/race/RaceDirector.cpp
        src/server/ranch/RanchDirector.cpp)

# Version variables
set(AliciaServerBuildName alpha)
set(AliciaServerBuildVersionMajor ${CMAKE_PROJECT_VERSION_MAJOR})
set(AliciaServerBuildVersionMinor ${CMAKE_PROJECT_VERSION_MINOR})
set(AliciaServerBuildVersion ${CMAKE_PROJECT_VERSION}-${AliciaServerBuildName})

# Generated version header.
configure_file(
        include/server/Version.hpp.in
        generated/Version.hpp)

target_include_directories(alicia-server PRIVATE
        include/)
target_link_libraries(alicia-server
        PRIVATE project-properties alicia-libserver)

target_include_directories(alicia-server PUBLIC
        "${PROJECT_BINARY_DIR}/generated")

if (${MSVC})
    target_compile_options(alicia-server
            PRIVATE /utf-8)
endif ()

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Adding -fexperimental-library for Clang compiler")
    target_compile_options(alicia-libserver
            PRIVATE -fexperimental-library)
    target_compile_options(alicia-server
            PRIVATE -fexperimental-library)
endif ()

add_custom_command(
        TARGET alicia-server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources)

install(TARGETS alicia-server DESTINATION dist)
install(DIRECTORY resources DESTINATION dist)  
