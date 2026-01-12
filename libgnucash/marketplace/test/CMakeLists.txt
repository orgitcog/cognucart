# CMakeLists.txt for marketplace tests

set(test_marketplace_SOURCES
    gtest-marketplace.cpp
)

add_executable(gtest-marketplace ${test_marketplace_SOURCES})

target_link_libraries(gtest-marketplace
    gnc-marketplace
    gnc-engine
    gnc-opencog
    gtest
    gtest_main
    pthread
)

add_test(NAME gtest-marketplace COMMAND gtest-marketplace)
