#ifndef QUANTBENCH_CUSTOM_EXCEPTIONS_H
#define QUANTBENCH_CUSTOM_EXCEPTIONS_H

#include <exception>
#include <string>
#include <utility>

namespace quantbench {

/**
 * @brief Base exception class for all QuantBench domain & runtime errors.
 * Demonstrates Module 3: Custom exception handling hierarchy & inheritance from std::exception.
 */
class QuantBenchException : public std::exception {
protected:
    std::string message_;

public:
    explicit QuantBenchException(std::string message)
        : message_(std::move(message)) {}

    ~QuantBenchException() noexcept override = default;

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

/**
 * @brief Thrown when a CSV file or report file cannot be opened or found.
 */
class DataFileException : public QuantBenchException {
public:
    explicit DataFileException(const std::string& filename, const std::string& details = "Unable to open file")
        : QuantBenchException("DataFileException [" + filename + "]: " + details) {}
};

/**
 * @brief Thrown when CSV rows are malformed, OHLC values are inconsistent,
 * volume is negative, or dates are out of chronological order.
 */
class DataValidationException : public QuantBenchException {
private:
    size_t rowNumber_;

public:
    DataValidationException(size_t rowNumber, const std::string& reason)
        : QuantBenchException("DataValidationException at row " + std::to_string(rowNumber) + ": " + reason),
          rowNumber_(rowNumber) {}

    explicit DataValidationException(const std::string& reason)
        : QuantBenchException("DataValidationException: " + reason),
          rowNumber_(0) {}

    size_t getRowNumber() const noexcept { return rowNumber_; }
};

/**
 * @brief Thrown when BacktestConfig or Strategy parameters are invalid.
 */
class ConfigurationException : public QuantBenchException {
public:
    explicit ConfigurationException(const std::string& reason)
        : QuantBenchException("ConfigurationException: " + reason) {}
};

/**
 * @brief Thrown when an order violates portfolio constraints (e.g. insufficient funds,
 * negative quantity, or selling without an active position).
 */
class OrderExecutionException : public QuantBenchException {
public:
    explicit OrderExecutionException(const std::string& reason)
        : QuantBenchException("OrderExecutionException: " + reason) {}
};

/**
 * @brief Thrown when saving or loading a serialized BacktestResult fails.
 */
class SerializationException : public QuantBenchException {
public:
    explicit SerializationException(const std::string& reason)
        : QuantBenchException("SerializationException: " + reason) {}
};

/**
 * @brief Thrown when operations on custom data structures (such as LinkedQueue) are invalid.
 */
class EmptyQueueException : public QuantBenchException {
public:
    EmptyQueueException()
        : QuantBenchException("EmptyQueueException: Attempted to access or dequeue from an empty LinkedQueue.") {}
};

} // namespace quantbench

#endif // QUANTBENCH_CUSTOM_EXCEPTIONS_H
