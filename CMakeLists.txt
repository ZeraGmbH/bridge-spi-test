cmake_minimum_required(VERSION 3.1)

project(bridge-spi-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(BRIDGESPITEST_VERSION_MAJOR "0")
set(BRIDGESPITEST_VERSION_MINOR "2")
set(BRIDGESPITEST_VERSION_PATCH "0")
set(PROJECT_VERSION "${BRIDGESPITEST_VERSION_MAJOR}.${BRIDGESPITEST_VERSION_MINOR}.${BRIDGESPITEST_VERSION_PATCH}")

include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt5 COMPONENTS Core Network CONFIG REQUIRED)
find_package(remotecommon REQUIRED)
find_package(spidevice REQUIRED)
find_package(bridgefmtspihelper REQUIRED)

add_executable(bridge-spi-test
    main.cpp
)

target_link_libraries (bridge-spi-test
    PRIVATE
    Qt5::Core
    remotecommon
    spidevice
    bridgefmtspihelper
)

install(TARGETS bridge-spi-test
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
