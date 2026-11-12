#ifndef QUANTBENCH_BACKTEST_RESULT_H
#define QUANTBENCH_BACKTEST_RESULT_H

#include <string>
#include <vector>
#include "models/Trade.h"
#include "models/Order.h"

namespace quantbench {

/**
 * @brief Represents a single point on the historical portfolio equity curve.
 */
struct EquitySnapshot {
    std::string date;
    double cash = 0.0;
    int shares = 0;
    double closePrice = 0.0;
    double totalEquity = 0.0;
    double unrealizedPnL = 0.0;
};

/**
 * @brief Stores the complete output of a backtest simulation, including performance
 * statistics, completed trades, order history, and the historical equity curve.
 */
class BacktestResult {
public:
    std::string strategyName;
    double initialCapital = 0.0;
    double finalEquity = 0.0;
    double realizedPnL = 0.0;
    double unrealizedPnL = 0.0;
    double netProfit = 0.0;
    double totalReturnPct = 0.0;

    int totalTrades = 0;
    int winningTrades = 0;
    int losingTrades = 0;
    double winRatePct = 0.0;
    double maxDrawdownPct = 0.0;

    std::vector<Trade> trades;
    std::vector<Order> orders;
    std::vector<EquitySnapshot> equityCurve;

    BacktestResult() = default;
};

} // namespace quantbench

#endif // QUANTBENCH_BACKTEST_RESULT_H
