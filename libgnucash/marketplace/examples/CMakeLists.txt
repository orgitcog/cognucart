# CMakeLists.txt for marketplace examples

# Demo application
add_executable(marketplace-demo
    marketplace_demo.cpp
)

target_link_libraries(marketplace-demo
    gnc-marketplace
    gnc-opencog
    Threads::Threads
)

# Installation is optional for examples
# install(TARGETS marketplace-demo DESTINATION ${CMAKE_INSTALL_BINDIR})
