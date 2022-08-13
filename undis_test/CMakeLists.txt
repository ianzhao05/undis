include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
        undis_test
        utils.h
        kvstore_test.cpp
        serializer_test.cpp
        threadpool_test.cpp
        command_test.cpp)
target_link_libraries(
        undis_test
        undis_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(undis_test)
