#include "analysis/PerformanceAnalyzer.h"
#include <algorithm>

namespace quantbench {

double PerformanceAnalyzer::calculateMaxDrawdown(const std::vector<EquitySnapshot>& equityCurve) {
    if (equityCurve.empty()) {
        return 0.0;
    }

    double peakEquity = equityCurve.front().totalEquity;
    double maxDrawdownPct = 0.0;

    for (const auto& snapshot : equityCurve) {
        if (snapshot.totalEquity > peakEquity) {
            peakEquity = snapshot.totalEquity;
        }
        if (peakEquity > 0.0) {
            double drawdownPct = ((peakEquity - snapshot.totalEquity) / peakEquity) * 100.0;
            if (drawdownPct > maxDrawdownPct) {
                maxDrawdownPct = drawdownPct;
            }
        }
    }

    return maxDrawdownPct;
}

BacktestResult PerformanceAnalyzer::analyze(
    const std::string& strategyName,
    double initialCapital,
    const Portfolio& finalPortfolio,
    double lastClosePrice,
    const std::vector<Trade>& trades,
    const std::vector<Order>& orders,
    const std::vector<EquitySnapshot>& equityCurve
) {
    BacktestResult res;
    res.strategyName = strategyName;
    res.initialCapital = initialCapital;
    res.finalEquity = finalPortfolio.getEquity(lastClosePrice);
    res.realizedPnL = finalPortfolio.getRealizedPnL();
    res.unrealizedPnL = finalPortfolio.getUnrealizedPnL(lastClosePrice);
    res.netProfit = res.finalEquity - initialCapital;
    res.totalReturnPct = (initialCapital > 0.0)
        ? ((res.finalEquity - initialCapital) / initialCapital) * 100.0
        : 0.0;

    res.totalTrades = static_cast<int>(trades.size());
    res.winningTrades = 0;
    res.losingTrades = 0;

    for (const auto& t : trades) {
        if (t.getNetPnL() > 0.0) {
            ++res.winningTrades;
        } else {
            ++res.losingTrades;
        }
    }

    res.winRatePct = (res.totalTrades > 0)
        ? (static_cast<double>(res.winningTrades) / static_cast<double>(res.totalTrades)) * 100.0
        : 0.0;

    res.maxDrawdownPct = calculateMaxDrawdown(equityCurve);
    res.trades = trades;
    res.orders = orders;
    res.equityCurve = equityCurve;

    return res;
}

} // namespace quantbench
