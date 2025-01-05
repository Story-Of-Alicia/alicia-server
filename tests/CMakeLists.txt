include(CTest)

add_executable(test_magic)
target_sources(test_magic PRIVATE
        src/TestMagic.cpp)
target_link_libraries(test_magic
        PRIVATE project-properties alicia-libserver)

add_executable(test_buffers)
target_sources(test_buffers PRIVATE
        src/TestBuffers.cpp)
target_link_libraries(test_buffers
        PRIVATE project-properties alicia-libserver)

add_test(NAME TestMagic COMMAND test_magic)
add_test(NAME TestBuffers COMMAND test_buffers)
