#include <iostream>
#include <cassert>
#include <cmath>
#include "engine/BacktestEngine.h"
#include "strategies/BuyAndHoldStrategy.h"
#include "strategies/SMACrossoverStrategy.h"

using namespace quantbench;

namespace {
bool approxEq(double a, double b, double eps = 1e-4) {
    return std::fabs(a - b) < eps;
}
}

void runEngineTests() {
    std::cout << "[TEST SUITE] Running BacktestEngine Simulation Loop Tests...\n";

    // 3-day dataset:
    // Day 1 (2025-01-01): Close 100 -> BuyAndHold generates BUY at Day 1 Close
    // Day 2 (2025-01-02): Open 105, Close 110 -> Pending BUY executes at Day 2 Open ($105), NOT Day 1 Close ($100)!
    // Day 3 (2025-01-03): Open 112, Close 120 -> End of simulation forced liquidation at Day 3 Close ($120)
    std::vector<Candle> data = {
        Candle("2025-01-01", 99.0, 101.0, 98.0, 100.0, 1000),
        Candle("2025-01-02", 105.0, 111.0, 104.0, 110.0, 1000),
        Candle("2025-01-03", 112.0, 121.0, 111.0, 120.0, 1000)
    };

    BacktestConfig cfg;
    cfg.initialCapital = 10000.0;
    cfg.tradingQuantity = 10;
    cfg.shortSmaPeriod = 2;
    cfg.longSmaPeriod = 3;
    cfg.brokerageRate = 0.0;
    cfg.slippageRate = 0.0;
    cfg.forceLiquidationAtEnd = true;

    BuyAndHoldStrategy bh;
    BacktestEngine engine;
    BacktestResult res = engine.run(data, bh, cfg);

    // Verify next-candle Open execution ($105.0) and final close liquidation ($120.0)
    assert(res.trades.size() == 1);
    assert(res.trades[0].getEntryDate() == "2025-01-02");
    assert(approxEq(res.trades[0].getEntryPrice(), 105.0));
    assert(res.trades[0].getExitDate() == "2025-01-03");
    assert(approxEq(res.trades[0].getExitPrice(), 120.0));
    assert(approxEq(res.netProfit, 150.0)); // 10 shares * (120 - 105)
    std::cout << "  ✓ Next-candle Open execution & final liquidation verified (Zero look-ahead bias)\n";

    // Verify order rejection when capital is insufficient
    BacktestConfig poorCfg = cfg;
    poorCfg.initialCapital = 500.0; // Needs $1050 for 10 shares @ $105
    BacktestResult poorRes = engine.run(data, bh, poorCfg);
    assert(poorRes.trades.empty());
    assert(!poorRes.orders.empty());
    assert(poorRes.orders[0].getStatus() == OrderStatus::REJECTED);
    std::cout << "  ✓ Insufficient capital order rejection recorded cleanly\n";
}
