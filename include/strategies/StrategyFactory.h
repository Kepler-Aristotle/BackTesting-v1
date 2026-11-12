#ifndef QUANTBENCH_STRATEGY_FACTORY_H
#define QUANTBENCH_STRATEGY_H_FACTORY

#include <memory>
#include <string>
#include <algorithm>
#include <cctype>
#include "strategies/Strategy.h"
#include "strategies/SMACrossoverStrategy.h"
#include "strategies/BuyAndHoldStrategy.h"
#include "models/BacktestConfig.h"
#include "exceptions/CustomExceptions.h"

namespace quantbench {

/**
 * @brief Factory Pattern implementation for instantiating Strategy objects dynamically.
 * Demonstrates Module 5: Factory Design Pattern & smart pointer RAII ownership.
 */
class StrategyFactory {
public:
    static std::unique_ptr<Strategy> createStrategy(const std::string& type, const BacktestConfig& config) {
        std::string key = type;
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (key == "sma" || key == "smacrossover" || key == "sma_crossover") {
            return std::make_unique<SMACrossoverStrategy>(config.shortSmaPeriod, config.longSmaPeriod);
        } else if (key == "buyandhold" || key == "buy_and_hold" || key == "hold" || key == "bh") {
            return std::make_unique<BuyAndHoldStrategy>();
        }

        throw ConfigurationException("Unknown strategy type '" + type + "'. Supported: 'sma', 'buy_and_hold'.");
    }
};

} // namespace quantbench

#endif // QUANTBENCH_STRATEGY_H_FACTORY
