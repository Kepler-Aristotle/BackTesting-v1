#ifndef QUANTBENCH_TRADE_H
#define QUANTBENCH_TRADE_H

#include <string>
#include <utility>

namespace quantbench {

/**
 * @brief Represents a completed round-trip transaction (entry BUY to exit SELL).
 * Demonstrates Module 2: Static members (shared trade ID counter) vs non-static instance members.
 */
class Trade {
private:
    static int nextTradeId_;

    int tradeId_;
    std::string entryDate_;
    std::string exitDate_;
    double entryPrice_;
    double exitPrice_;
    int quantity_;
    double grossPnL_;
    double transactionFees_;
    double netPnL_;

public:
    Trade()
        : tradeId_(++nextTradeId_),
          entryDate_(""),
          exitDate_(""),
          entryPrice_(0.0),
          exitPrice_(0.0),
          quantity_(0),
          grossPnL_(0.0),
          transactionFees_(0.0),
          netPnL_(0.0) {}

    Trade(std::string entryDate, std::string exitDate,
          double entryPrice, double exitPrice, int quantity,
          double entryFee, double exitFee)
        : tradeId_(++nextTradeId_),
          entryDate_(std::move(entryDate)),
          exitDate_(std::move(exitDate)),
          entryPrice_(entryPrice),
          exitPrice_(exitPrice),
          quantity_(quantity),
          grossPnL_((exitPrice - entryPrice) * quantity),
          transactionFees_(entryFee + exitFee),
          netPnL_(grossPnL_ - transactionFees_) {}

    // Explicit constructor used during report deserialization
    Trade(int explicitId, std::string entryDate, std::string exitDate,
          double entryPrice, double exitPrice, int quantity,
          double grossPnL, double transactionFees, double netPnL)
        : tradeId_(explicitId),
          entryDate_(std::move(entryDate)),
          exitDate_(std::move(exitDate)),
          entryPrice_(entryPrice),
          exitPrice_(exitPrice),
          quantity_(quantity),
          grossPnL_(grossPnL),
          transactionFees_(transactionFees),
          netPnL_(netPnL) {
        if (explicitId > nextTradeId_) {
            nextTradeId_ = explicitId;
        }
    }

    int getTradeId() const noexcept { return tradeId_; }
    const std::string& getEntryDate() const noexcept { return entryDate_; }
    const std::string& getExitDate() const noexcept { return exitDate_; }
    double getEntryPrice() const noexcept { return entryPrice_; }
    double getExitPrice() const noexcept { return exitPrice_; }
    int getQuantity() const noexcept { return quantity_; }
    double getGrossPnL() const noexcept { return grossPnL_; }
    double getTransactionFees() const noexcept { return transactionFees_; }
    double getNetPnL() const noexcept { return netPnL_; }
    bool isWin() const noexcept { return netPnL_ > 0.0; }

    static void resetIdCounter(int start = 0) noexcept {
        nextTradeId_ = start;
    }

    static int getCurrentCounter() noexcept {
        return nextTradeId_;
    }
};

inline int Trade::nextTradeId_ = 0;

} // namespace quantbench

#endif // QUANTBENCH_TRADE_H
