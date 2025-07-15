include(CTest)

add_executable(protocol_test_magic)
target_sources(protocol_test_magic PRIVATE
        src/protocol/TestMagic.cpp)
target_link_libraries(protocol_test_magic
        PRIVATE project-properties alicia-libserver)

add_executable(util_test_stream)
target_sources(util_test_stream PRIVATE
        src/util/TestStream.cpp)
target_link_libraries(util_test_stream
        PRIVATE project-properties alicia-libserver)

add_executable(util_test_scheduler)
target_sources(util_test_scheduler PRIVATE
        src/util/TestScheduler.cpp)
target_link_libraries(util_test_scheduler
        PRIVATE project-properties alicia-libserver)

add_executable(util_test_locale)
target_sources(util_test_locale PRIVATE
        src/util/TestLocale.cpp)
target_link_libraries(util_test_locale
        PRIVATE project-properties alicia-libserver)

add_test(NAME ProtocolTestMagic COMMAND protocol_test_magic)
add_test(NAME UtilTestStream COMMAND util_test_stream)
add_test(NAME UtilTestScheduler COMMAND util_test_scheduler)
add_test(NAME UtilTestLocale COMMAND util_test_locale)

