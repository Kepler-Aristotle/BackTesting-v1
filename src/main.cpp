#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>

#include "data/CsvDataSource.h"
#include "strategies/StrategyFactory.h"
#include "engine/BacktestEngine.h"
#include "persistence/ResultSerializer.h"
#include "exceptions/CustomExceptions.h"

using namespace quantbench;

namespace {

void printBanner() {
    std::cout << "=============================================================================\n";
    std::cout << "  QuantBench — Modular C++17 Historical Stock-Market Backtesting Engine\n";
    std::cout << "=============================================================================\n";
}

void printUsage(const char* progName) {
    std::cout << "\nUsage:\n"
              << "  " << progName << " [options]\n\n"
              << "Options:\n"
              << "  --csv <path>          Path to historical OHLCV CSV file (default: data/sample_data.csv)\n"
              << "  --strategy <name>     Strategy: 'sma' or 'buy_and_hold' (default: sma)\n"
              << "  --capital <amount>    Initial capital in USD (default: 100000.0)\n"
              << "  --qty <shares>        Fixed trading quantity per order (default: 100)\n"
              << "  --short <period>      Short SMA window (default: 3)\n"
              << "  --long <period>       Long SMA window (default: 7)\n"
              << "  --brokerage <rate>    Brokerage rate e.g. 0.001 for 0.1% (default: 0.001)\n"
              << "  --slippage <rate>     Slippage rate e.g. 0.0005 for 0.05% (default: 0.0005)\n"
              << "  --start <YYYY-MM-DD>  Optional inclusive start date filter\n"
              << "  --end <YYYY-MM-DD>    Optional inclusive end date filter\n"
              << "  --save <path>         Save serialized report to file (default: reports/sample_report.txt)\n"
              << "  --load-report <path>  Load and display a previously saved report file\n"
              << "  --help                Display this help guide\n\n";
}

void printResultReport(const BacktestResult& result) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n-----------------------------------------------------------------------------\n";
    std::cout << "  PERFORMANCE SUMMARY  |  Strategy: " << result.strategyName << "\n";
    std::cout << "-----------------------------------------------------------------------------\n";
    std::cout << "  Initial Capital      : $" << std::setw(12) << result.initialCapital << "\n";
    std::cout << "  Final Portfolio Eq.  : $" << std::setw(12) << result.finalEquity << "\n";
    std::cout << "  Net Profit / Loss    : $" << std::setw(12) << result.netProfit
              << "  (" << (result.totalReturnPct >= 0.0 ? "+" : "") << result.totalReturnPct << "%)\n";
    std::cout << "  Realized P&L         : $" << std::setw(12) << result.realizedPnL << "\n";
    std::cout << "  Unrealized P&L       : $" << std::setw(12) << result.unrealizedPnL << "\n";
    std::cout << "  Total Completed Trd. :  " << std::setw(12) << result.totalTrades << "\n";
    std::cout << "  Winning / Losing Trd.:  " << std::setw(5) << result.winningTrades
              << " / " << result.losingTrades << "\n";
    std::cout << "  Win Rate             :  " << std::setw(11) << result.winRatePct << "%\n";
    std::cout << "  Maximum Drawdown     :  " << std::setw(11) << result.maxDrawdownPct << "%\n";
    std::cout << "-----------------------------------------------------------------------------\n";

    std::cout << "\n  COMPLETED TRADES LOG (" << result.trades.size() << " round-trip trades):\n";
    if (result.trades.empty()) {
        std::cout << "  (No completed trades recorded)\n";
    } else {
        std::cout << "  " << std::left
                  << std::setw(6)  << "ID"
                  << std::setw(13) << "Entry Date"
                  << std::setw(13) << "Exit Date"
                  << std::right
                  << std::setw(11) << "Entry $"
                  << std::setw(11) << "Exit $"
                  << std::setw(7)  << "Qty"
                  << std::setw(11) << "Fees $"
                  << std::setw(12) << "Net P&L $" << "\n";
        std::cout << "  " << std::string(84, '-') << "\n";
        for (const auto& t : result.trades) {
            std::cout << "  " << std::left
                      << std::setw(6)  << t.getTradeId()
                      << std::setw(13) << t.getEntryDate()
                      << std::setw(13) << t.getExitDate()
                      << std::right
                      << std::setw(11) << t.getEntryPrice()
                      << std::setw(11) << t.getExitPrice()
                      << std::setw(7)  << t.getQuantity()
                      << std::setw(11) << t.getTransactionFees()
                      << std::setw(12) << t.getNetPnL() << "\n";
        }
    }

    if (!result.orders.empty()) {
        std::cout << "\n  ORDER AUDIT TRAIL (" << result.orders.size() << " orders):\n";
        for (const auto& o : result.orders) {
            std::cout << "  • Order #" << o.getOrderId() << " ["
                      << Order::sideToString(o.getSide()) << " " << o.getQuantity()
                      << " sh, Signal @ " << o.getSignalDate() << "] -> "
                      << Order::statusToString(o.getStatus()) << " (" << o.getStatusReason() << ")\n";
        }
    }
    std::cout << "=============================================================================\n";
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    printBanner();

    std::string csvPath = "data/sample_data.csv";
    std::string strategyType = "sma";
    std::string startDate = "";
    std::string endDate = "";
    std::string savePath = "reports/sample_report.txt";
    std::string loadReportPath = "";

    BacktestConfig config;
    config.initialCapital = 100000.0;
    config.tradingQuantity = 100;
    config.shortSmaPeriod = 3;
    config.longSmaPeriod = 7;
    config.brokerageRate = 0.001;
    config.slippageRate = 0.0005;
    config.forceLiquidationAtEnd = true;

    // Parse CLI flags
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--csv" && i + 1 < argc) {
            csvPath = argv[++i];
        } else if (arg == "--strategy" && i + 1 < argc) {
            strategyType = argv[++i];
        } else if (arg == "--capital" && i + 1 < argc) {
            config.initialCapital = std::stod(argv[++i]);
        } else if (arg == "--qty" && i + 1 < argc) {
            config.tradingQuantity = std::stoi(argv[++i]);
        } else if (arg == "--short" && i + 1 < argc) {
            config.shortSmaPeriod = std::stoi(argv[++i]);
        } else if (arg == "--long" && i + 1 < argc) {
            config.longSmaPeriod = std::stoi(argv[++i]);
        } else if (arg == "--brokerage" && i + 1 < argc) {
            config.brokerageRate = std::stod(argv[++i]);
        } else if (arg == "--slippage" && i + 1 < argc) {
            config.slippageRate = std::stod(argv[++i]);
        } else if (arg == "--start" && i + 1 < argc) {
            startDate = argv[++i];
        } else if (arg == "--end" && i + 1 < argc) {
            endDate = argv[++i];
        } else if (arg == "--save" && i + 1 < argc) {
            savePath = argv[++i];
        } else if (arg == "--load-report" && i + 1 < argc) {
            loadReportPath = argv[++i];
        }
    }

    try {
        // Mode A: Reopen and display a previously serialized report
        if (!loadReportPath.empty()) {
            std::cout << "[INFO] Deserializing saved report from: " << loadReportPath << "\n";
            BacktestResult restored = ResultSerializer::load(loadReportPath);
            printResultReport(restored);
            return 0;
        }

        // Mode B: Execute full historical backtest
        CsvDataSource dataSource;
        std::vector<Candle> candles;

        if (!startDate.empty() || !endDate.empty()) {
            std::cout << "[INFO] Loading historical CSV '" << csvPath
                      << "' filtered [" << (startDate.empty() ? "BEGIN" : startDate)
                      << " to " << (endDate.empty() ? "END" : endDate) << "]...\n";
            candles = dataSource.load(csvPath, startDate, endDate);
        } else {
            std::cout << "[INFO] Loading historical CSV '" << csvPath << "'...\n";
            candles = dataSource.load(csvPath);
        }

        std::cout << "[INFO] Loaded " << candles.size() << " valid trading sessions ("
                  << candles.front().date << " to " << candles.back().date << ").\n";

        std::unique_ptr<Strategy> strategy = StrategyFactory::createStrategy(strategyType, config);
        std::cout << "[INFO] Selected Strategy: " << strategy->name() << "\n";

        BacktestEngine engine;
        BacktestResult result = engine.run(candles, *strategy, config);

        printResultReport(result);

        if (!savePath.empty()) {
            ResultSerializer::save(result, savePath);
            std::cout << "[INFO] Report serialized and saved to: " << savePath << "\n";
        }

    } catch (const QuantBenchException& e) {
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n[UNEXPECTED ERROR] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
