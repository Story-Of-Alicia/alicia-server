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

add_executable(test_executor)
target_sources(test_executor PRIVATE
        src/TestExecutor.cpp)
target_link_libraries(test_executor
        PRIVATE project-properties alicia-libserver)

add_executable(test_executor_timeout)
target_sources(test_executor_timeout PRIVATE
        src/TestExecutorTimeout.cpp)
target_link_libraries(test_executor_timeout
        PRIVATE project-properties alicia-libserver)

add_test(NAME TestMagic COMMAND test_magic)

add_test(NAME TestBuffers COMMAND test_buffers)

add_test(NAME TestExecutor COMMAND test_executor)

add_test(NAME TestExecutorTimeout COMMAND test_executor_timeout)
set_property(TEST TestExecutorTimeout PROPERTY WILL_FAIL TRUE)
