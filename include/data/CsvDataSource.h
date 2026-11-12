#ifndef QUANTBENCH_CSV_DATASOURCE_H
#define QUANTBENCH_CSV_DATASOURCE_H

#include <string>
#include <vector>
#include "data/IDataSource.h"

namespace quantbench {

/**
 * @brief Concrete implementation of IDataSource that parses and validates local OHLCV CSV files.
 * Demonstrates Module 2 (Inheritance, Overloading, Overriding), Module 3 (Interface implementation,
 * Custom Exceptions), and Module 4 (Stream classes: ifstream, stringstream).
 */
class CsvDataSource : public IDataSource {
private:
    static std::string trim(const std::string& str);
    static Candle parseRow(const std::string& line, size_t rowNumber);

public:
    CsvDataSource() = default;
    ~CsvDataSource() override = default;

    std::vector<Candle> load(const std::string& filename) const override;

    std::vector<Candle> load(const std::string& filename,
                             const std::string& startDate,
                             const std::string& endDate) const override;
};

} // namespace quantbench

#endif // QUANTBENCH_CSV_DATASOURCE_H
