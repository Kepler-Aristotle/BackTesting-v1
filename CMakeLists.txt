cmake_minimum_required(VERSION 3.16)
project(QuantBench VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 1. Core Library (Independent of CLI and GUI)
set(CORE_SOURCES
    src/data/CsvDataSource.cpp
    src/portfolio/Portfolio.cpp
    src/strategies/SMACrossoverStrategy.cpp
    src/strategies/BuyAndHoldStrategy.cpp
    src/engine/BacktestEngine.cpp
    src/analysis/PerformanceAnalyzer.cpp
    src/persistence/ResultSerializer.cpp
)

add_library(quantbench_core STATIC ${CORE_SOURCES})
target_include_directories(quantbench_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 2. Command-Line Backtesting Application
add_executable(quantbench_cli src/main.cpp)
target_link_libraries(quantbench_cli PRIVATE quantbench_core)

# 3. Automated Test Suite
set(TEST_SOURCES
    tests/test_main.cpp
    tests/test_csv.cpp
    tests/test_portfolio.cpp
    tests/test_strategy.cpp
    tests/test_engine.cpp
    tests/test_metrics.cpp
)

add_executable(quantbench_tests ${TEST_SOURCES})
target_link_libraries(quantbench_tests PRIVATE quantbench_core)

enable_testing()
add_test(NAME QuantBenchUnitTests COMMAND quantbench_tests WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# 4. Optional Qt 6 GUI Target (Added when user commands "INTERFACE" and Qt6 is installed)
option(BUILD_QUANTBENCH_GUI "Build the optional Qt 6 GUI executable" OFF)
if(BUILD_QUANTBENCH_GUI)
    find_package(Qt6 COMPONENTS Widgets QUIET)
    if(Qt6_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gui/main.cpp")
        set(CMAKE_AUTOMOC ON)
        add_executable(quantbench_gui
            gui/main.cpp
            gui/MainWindow.cpp
            gui/EquityCurveWidget.cpp
        )
        target_link_libraries(quantbench_gui PRIVATE quantbench_core Qt6::Widgets)
    else()
        message(STATUS "Qt6 Widgets or gui/ sources not present; skipping quantbench_gui.")
    endif()
endif()
