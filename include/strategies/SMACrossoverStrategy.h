#ifndef QUANTBENCH_SMA_CROSSOVER_STRATEGY_H
#define QUANTBENCH_SMA_CROSSOVER_STRATEGY_H

#include "strategies/Strategy.h"

namespace quantbench {

/**
 * @brief Concrete Strategy implementing Simple Moving Average (SMA) Crossover.
 * Emits BUY when Short SMA crosses strictly above Long SMA (from <= to >),
 * and SELL when Short SMA crosses strictly below Long SMA (from >= to <).
 */
class SMACrossoverStrategy : public Strategy {
private:
    int shortPeriod_;
    int longPeriod_;

    static double calculateSMA(const std::vector<Candle>& data, size_t endIndex, int period);

public:
    SMACrossoverStrategy(int shortPeriod = 5, int longPeriod = 20);
    ~SMACrossoverStrategy() override = default;

    Signal generateSignal(const std::vector<Candle>& data, size_t index) const override;
    std::string name() const override;

    int getShortPeriod() const noexcept { return shortPeriod_; }
    int getLongPeriod() const noexcept { return longPeriod_; }
};

} // namespace quantbench

#endif // QUANTBENCH_SMA_CROSSOVER_STRATEGY_H
