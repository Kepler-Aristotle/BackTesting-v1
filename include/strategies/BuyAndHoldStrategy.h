#ifndef QUANTBENCH_BUY_AND_HOLD_STRATEGY_H
#define QUANTBENCH_BUY_AND_HOLD_STRATEGY_H

#include "strategies/Strategy.h"

namespace quantbench {

/**
 * @brief Concrete Strategy that buys on the very first historical bar (index 0)
 * and holds until the end of the simulation.
 * Demonstrates runtime polymorphism alongside SMACrossoverStrategy.
 */
class BuyAndHoldStrategy : public Strategy {
public:
    BuyAndHoldStrategy() = default;
    ~BuyAndHoldStrategy() override = default;

    Signal generateSignal(const std::vector<Candle>& data, size_t index) const override;
    std::string name() const override;
};

} // namespace quantbench

#endif // QUANTBENCH_BUY_AND_HOLD_STRATEGY_H
