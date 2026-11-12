#ifndef QUANTBENCH_CANDLE_H
#define QUANTBENCH_CANDLE_H

#include <string>
#include <utility>
#include "exceptions/CustomExceptions.h"

namespace quantbench {

/**
 * @brief Represents a single historical OHLCV market bar (trading session).
 */
class Candle {
public:
    std::string date;
    double open;
    double high;
    double low;
    double close;
    long long volume;

    Candle()
        : date(""), open(0.0), high(0.0), low(0.0), close(0.0), volume(0) {}

    Candle(std::string dt, double o, double h, double l, double c, long long v)
        : date(std::move(dt)), open(o), high(h), low(l), close(c), volume(v) {}

    /**
     * @brief Validates OHLCV integrity rules:
     * - Date must be non-empty
     * - All prices > 0
     * - High >= max(Open, Close, Low)
     * - Low <= min(Open, Close, High)
     * - Volume >= 0
     */
    void validate(size_t rowNumber = 0) const {
        if (date.empty()) {
            throw DataValidationException(rowNumber, "Candle date cannot be empty.");
        }
        if (open <= 0.0 || high <= 0.0 || low <= 0.0 || close <= 0.0) {
            throw DataValidationException(rowNumber, "OHLC prices must all be strictly positive (> 0).");
        }
        if (high < open || high < close || high < low) {
            throw DataValidationException(rowNumber, "High price cannot be lower than Open, Close, or Low.");
        }
        if (low > open || low > close || low > high) {
            throw DataValidationException(rowNumber, "Low price cannot be higher than Open, Close, or High.");
        }
        if (volume < 0) {
            throw DataValidationException(rowNumber, "Volume cannot be negative.");
        }
    }
};

} // namespace quantbench

#endif // QUANTBENCH_CANDLE_H
