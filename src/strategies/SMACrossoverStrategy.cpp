#include "strategies/SMACrossoverStrategy.h"
#include "exceptions/CustomExceptions.h"

namespace quantbench {

SMACrossoverStrategy::SMACrossoverStrategy(int shortPeriod, int longPeriod)
    : shortPeriod_(shortPeriod), longPeriod_(longPeriod) {
    if (shortPeriod <= 0 || longPeriod <= 0) {
        throw ConfigurationException("SMA periods must be positive integers (> 0).");
    }
    if (shortPeriod >= longPeriod) {
        throw ConfigurationException("Short SMA period must be strictly smaller than Long SMA period.");
    }
}

double SMACrossoverStrategy::calculateSMA(const std::vector<Candle>& data, size_t endIndex, int period) {
    double sum = 0.0;
    size_t startIndex = endIndex + 1 - static_cast<size_t>(period);
    for (size_t i = startIndex; i <= endIndex; ++i) {
        sum += data[i].close;
    }
    return sum / static_cast<double>(period);
}

Signal SMACrossoverStrategy::generateSignal(const std::vector<Candle>& data, size_t index) const {
    if (index >= data.size()) {
        return Signal::HOLD;
    }

    // To detect a true crossover between (index - 1) and (index),
    // both (index - 1) and (index) must have at least `longPeriod_` candles of history.
    // Specifically, (index - 1) needs `longPeriod_ - 1` prior bars, so `index` must be >= `longPeriod_`.
    if (index < static_cast<size_t>(longPeriod_)) {
        return Signal::HOLD;
    }

    double prevShortSMA = calculateSMA(data, index - 1, shortPeriod_);
    double prevLongSMA  = calculateSMA(data, index - 1, longPeriod_);

    double currShortSMA = calculateSMA(data, index, shortPeriod_);
    double currLongSMA  = calculateSMA(data, index, longPeriod_);

    // Bullish Crossover: short SMA was <= long SMA on previous bar, and is now strictly > long SMA
    if (prevShortSMA <= prevLongSMA && currShortSMA > currLongSMA) {
        return Signal::BUY;
    }

    // Bearish Crossover: short SMA was >= long SMA on previous bar, and is now strictly < long SMA
    if (prevShortSMA >= prevLongSMA && currShortSMA < currLongSMA) {
        return Signal::SELL;
    }

    return Signal::HOLD;
}

std::string SMACrossoverStrategy::name() const {
    return "SMA Crossover (" + std::to_string(shortPeriod_) + "/" + std::to_string(longPeriod_) + ")";
}

} // namespace quantbench
