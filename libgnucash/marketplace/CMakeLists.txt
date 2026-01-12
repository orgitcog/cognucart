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

add_library(gnc-marketplace ${marketplace_SOURCES} ${marketplace_HEADERS})

target_link_libraries(gnc-marketplace
    gnc-engine
    gnc-opencog
)

target_include_directories(gnc-marketplace PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS gnc-marketplace DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${marketplace_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gnucash/marketplace)

# Tests
if(WITH_TESTS)
    add_subdirectory(test)
endif()
