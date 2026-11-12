#ifndef QUANTBENCH_PORTFOLIO_H
#define QUANTBENCH_PORTFOLIO_H

#include <string>
#include "models/Trade.h"

namespace quantbench {

/**
 * @brief Manages available cash, held shares, cost basis, and realized/unrealized P&L.
 * Demonstrates Module 1 & 2: Strict Encapsulation (private state, controlled public mutators/getters)
 * and Method Overloading.
 */
class Portfolio {
private:
    double cash_;
    int shares_;
    double entryPrice_;
    double entryFee_;
    std::string entryDate_;
    double realizedPnL_;

public:
    explicit Portfolio(double initialCapital = 100000.0);

    /**
     * @brief Executes a BUY order at the specified execution price (without extra brokerage/date).
     */
    void buy(int quantity, double executionPrice);

    /**
     * @brief Overloaded BUY method accounting for brokerage rate and entry date.
     */
    void buy(int quantity, double executionPrice, double brokerageRate, const std::string& date);

    /**
     * @brief Executes a SELL order at the specified execution price (without extra brokerage/date).
     */
    void sell(int quantity, double executionPrice);

    /**
     * @brief Overloaded SELL method accounting for brokerage rate and exit date, returning the completed Trade.
     */
    Trade sell(int quantity, double executionPrice, double brokerageRate, const std::string& exitDate);

    // Getters (Const-correct encapsulation)
    double getCash() const noexcept { return cash_; }
    int getShares() const noexcept { return shares_; }
    double getEntryPrice() const noexcept { return entryPrice_; }
    double getEntryFee() const noexcept { return entryFee_; }
    const std::string& getEntryDate() const noexcept { return entryDate_; }
    double getRealizedPnL() const noexcept { return realizedPnL_; }
    bool hasOpenPosition() const noexcept { return shares_ > 0; }

    /**
     * @brief Computes total mark-to-market portfolio equity at the given market price.
     */
    double getEquity(double marketPrice) const noexcept;

    /**
     * @brief Computes current unrealized P&L (excluding exit fee) at the given market price.
     */
    double getUnrealizedPnL(double marketPrice) const noexcept;
};

} // namespace quantbench

#endif // QUANTBENCH_PORTFOLIO_H
