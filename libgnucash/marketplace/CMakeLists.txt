# CMakeLists.txt for GnuCash Marketplace Module

set(marketplace_SOURCES
    marketplace_engine.cpp
    product.cpp
    order.cpp
    storefront.cpp
    inventory.cpp
    customer.cpp
)

set(marketplace_HEADERS
    marketplace_engine.hpp
    product.hpp
    order.hpp
    storefront.hpp
    inventory.hpp
    customer.hpp
    marketplace_types.hpp
)

# Create the marketplace library as a static library
add_library(gnc-marketplace STATIC
    ${marketplace_SOURCES}
)

# Include directories
target_include_directories(gnc-marketplace
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# C++17 is required for marketplace features
target_compile_features(gnc-marketplace PUBLIC cxx_std_17)

# Link with OpenCog for AI features
target_link_libraries(gnc-marketplace
    PUBLIC
        gnc-opencog
)

# Thread support is required
find_package(Threads REQUIRED)
target_link_libraries(gnc-marketplace
    PUBLIC
        Threads::Threads
)

# Installation
install(TARGETS gnc-marketplace
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install headers
install(FILES ${marketplace_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gnucash/marketplace
)

# Tests
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_subdirectory(test)
endif()

# Examples
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()
