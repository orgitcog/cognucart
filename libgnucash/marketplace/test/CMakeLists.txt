# CMakeLists.txt for marketplace tests

set(test_marketplace_SOURCES
    gtest-marketplace.cpp
)

# Check if GTest is available
find_package(GTest)

if(GTEST_FOUND)
    add_executable(gtest-marketplace ${test_marketplace_SOURCES})

    target_link_libraries(gtest-marketplace
        gnc-marketplace
        gnc-opencog
        GTest::GTest
        GTest::Main
        Threads::Threads
    )

    add_test(NAME gtest-marketplace COMMAND gtest-marketplace)
else()
    message(STATUS "GTest not found, skipping marketplace tests")
endif()
