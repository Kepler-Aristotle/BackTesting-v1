#ifndef QUANTBENCH_BACKTEST_ENGINE_H
#define QUANTBENCH_BACKTEST_ENGINE_H

#include <vector>
#include "models/Candle.h"
#include "models/BacktestConfig.h"
#include "models/BacktestResult.h"
#include "strategies/Strategy.h"

namespace quantbench {

/**
 * @brief Coordinates the chronological historical simulation loop.
 * Enforces zero look-ahead bias by executing orders generated at Candle[i] Close
 * at Candle[i+1] Open, applying adverse slippage and brokerage fees, and tracking
 * portfolio mark-to-market equity.
 */
class BacktestEngine {
public:
    BacktestEngine() = default;
    ~BacktestEngine() = default;

    BacktestResult run(
        const std::vector<Candle>& data,
        Strategy& strategy,
        const BacktestConfig& config
    );
};

} // namespace quantbench

#endif // QUANTBENCH_BACKTEST_ENGINE_H
