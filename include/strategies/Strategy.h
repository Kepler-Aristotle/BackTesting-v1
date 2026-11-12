#ifndef QUANTBENCH_STRATEGY_H
#define QUANTBENCH_STRATEGY_H

#include <string>
#include <vector>
#include <cstddef>
#include "models/Candle.h"
#include "models/Order.h"

namespace quantbench {

/**
 * @brief Abstract base class defining the Strategy Pattern contract.
 * Demonstrates Module 1 (Polymorphism & Late Binding), Module 2 (Method Overriding),
 * Module 3 (Abstract Classes), and Module 5 (Strategy Design Pattern).
 */
class Strategy {
public:
    virtual ~Strategy() = default;

    /**
     * @brief Evaluates historical market data up to and including `index`
     * without accessing any future candles (> index).
     */
    virtual Signal generateSignal(
        const std::vector<Candle>& data,
        size_t index
    ) const = 0;

    /**
     * @brief Returns the human-readable name of the strategy.
     */
    virtual std::string name() const = 0;
};

} // namespace quantbench

#endif // QUANTBENCH_STRATEGY_H
