#include "portfolio/Portfolio.h"
#include "exceptions/CustomExceptions.h"

namespace quantbench {

Portfolio::Portfolio(double initialCapital)
    : cash_(initialCapital),
      shares_(0),
      entryPrice_(0.0),
      entryFee_(0.0),
      entryDate_(""),
      realizedPnL_(0.0) {
    if (initialCapital <= 0.0) {
        throw ConfigurationException("Portfolio initial capital must be strictly positive.");
    }
}

void Portfolio::buy(int quantity, double executionPrice) {
    buy(quantity, executionPrice, 0.0, "");
}

void Portfolio::buy(int quantity, double executionPrice, double brokerageRate, const std::string& date) {
    if (quantity <= 0) {
        throw OrderExecutionException("BUY order quantity must be strictly positive.");
    }
    if (executionPrice <= 0.0) {
        throw OrderExecutionException("BUY execution price must be strictly positive.");
    }
    if (shares_ > 0) {
        throw OrderExecutionException("Position already open (pyramiding disabled in single-position long-only simulator).");
    }

    double grossCost = static_cast<double>(quantity) * executionPrice;
    double brokerageFee = grossCost * brokerageRate;
    double totalCost = grossCost + brokerageFee;

    if (totalCost > cash_) {
        throw OrderExecutionException(
            "Insufficient funds for BUY: Required $" + std::to_string(totalCost) +
            " (including $" + std::to_string(brokerageFee) + " fee), Available $" + std::to_string(cash_)
        );
    }

    cash_ -= totalCost;
    shares_ = quantity;
    entryPrice_ = executionPrice;
    entryFee_ = brokerageFee;
    entryDate_ = date;
}

void Portfolio::sell(int quantity, double executionPrice) {
    sell(quantity, executionPrice, 0.0, "");
}

Trade Portfolio::sell(int quantity, double executionPrice, double brokerageRate, const std::string& exitDate) {
    if (quantity <= 0) {
        throw OrderExecutionException("SELL order quantity must be strictly positive.");
    }
    if (executionPrice <= 0.0) {
        throw OrderExecutionException("SELL execution price must be strictly positive.");
    }
    if (shares_ <= 0) {
        throw OrderExecutionException("Cannot SELL without an existing open position (short selling disabled).");
    }
    if (quantity != shares_) {
        throw OrderExecutionException("Must liquidate entire open position of " + std::to_string(shares_) + " shares.");
    }

    double grossProceeds = static_cast<double>(quantity) * executionPrice;
    double exitFee = grossProceeds * brokerageRate;
    double netProceeds = grossProceeds - exitFee;

    Trade completedTrade(entryDate_, exitDate, entryPrice_, executionPrice, quantity, entryFee_, exitFee);

    cash_ += netProceeds;
    realizedPnL_ += completedTrade.getNetPnL();

    // Reset open position state
    shares_ = 0;
    entryPrice_ = 0.0;
    entryFee_ = 0.0;
    entryDate_ = "";

    return completedTrade;
}

double Portfolio::getEquity(double marketPrice) const noexcept {
    return cash_ + (static_cast<double>(shares_) * marketPrice);
}

double Portfolio::getUnrealizedPnL(double marketPrice) const noexcept {
    if (shares_ <= 0) {
        return 0.0;
    }
    return ((marketPrice - entryPrice_) * static_cast<double>(shares_)) - entryFee_;
}

} // namespace quantbench
