#include "persistence/ResultSerializer.h"
#include "exceptions/CustomExceptions.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

namespace quantbench {

namespace {
std::string trimStr(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

std::vector<std::string> splitCsv(const std::string& line) {
    std::vector<std::string> out;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ',')) {
        out.push_back(trimStr(item));
    }
    return out;
}
} // anonymous namespace

void ResultSerializer::save(const BacktestResult& result, const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        throw SerializationException("Unable to open file '" + filename + "' for writing.");
    }

    out << std::fixed << std::setprecision(6);
    out << FORMAT_HEADER << "\n";
    out << "[SUMMARY]\n";
    out << "StrategyName," << result.strategyName << "\n";
    out << "InitialCapital," << result.initialCapital << "\n";
    out << "FinalEquity," << result.finalEquity << "\n";
    out << "RealizedPnL," << result.realizedPnL << "\n";
    out << "UnrealizedPnL," << result.unrealizedPnL << "\n";
    out << "NetProfit," << result.netProfit << "\n";
    out << "TotalReturnPct," << result.totalReturnPct << "\n";
    out << "TotalTrades," << result.totalTrades << "\n";
    out << "WinningTrades," << result.winningTrades << "\n";
    out << "LosingTrades," << result.losingTrades << "\n";
    out << "WinRatePct," << result.winRatePct << "\n";
    out << "MaxDrawdownPct," << result.maxDrawdownPct << "\n";

    out << "[TRADES]\n";
    out << "TradeID,EntryDate,ExitDate,EntryPrice,ExitPrice,Quantity,GrossPnL,Fees,NetPnL\n";
    for (const auto& t : result.trades) {
        out << t.getTradeId() << ","
            << t.getEntryDate() << ","
            << t.getExitDate() << ","
            << t.getEntryPrice() << ","
            << t.getExitPrice() << ","
            << t.getQuantity() << ","
            << t.getGrossPnL() << ","
            << t.getTransactionFees() << ","
            << t.getNetPnL() << "\n";
    }

    out << "[EQUITY_CURVE]\n";
    out << "Date,Cash,Shares,ClosePrice,TotalEquity,UnrealizedPnL\n";
    for (const auto& eq : result.equityCurve) {
        out << eq.date << ","
            << eq.cash << ","
            << eq.shares << ","
            << eq.closePrice << ","
            << eq.totalEquity << ","
            << eq.unrealizedPnL << "\n";
    }

    if (!out.good()) {
        throw SerializationException("I/O error occurred while writing report to '" + filename + "'.");
    }
}

BacktestResult ResultSerializer::load(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        throw SerializationException("Unable to open report file '" + filename + "' for reading.");
    }

    std::string line;
    if (!std::getline(in, line) || trimStr(line) != FORMAT_HEADER) {
        throw SerializationException("Invalid or corrupted report header in '" + filename + "'. Expected " + std::string(FORMAT_HEADER));
    }

    BacktestResult res;
    std::string currentSection = "";
    bool hasSummary = false;

    try {
        while (std::getline(in, line)) {
            std::string trimmed = trimStr(line);
            if (trimmed.empty()) continue;

            if (trimmed == "[SUMMARY]" || trimmed == "[TRADES]" || trimmed == "[EQUITY_CURVE]") {
                currentSection = trimmed;
                continue;
            }

            if (currentSection == "[SUMMARY]") {
                auto pos = trimmed.find(',');
                if (pos == std::string::npos) {
                    throw SerializationException("Malformed summary line: " + trimmed);
                }
                std::string key = trimStr(trimmed.substr(0, pos));
                std::string val = trimStr(trimmed.substr(pos + 1));

                if (key == "StrategyName") { res.strategyName = val; hasSummary = true; }
                else if (key == "InitialCapital") res.initialCapital = std::stod(val);
                else if (key == "FinalEquity") res.finalEquity = std::stod(val);
                else if (key == "RealizedPnL") res.realizedPnL = std::stod(val);
                else if (key == "UnrealizedPnL") res.unrealizedPnL = std::stod(val);
                else if (key == "NetProfit") res.netProfit = std::stod(val);
                else if (key == "TotalReturnPct") res.totalReturnPct = std::stod(val);
                else if (key == "TotalTrades") res.totalTrades = std::stoi(val);
                else if (key == "WinningTrades") res.winningTrades = std::stoi(val);
                else if (key == "LosingTrades") res.losingTrades = std::stoi(val);
                else if (key == "WinRatePct") res.winRatePct = std::stod(val);
                else if (key == "MaxDrawdownPct") res.maxDrawdownPct = std::stod(val);
            } else if (currentSection == "[TRADES]") {
                if (trimmed.find("TradeID,") == 0) continue; // Header row
                auto cols = splitCsv(trimmed);
                if (cols.size() != 9) {
                    throw SerializationException("Malformed trade record in report: " + trimmed);
                }
                Trade t(
                    std::stoi(cols[0]),
                    cols[1],
                    cols[2],
                    std::stod(cols[3]),
                    std::stod(cols[4]),
                    std::stoi(cols[5]),
                    std::stod(cols[6]),
                    std::stod(cols[7]),
                    std::stod(cols[8])
                );
                res.trades.push_back(t);
            } else if (currentSection == "[EQUITY_CURVE]") {
                if (trimmed.find("Date,") == 0) continue; // Header row
                auto cols = splitCsv(trimmed);
                if (cols.size() != 6) {
                    throw SerializationException("Malformed equity curve row in report: " + trimmed);
                }
                EquitySnapshot snap;
                snap.date = cols[0];
                snap.cash = std::stod(cols[1]);
                snap.shares = std::stoi(cols[2]);
                snap.closePrice = std::stod(cols[3]);
                snap.totalEquity = std::stod(cols[4]);
                snap.unrealizedPnL = std::stod(cols[5]);
                res.equityCurve.push_back(snap);
            }
        }
    } catch (const SerializationException&) {
        throw;
    } catch (const std::exception& e) {
        throw SerializationException(std::string("Failed to parse report file: ") + e.what());
    }

    if (!hasSummary) {
        throw SerializationException("Report file '" + filename + "' is missing required [SUMMARY] section.");
    }

    return res;
}

} // namespace quantbench
