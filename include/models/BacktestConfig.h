#ifndef QUANTBENCH_BACKTEST_CONFIG_H
#define QUANTBENCH_BACKTEST_CONFIG_H

#include "exceptions/CustomExceptions.h"

namespace quantbench {

/**
 * @brief Stores and validates user configuration parameters for a backtest simulation.
 */
class BacktestConfig {
public:
    double initialCapital;
    int tradingQuantity;
    int shortSmaPeriod;
    int longSmaPeriod;
    double brokerageRate;   // Fractional rate: e.g. 0.001 for 0.1%
    double slippageRate;    // Fractional rate: e.g. 0.0005 for 0.05%
    bool forceLiquidationAtEnd;

    BacktestConfig(
        double capital = 100000.0,
        int quantity = 100,
        int shortPeriod = 5,
        int longPeriod = 20,
        double brokerage = 0.001,
        double slippage = 0.0005,
        bool forceLiquidate = true
    ) : initialCapital(capital),
        tradingQuantity(quantity),
        shortSmaPeriod(shortPeriod),
        longSmaPeriod(longPeriod),
        brokerageRate(brokerage),
        slippageRate(slippage),
        forceLiquidationAtEnd(forceLiquidate) {}

    /**
     * @brief Validates all simulation configuration rules before running the engine.
     */
    void validate() const {
        if (initialCapital <= 0.0) {
            throw ConfigurationException("Initial capital must be strictly positive (> 0).");
        }
        if (tradingQuantity <= 0) {
            throw ConfigurationException("Trading quantity must be at least 1 share.");
        }
        if (shortSmaPeriod <= 0 || longSmaPeriod <= 0) {
            throw ConfigurationException("SMA periods must be positive integers (> 0).");
        }
        if (shortSmaPeriod >= longSmaPeriod) {
            throw ConfigurationException("Short SMA period must be strictly less than Long SMA period.");
        }
        if (brokerageRate < 0.0 || brokerageRate >= 0.5) {
            throw ConfigurationException("Brokerage rate must be non-negative and less than 50% (0.5).");
        }
        if (slippageRate < 0.0 || slippageRate >= 0.5) {
            throw ConfigurationException("Slippage rate must be non-negative and less than 50% (0.5).");
        }
    }
};

} // namespace quantbench

#endif // QUANTBENCH_BACKTEST_CONFIG_H
