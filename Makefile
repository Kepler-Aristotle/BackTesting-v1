# QuantBench Makefile (Works directly with g++ / clang++ on macOS and MinGW on Windows)
CXX      ?= g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude

CORE_SRCS := \
	src/data/CsvDataSource.cpp \
	src/portfolio/Portfolio.cpp \
	src/strategies/SMACrossoverStrategy.cpp \
	src/strategies/BuyAndHoldStrategy.cpp \
	src/engine/BacktestEngine.cpp \
	src/analysis/PerformanceAnalyzer.cpp \
	src/persistence/ResultSerializer.cpp

TEST_SRCS := \
	tests/test_main.cpp \
	tests/test_csv.cpp \
	tests/test_portfolio.cpp \
	tests/test_strategy.cpp \
	tests/test_engine.cpp \
	tests/test_metrics.cpp

.PHONY: all cli tests run test clean

all: cli tests

cli:
	$(CXX) $(CXXFLAGS) $(CORE_SRCS) src/main.cpp -o quantbench_cli

tests:
	$(CXX) $(CXXFLAGS) $(CORE_SRCS) $(TEST_SRCS) -o quantbench_tests

run: cli
	./quantbench_cli

test: tests
	./quantbench_tests

clean:
	rm -f quantbench_cli quantbench_tests quantbench_cli.exe quantbench_tests.exe
