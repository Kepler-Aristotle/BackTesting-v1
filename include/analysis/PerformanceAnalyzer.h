#ifndef QUANTBENCH_PERFORMANCE_ANALYZER_H
#define QUANTBENCH_PERFORMANCE_ANALYZER_H

#include <string>
#include <vector>
#include "models/BacktestResult.h"
#include "models/Trade.h"
#include "models/Order.h"
#include "portfolio/Portfolio.h"

namespace quantbench {

/**
 * @brief Calculates all required financial and statistical metrics from completed
 * trades and the historical portfolio equity curve.
 */
class PerformanceAnalyzer {
public:
    /**
     * @brief Computes all metrics and populates a BacktestResult object.
     */
    static BacktestResult analyze(
        const std::string& strategyName,
        double initialCapital,
        const Portfolio& finalPortfolio,
        double lastClosePrice,
        const std::vector<Trade>& trades,
        const std::vector<Order>& orders,
        const std::vector<EquitySnapshot>& equityCurve
    );

    /**
     * @brief Computes maximum percentage drawdown from the historical equity curve.
     */
    static double calculateMaxDrawdown(const std::vector<EquitySnapshot>& equityCurve);
};

} // namespace quantbench

#endif // QUANTBENCH_PERFORMANCE_ANALYZER_H
