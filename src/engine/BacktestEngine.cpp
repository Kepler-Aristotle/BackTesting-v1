#include "engine/BacktestEngine.h"
#include "portfolio/Portfolio.h"
#include "structures/LinkedQueue.h"
#include "analysis/PerformanceAnalyzer.h"
#include "exceptions/CustomExceptions.h"

namespace quantbench {

BacktestResult BacktestEngine::run(
    const std::vector<Candle>& data,
    Strategy& strategy,
    const BacktestConfig& config
) {
    // 1. Validate configuration & input dataset
    config.validate();
    if (data.empty()) {
        throw DataValidationException("BacktestEngine::run called with empty historical candle dataset.");
    }

    // 2. Initialize Portfolio, Custom LinkedQueue<Order>, and tracking lists
    Portfolio portfolio(config.initialCapital);
    LinkedQueue<Order> pendingOrders;
    std::vector<Trade> completedTrades;
    std::vector<Order> orderHistory;
    std::vector<EquitySnapshot> equityCurve;
    equityCurve.reserve(data.size());

    // 3. Chronological simulation loop: earliest candle to latest candle
    for (size_t i = 0; i < data.size(); ++i) {
        const Candle& currentCandle = data[i];

        // Step A: Execute any pending orders generated on the previous candle
        // at the current candle's OPENING price (prevents look-ahead bias)
        while (!pendingOrders.empty()) {
            Order pendingOrder = pendingOrders.peek();
            pendingOrders.dequeue();

            if (pendingOrder.getSide() == OrderSide::BUY) {
                if (portfolio.hasOpenPosition()) {
                    pendingOrder.markRejected("Position already open; pyramiding not allowed.");
                } else {
                    // Adverse slippage on BUY: Open * (1 + slippageRate)
                    double execPrice = currentCandle.open * (1.0 + config.slippageRate);
                    try {
                        portfolio.buy(
                            pendingOrder.getQuantity(),
                            execPrice,
                            config.brokerageRate,
                            currentCandle.date
                        );
                        pendingOrder.markExecuted(currentCandle.date);
                    } catch (const OrderExecutionException& e) {
                        pendingOrder.markRejected(e.what());
                    }
                }
            } else if (pendingOrder.getSide() == OrderSide::SELL) {
                if (!portfolio.hasOpenPosition()) {
                    pendingOrder.markRejected("No open position to sell; short selling disabled.");
                } else {
                    // Adverse slippage on SELL: Open * (1 - slippageRate)
                    double execPrice = currentCandle.open * (1.0 - config.slippageRate);
                    try {
                        Trade completed = portfolio.sell(
                            portfolio.getShares(),
                            execPrice,
                            config.brokerageRate,
                            currentCandle.date
                        );
                        completedTrades.push_back(completed);
                        pendingOrder.markExecuted(currentCandle.date);
                    } catch (const OrderExecutionException& e) {
                        pendingOrder.markRejected(e.what());
                    }
                }
            }

            orderHistory.push_back(pendingOrder);
        }

        // Step B: Evaluate strategy using historical data available up to current candle `i`
        Signal signal = strategy.generateSignal(data, i);

        // Step C: If BUY or SELL, queue an Order for execution on the NEXT candle (i + 1)
        if (signal == Signal::BUY) {
            Order buyOrder(OrderSide::BUY, config.tradingQuantity, currentCandle.date);
            if (i + 1 < data.size()) {
                pendingOrders.enqueue(buyOrder);
            } else {
                // Final candle signal has no next historical candle on which to execute
                buyOrder.markDiscarded("Signal generated on final candle; no next candle available for execution.");
                orderHistory.push_back(buyOrder);
            }
        } else if (signal == Signal::SELL) {
            int qtyToSell = portfolio.hasOpenPosition() ? portfolio.getShares() : config.tradingQuantity;
            Order sellOrder(OrderSide::SELL, qtyToSell, currentCandle.date);
            if (i + 1 < data.size()) {
                pendingOrders.enqueue(sellOrder);
            } else {
                sellOrder.markDiscarded("Signal generated on final candle; no next historical candle available.");
                orderHistory.push_back(sellOrder);
            }
        }

        // Step D: Mark portfolio to market at the current candle's CLOSING price
        EquitySnapshot snap;
        snap.date = currentCandle.date;
        snap.cash = portfolio.getCash();
        snap.shares = portfolio.getShares();
        snap.closePrice = currentCandle.close;
        snap.totalEquity = portfolio.getEquity(currentCandle.close);
        snap.unrealizedPnL = portfolio.getUnrealizedPnL(currentCandle.close);
        equityCurve.push_back(snap);
    }

    // 4. End-of-simulation force liquidation if enabled and position remains open
    const Candle& finalCandle = data.back();
    if (config.forceLiquidationAtEnd && portfolio.hasOpenPosition()) {
        double exitExecPrice = finalCandle.close * (1.0 - config.slippageRate);
        int openQty = portfolio.getShares();
        Trade forcedTrade = portfolio.sell(openQty, exitExecPrice, config.brokerageRate, finalCandle.date);
        completedTrades.push_back(forcedTrade);

        Order liqOrder(OrderSide::SELL, openQty, finalCandle.date);
        liqOrder.markExecuted(finalCandle.date + " (Forced Final Liquidation at Close)");
        orderHistory.push_back(liqOrder);

        // Update final equity curve point to reflect post-liquidation cash balance
        if (!equityCurve.empty()) {
            EquitySnapshot& lastSnap = equityCurve.back();
            lastSnap.cash = portfolio.getCash();
            lastSnap.shares = portfolio.getShares();
            lastSnap.totalEquity = portfolio.getEquity(finalCandle.close);
            lastSnap.unrealizedPnL = 0.0;
        }
    }

    // 5. Compute final metrics and return BacktestResult
    return PerformanceAnalyzer::analyze(
        strategy.name(),
        config.initialCapital,
        portfolio,
        finalCandle.close,
        completedTrades,
        orderHistory,
        equityCurve
    );
}

} // namespace quantbench
