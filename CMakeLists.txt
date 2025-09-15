cmake_minimum_required(VERSION 3.20)

project(alicia-server)

option(BUILD_TESTS "Build tests" ON)

find_package(Boost 1.80.0 MODULE)

add_subdirectory(3rd-party SYSTEM)

include(cmake/Versioning.cmake)
get_semver()

# Version variables
set(AliciaServerBuildVersion ${ServerSemver})

# Generated version header.
configure_file(
        include/server/Version.hpp.in
        generated/Version.hpp)

# Project properties interface library
add_library(project-properties INTERFACE)
target_compile_features(project-properties
        INTERFACE cxx_std_23)

# Platform properties interface library
add_library(platform-properties INTERFACE)
if (${MSVC})
    target_compile_definitions(platform-properties INTERFACE
            #_WIN32_WINNT=_WIN32_WINNT_WIN10
            NTDDI_VERSION=NTDDI_WIN10_19H1)
    target_link_libraries(platform-properties INTERFACE
            icu.lib)
    target_compile_options(platform-properties INTERFACE
            /utf-8)
else()
    target_link_libraries(platform-properties INTERFACE
            icuuc)
endif()

# alicia-libserver target
add_library(alicia-libserver STATIC
        src/libserver/data/DataDirector.cpp
        src/libserver/data/helper/ProtocolHelper.cpp
        src/libserver/data/file/FileDataSource.cpp
        #src/libserver/data/pq/PqDataSource.cpp
        src/libserver/network/Server.cpp
        src/libserver/network/chatter/proto/ChatterMessageDefinitions.cpp
        src/libserver/network/chatter/ChatterServer.cpp
        src/libserver/network/command/CommandProtocol.cpp
        src/libserver/network/command/CommandServer.cpp
        src/libserver/network/command/proto/CommonStructureDefinitions.cpp
        src/libserver/network/command/proto/LobbyMessageDefinitions.cpp
        src/libserver/network/command/proto/RaceMessageDefinitions.cpp
        src/libserver/network/command/proto/RanchMessageDefinitions.cpp
        src/libserver/registry/HorseRegistry.cpp
        src/libserver/registry/ItemRegistry.cpp
        src/libserver/registry/OtpRegistry.cpp
        src/libserver/registry/PetRegistry.cpp
        src/libserver/registry/RoomRegistry.cpp
        src/libserver/util/Locale.cpp
        src/libserver/util/Scheduler.cpp
        src/libserver/util/Stream.cpp
        src/libserver/util/Util.cpp)
target_include_directories(alicia-libserver PUBLIC
        include/)
target_link_libraries(alicia-libserver PUBLIC
        project-properties
        platform-properties
        spdlog::spdlog
        Boost::headers
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
        zlibstatic)

# alicia-server target
add_executable(alicia-server
        src/server/main.cpp
        src/server/ServerInstance.cpp
        src/server/Config.cpp
        src/server/lobby/LobbyDirector.cpp
        src/server/lobby/LoginHandler.cpp
        src/server/messenger/MessengerDirector.cpp
        src/server/race/RaceDirector.cpp
        src/server/ranch/RanchDirector.cpp
        src/server/system/ChatSystem.cpp
        src/server/system/InfractionSystem.cpp
        src/server/tracker/WorldTracker.cpp)
target_include_directories(alicia-server
        PRIVATE include/)
target_link_libraries(alicia-server PRIVATE
        project-properties
        alicia-libserver)
target_include_directories(alicia-server PUBLIC
        "${PROJECT_BINARY_DIR}/generated")

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
        ${CMAKE_CURRENT_BINARY_DIR})
install(TARGETS alicia-server)
