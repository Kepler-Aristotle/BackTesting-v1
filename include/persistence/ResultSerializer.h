#ifndef QUANTBENCH_RESULT_SERIALIZER_H
#define QUANTBENCH_RESULT_SERIALIZER_H

#include <string>
#include "models/BacktestResult.h"

namespace quantbench {

/**
 * @brief Persists and restores BacktestResult objects to/from structured,
 * versioned text report files using std::ofstream and std::ifstream.
 * Demonstrates Module 4: Stream classes, Serialization, and Deserialization.
 */
class ResultSerializer {
public:
    static constexpr const char* FORMAT_HEADER = "QUANTBENCH_REPORT_V1";

    /**
     * @brief Serializes a BacktestResult to a structured text report file.
     */
    static void save(const BacktestResult& result, const std::string& filename);

    /**
     * @brief Deserializes a BacktestResult from a structured text report file.
     */
    static BacktestResult load(const std::string& filename);
};

} // namespace quantbench

#endif // QUANTBENCH_RESULT_SERIALIZER_H
