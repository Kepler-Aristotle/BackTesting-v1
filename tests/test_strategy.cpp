#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include "strategies/SMACrossoverStrategy.h"
#include "strategies/BuyAndHoldStrategy.h"

using namespace quantbench;

void runStrategyTests() {
    std::cout << "[TEST SUITE] Running Strategy & Polymorphism Tests...\n";

    // Construct a controlled 7-candle dataset for SMA(2, 4)
    // Prices: 100, 100, 100, 100, 110, 120, 90
    std::vector<Candle> candles = {
        Candle("2025-01-01", 100, 101, 99, 100, 1000), // idx 0
        Candle("2025-01-02", 100, 101, 99, 100, 1000), // idx 1
        Candle("2025-01-03", 100, 101, 99, 100, 1000), // idx 2
        Candle("2025-01-04", 100, 101, 99, 100, 1000), // idx 3: SMA2=100, SMA4=100
        Candle("2025-01-05", 100, 112, 99, 110, 1000), // idx 4: prev(100<=100), curr SMA2=105 > SMA4=102.5 -> BUY!
        Candle("2025-01-06", 110, 122, 109, 120, 1000),// idx 5: prev(105>102.5), curr SMA2=115 > SMA4=107.5 -> HOLD (already above)
        Candle("2025-01-07", 120, 121, 88, 90, 1000)   // idx 6: prev(115>=107.5), curr SMA2=105 == SMA4=105 -> HOLD
    };
    candles.push_back(Candle("2025-01-08", 90, 92, 79, 80, 1000)); // idx 7: prev(105>=105), curr SMA2=85 < SMA4=100 -> SELL!

    std::unique_ptr<Strategy> sma = std::make_unique<SMACrossoverStrategy>(2, 4);

    // 1. Insufficient history returns HOLD
    assert(sma->generateSignal(candles, 0) == Signal::HOLD);
    assert(sma->generateSignal(candles, 1) == Signal::HOLD);
    assert(sma->generateSignal(candles, 2) == Signal::HOLD);
    assert(sma->generateSignal(candles, 3) == Signal::HOLD);
    std::cout << "  ✓ Insufficient SMA history returns HOLD\n";

    // 2. Bullish crossover at index 4
    assert(sma->generateSignal(candles, 4) == Signal::BUY);
    std::cout << "  ✓ Bullish SMA crossover emits BUY\n";

    // 3. Staying above without new crossover returns HOLD
    assert(sma->generateSignal(candles, 5) == Signal::HOLD);
    std::cout << "  ✓ Continuing trend without fresh crossover emits HOLD\n";

    // 4. Bearish crossover at index 7
    assert(sma->generateSignal(candles, 7) == Signal::SELL);
    std::cout << "  ✓ Bearish SMA crossover emits SELL\n";

    // 5. No future-data leakage verification: modifying future candles after index 4 must not alter signal at index 4
    std::vector<Candle> truncated(candles.begin(), candles.begin() + 5);
    assert(sma->generateSignal(truncated, 4) == Signal::BUY);
    std::cout << "  ✓ Zero future-data leakage verified\n";

    // 6. Runtime polymorphism with BuyAndHoldStrategy
    std::unique_ptr<Strategy> bh = std::make_unique<BuyAndHoldStrategy>();
    assert(bh->generateSignal(candles, 0) == Signal::BUY);
    assert(bh->generateSignal(candles, 4) == Signal::HOLD);
    std::cout << "  ✓ Runtime polymorphism verified across Strategy hierarchy\n";
}
