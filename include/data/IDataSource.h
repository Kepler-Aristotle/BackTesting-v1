#ifndef QUANTBENCH_IDATASOURCE_H
#define QUANTBENCH_IDATASOURCE_H

#include <string>
#include <vector>
#include "models/Candle.h"

namespace quantbench {

/**
 * @brief Pure abstract interface defining the contract for loading historical market data.
 * Demonstrates Module 3: Concept and implementation of an interface in C++.
 */
class IDataSource {
public:
    virtual ~IDataSource() = default;

    /**
     * @brief Loads all valid historical candles from the given source.
     */
    virtual std::vector<Candle> load(const std::string& filename) const = 0;

    /**
     * @brief Overloaded method: Loads historical candles within an inclusive [startDate, endDate] range.
     * Demonstrates Module 2: Method overloading.
     */
    virtual std::vector<Candle> load(const std::string& filename,
                                     const std::string& startDate,
                                     const std::string& endDate) const = 0;
};

} // namespace quantbench

#endif // QUANTBENCH_IDATASOURCE_H
