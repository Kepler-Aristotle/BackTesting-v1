#include "strategies/BuyAndHoldStrategy.h"

namespace quantbench {

Signal BuyAndHoldStrategy::generateSignal(const std::vector<Candle>& data, size_t index) const {
    if (data.empty() || index >= data.size()) {
        return Signal::HOLD;
    }
    if (index == 0) {
        return Signal::BUY;
    }
    return Signal::HOLD;
}

std::string BuyAndHoldStrategy::name() const {
    return "Buy and Hold";
}

} // namespace quantbench
