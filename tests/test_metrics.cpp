#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdio>
#include "analysis/PerformanceAnalyzer.h"
#include "persistence/ResultSerializer.h"

using namespace quantbench;

namespace {
bool near(double a, double b, double eps = 1e-4) {
    return std::fabs(a - b) < eps;
}
}

void runMetricsAndPersistenceTests() {
    std::cout << "[TEST SUITE] Running Performance Metrics & ResultSerializer Tests...\n";

    // 1. Historical Maximum Drawdown calculation
    // Equity sequence: 10000 -> 12000 (peak) -> 9000 (25% DD from 12000) -> 11000 -> 13000 (new peak) -> 11700 (10% DD)
    std::vector<EquitySnapshot> curve = {
        {"2025-01-01", 10000.0, 0, 100.0, 10000.0, 0.0},
        {"2025-01-02", 0.0, 100, 120.0, 12000.0, 2000.0},
        {"2025-01-03", 0.0, 100, 90.0,  9000.0, -1000.0},
        {"2025-01-04", 0.0, 100, 110.0, 11000.0, 1000.0},
        {"2025-01-05", 13000.0, 0, 130.0, 13000.0, 0.0},
        {"2025-01-06", 11700.0, 0, 117.0, 11700.0, 0.0}
    };

    double maxDd = PerformanceAnalyzer::calculateMaxDrawdown(curve);
    assert(near(maxDd, 25.0));
    std::cout << "  ✓ Historical Equity Curve Maximum Drawdown (25.0%) verified\n";

    // 2. Win rate, total return, and zero-trade edge cases
    Portfolio finalPortfolio(11700.0);
    std::vector<Trade> trades = {
        Trade(1, "2025-01-02", "2025-01-04", 100.0, 125.0, 100, 2500.0, 10.0, 2490.0),
        Trade(2, "2025-01-05", "2025-01-06", 125.0, 117.1, 100, -790.0, 0.0, -790.0)
    };
    std::vector<Order> orders;

    BacktestResult res = PerformanceAnalyzer::analyze(
        "Test Strategy", 10000.0, finalPortfolio, 117.0, trades, orders, curve
    );

    assert(near(res.totalReturnPct, 17.0));
    assert(res.totalTrades == 2);
    assert(res.winningTrades == 1);
    assert(res.losingTrades == 1);
    assert(near(res.winRatePct, 50.0));
    std::cout << "  ✓ Total Return (17.0%) and Win Rate (50.0%) verified\n";

    // 3. Serialization & Deserialization Round-Trip
    const std::string reportFile = "reports/test_roundtrip_report.txt";
    ResultSerializer::save(res, reportFile);
    BacktestResult loaded = ResultSerializer::load(reportFile);

    assert(loaded.strategyName == res.strategyName);
    assert(near(loaded.initialCapital, res.initialCapital));
    assert(near(loaded.finalEquity, res.finalEquity));
    assert(near(loaded.totalReturnPct, res.totalReturnPct));
    assert(near(loaded.winRatePct, res.winRatePct));
    assert(near(loaded.maxDrawdownPct, res.maxDrawdownPct));
    assert(loaded.trades.size() == 2);
    assert(loaded.equityCurve.size() == 6);
    std::remove(reportFile.c_str());
    std::cout << "  ✓ ResultSerializer save() & load() round-trip fidelity verified\n";
}
