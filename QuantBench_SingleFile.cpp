// =============================================================================
// QuantBench.cpp — Student-Friendly C++ OOP Stock Backtesting Engine + GUI Export
// =============================================================================
// Only 4 basic headers used (iostream, fstream, string, vector)
// Covers all 5 Syllabus Modules AND automatically generates "quantbench_gui.html"
// (an interactive visual dashboard with SVG Equity Curve & Trade Table)!
// =============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

// =============================================================================
// MODULE 3: CUSTOM EXCEPTION HANDLING CLASSES
// =============================================================================

class BacktestException {
protected:
    string message;

public:
    BacktestException(string msg) {
        message = msg;
    }
    string getMessage() const {
        return message;
    }
};

class DataException : public BacktestException {
public:
    DataException(string msg) : BacktestException("Data Error: " + msg) {}
};

class TradeException : public BacktestException {
public:
    TradeException(string msg) : BacktestException("Trade Error: " + msg) {}
};

// =============================================================================
// MODULE 4: TEMPLATES & LINKED LIST QUEUE DATA STRUCTURE
// =============================================================================

template <typename T>
class LinkedQueue {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val) {
            data = val;
            next = nullptr;
        }
    };

    Node* frontNode;
    Node* rearNode;

public:
    LinkedQueue() {
        frontNode = nullptr;
        rearNode = nullptr;
    }

    ~LinkedQueue() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    bool isEmpty() const {
        return frontNode == nullptr;
    }

    void enqueue(T value) {
        Node* newNode = new Node(value);
        if (isEmpty()) {
            frontNode = newNode;
            rearNode = newNode;
        } else {
            rearNode->next = newNode;
            rearNode = newNode;
        }
    }

    T dequeue() {
        if (isEmpty()) {
            throw BacktestException("Queue Error: Cannot dequeue from an empty queue!");
        }
        Node* temp = frontNode;
        T val = temp->data;
        frontNode = frontNode->next;

        if (frontNode == nullptr) {
            rearNode = nullptr;
        }
        delete temp;
        return val;
    }
};

// =============================================================================
// MODULE 1 & 2: DOMAIN CLASSES & STATIC MEMBERS
// =============================================================================

class Candle {
public:
    string date;
    double open;
    double high;
    double low;
    double close;
    long volume;

    Candle() {
        date = "";
        open = 0; high = 0; low = 0; close = 0; volume = 0;
    }

    Candle(string d, double o, double h, double l, double c, long v) {
        date = d;
        open = o;
        high = h;
        low = l;
        close = c;
        volume = v;
    }
};

const int SIGNAL_BUY  = 1;
const int SIGNAL_SELL = -1;
const int SIGNAL_HOLD = 0;

class Order {
public:
    int side;
    int quantity;
    string signalDate;

    Order() {
        side = SIGNAL_HOLD;
        quantity = 0;
        signalDate = "";
    }

    Order(int s, int q, string d) {
        side = s;
        quantity = q;
        signalDate = d;
    }
};

class Trade {
private:
    static int tradeCounter; // Static member shared across all Trade objects

public:
    int tradeId;
    string entryDate;
    string exitDate;
    double entryPrice;
    double exitPrice;
    int quantity;
    double feesPaid;
    double netProfit;

    Trade() {
        tradeId = 0;
        entryDate = ""; exitDate = "";
        entryPrice = 0; exitPrice = 0;
        quantity = 0; feesPaid = 0; netProfit = 0;
    }

    Trade(string inDate, string outDate, double inPrice, double outPrice, int qty, double fees) {
        tradeCounter++;
        tradeId = tradeCounter;
        entryDate = inDate;
        exitDate = outDate;
        entryPrice = inPrice;
        exitPrice = outPrice;
        quantity = qty;
        feesPaid = fees;
        double grossProfit = (exitPrice - entryPrice) * quantity;
        netProfit = grossProfit - fees;
    }
};

int Trade::tradeCounter = 0;

// =============================================================================
// MODULE 3 & 4: INTERFACE & CSV STREAM LOADER (ifstream)
// =============================================================================

class IDataSource {
public:
    virtual ~IDataSource() {}
    virtual vector<Candle> load(string filename) = 0;
    virtual vector<Candle> load(string filename, string startDate, string endDate) = 0;
};

class CsvDataSource : public IDataSource {
private:
    vector<string> splitByComma(string line) {
        vector<string> parts;
        string current = "";
        for (size_t i = 0; i < line.length(); i++) {
            if (line[i] == ',') {
                parts.push_back(current);
                current = "";
            } else if (line[i] != '\r' && line[i] != '\n') {
                current += line[i];
            }
        }
        parts.push_back(current);
        return parts;
    }

public:
    vector<Candle> load(string filename) override {
        ifstream file(filename);
        if (!file.is_open()) {
            throw DataException("Could not open CSV file: " + filename);
        }

        vector<Candle> candles;
        string line;
        int rowNumber = 0;

        while (getline(file, line)) {
            rowNumber++;
            if (line.empty()) continue;

            if (rowNumber == 1 && line.find("Date") != string::npos) {
                continue;
            }

            vector<string> cols = splitByComma(line);
            if (cols.size() != 6) {
                throw DataException("Invalid column count at row " + to_string(rowNumber));
            }

            string date = cols[0];
            double open  = stod(cols[1]);
            double high  = stod(cols[2]);
            double low   = stod(cols[3]);
            double close = stod(cols[4]);
            long volume  = stol(cols[5]);

            if (open <= 0 || high < low || low <= 0 || close <= 0 || volume < 0) {
                throw DataException("Invalid price or volume values at row " + to_string(rowNumber));
            }

            candles.push_back(Candle(date, open, high, low, close, volume));
        }

        file.close();
        if (candles.empty()) {
            throw DataException("CSV file is empty!");
        }
        return candles;
    }

    // Overloaded load method with date filtering (Module 2)
    vector<Candle> load(string filename, string startDate, string endDate) override {
        vector<Candle> allCandles = load(filename);
        vector<Candle> filtered;

        for (size_t i = 0; i < allCandles.size(); i++) {
            if (allCandles[i].date >= startDate && allCandles[i].date <= endDate) {
                filtered.push_back(allCandles[i]);
            }
        }
        return filtered;
    }
};

// =============================================================================
// MODULE 1 & 2: PORTFOLIO CLASS (Data Encapsulation)
// =============================================================================

class Portfolio {
private:
    double cash;
    int shares;
    double entryPrice;
    double entryFee;
    string entryDate;

public:
    Portfolio(double initialCapital) {
        if (initialCapital <= 0) {
            throw TradeException("Initial capital must be greater than zero!");
        }
        cash = initialCapital;
        shares = 0;
        entryPrice = 0.0;
        entryFee = 0.0;
        entryDate = "";
    }

    void buy(int qty, double price, double brokerageRate, string date) {
        if (shares > 0) {
            throw TradeException("Position already open! Cannot buy again until sold.");
        }
        double cost = qty * price;
        double fee  = cost * brokerageRate;
        double totalNeeded = cost + fee;

        if (totalNeeded > cash) {
            throw TradeException("Insufficient cash to buy " + to_string(qty) + " shares!");
        }

        cash -= totalNeeded;
        shares = qty;
        entryPrice = price;
        entryFee = fee;
        entryDate = date;
    }

    Trade sell(double price, double brokerageRate, string exitDate) {
        if (shares <= 0) {
            throw TradeException("No shares held to sell!");
        }
        int qtySold = shares;
        double proceeds = qtySold * price;
        double exitFee  = proceeds * brokerageRate;
        double netProceeds = proceeds - exitFee;

        cash += netProceeds;
        Trade completed(entryDate, exitDate, entryPrice, price, qtySold, entryFee + exitFee);

        shares = 0;
        entryPrice = 0.0;
        entryFee = 0.0;
        entryDate = "";

        return completed;
    }

    double getCash() const { return cash; }
    int getShares() const { return shares; }
    bool hasOpenPosition() const { return shares > 0; }

    double getTotalEquity(double currentMarketPrice) const {
        return cash + (shares * currentMarketPrice);
    }
};

// =============================================================================
// MODULE 1, 2, 3 & 5: STRATEGY PATTERN & RUNTIME POLYMORPHISM
// =============================================================================

class Strategy {
public:
    virtual ~Strategy() {}
    virtual int generateSignal(const vector<Candle>& data, int index) const = 0;
    virtual string getName() const = 0;
};

class SMACrossoverStrategy : public Strategy {
private:
    int shortPeriod;
    int longPeriod;

    double calculateAverage(const vector<Candle>& data, int endIdx, int period) const {
        double sum = 0.0;
        int startIdx = endIdx - period + 1;
        for (int i = startIdx; i <= endIdx; i++) {
            sum += data[i].close;
        }
        return sum / period;
    }

public:
    SMACrossoverStrategy(int shortP, int longP) {
        shortPeriod = shortP;
        longPeriod = longP;
    }

    int generateSignal(const vector<Candle>& data, int index) const override {
        if (index < longPeriod) {
            return SIGNAL_HOLD;
        }

        double prevShort = calculateAverage(data, index - 1, shortPeriod);
        double prevLong  = calculateAverage(data, index - 1, longPeriod);

        double currShort = calculateAverage(data, index, shortPeriod);
        double currLong  = calculateAverage(data, index, longPeriod);

        if (prevShort <= prevLong && currShort > currLong) {
            return SIGNAL_BUY;
        }
        if (prevShort >= prevLong && currShort < currLong) {
            return SIGNAL_SELL;
        }

        return SIGNAL_HOLD;
    }

    string getName() const override {
        return "SMA Crossover (" + to_string(shortPeriod) + "/" + to_string(longPeriod) + ")";
    }
};

class BuyAndHoldStrategy : public Strategy {
public:
    int generateSignal(const vector<Candle>& data, int index) const override {
        if (index == 0 && !data.empty()) {
            return SIGNAL_BUY;
        }
        return SIGNAL_HOLD;
    }

    string getName() const override {
        return "Buy and Hold";
    }
};

// =============================================================================
// MODULE 4: OBJECT SERIALIZATION & GUI HTML DASHBOARD GENERATOR
// =============================================================================

struct BacktestResult {
    string strategyName;
    double initialCapital;
    double finalEquity;
    double netProfit;
    double totalReturnPct;
    int totalTrades;
    int winningTrades;
    double winRatePct;
    double maxDrawdownPct;
    vector<Trade> trades;
    vector<string> dates;
    vector<double> closePrices;
    vector<double> equityHistory;
};

class ResultSerializer {
public:
    // 1. Save text report
    static void save(const BacktestResult& res, string filename) {
        ofstream out(filename);
        if (!out.is_open()) {
            throw DataException("Could not open report file to save: " + filename);
        }

        out << "QUANTBENCH_REPORT\n";
        out << res.strategyName << "\n";
        out << res.initialCapital << "\n";
        out << res.finalEquity << "\n";
        out << res.netProfit << "\n";
        out << res.totalReturnPct << "\n";
        out << res.totalTrades << "\n";
        out << res.winningTrades << "\n";
        out << res.winRatePct << "\n";
        out << res.maxDrawdownPct << "\n";

        for (size_t i = 0; i < res.trades.size(); i++) {
            const Trade& t = res.trades[i];
            out << t.tradeId << "," << t.entryDate << "," << t.exitDate << ","
                << t.entryPrice << "," << t.exitPrice << "," << t.quantity << ","
                << t.netProfit << "\n";
        }
        out.close();
    }

    // 2. Load text report
    static BacktestResult load(string filename) {
        ifstream in(filename);
        if (!in.is_open()) {
            throw DataException("Could not open saved report file: " + filename);
        }

        string header;
        getline(in, header);
        if (header != "QUANTBENCH_REPORT") {
            throw DataException("Invalid report file format!");
        }

        BacktestResult res;
        getline(in, res.strategyName);

        string line;
        getline(in, line); res.initialCapital = stod(line);
        getline(in, line); res.finalEquity    = stod(line);
        getline(in, line); res.netProfit      = stod(line);
        getline(in, line); res.totalReturnPct = stod(line);
        getline(in, line); res.totalTrades    = stoi(line);
        getline(in, line); res.winningTrades  = stoi(line);
        getline(in, line); res.winRatePct     = stod(line);
        getline(in, line); res.maxDrawdownPct = stod(line);

        in.close();
        return res;
    }

    // 3. Export Presentable GUI Dashboard (HTML + SVG Equity Curve Chart)
    static void saveGuiDashboard(const BacktestResult& res, string htmlFilename) {
        ofstream out(htmlFilename);
        if (!out.is_open()) {
            throw DataException("Could not write GUI HTML file: " + htmlFilename);
        }

        // Compute SVG polyline coordinates for the Equity Curve (width=760, height=240)
        double minEq = res.initialCapital;
        double maxEq = res.initialCapital;
        for (size_t i = 0; i < res.equityHistory.size(); i++) {
            if (res.equityHistory[i] < minEq) minEq = res.equityHistory[i];
            if (res.equityHistory[i] > maxEq) maxEq = res.equityHistory[i];
        }
        double range = maxEq - minEq;
        if (range < 1.0) range = 1.0;

        string svgPoints = "";
        int n = static_cast<int>(res.equityHistory.size());
        for (int i = 0; i < n; i++) {
            double x = 40.0 + (n > 1 ? (i * 700.0 / (n - 1)) : 350.0);
            double y = 210.0 - ((res.equityHistory[i] - minEq) / range) * 170.0;
            svgPoints += to_string(static_cast<int>(x)) + "," + to_string(static_cast<int>(y)) + " ";
        }

        out << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset='UTF-8'>\n"
            << "<title>QuantBench GUI Dashboard - " << res.strategyName << "</title>\n"
            << "<style>\n"
            << "  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #0f172a; color: #f8fafc; margin: 0; padding: 24px; }\n"
            << "  .container { max-width: 960px; margin: 0 auto; }\n"
            << "  .header { display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid #1e293b; padding-bottom: 16px; margin-bottom: 24px; }\n"
            << "  .badge { background: #064e3b; color: #34d399; padding: 4px 12px; border-radius: 99px; font-size: 12px; font-weight: 600; }\n"
            << "  .cards { display: grid; grid-template-columns: repeat(5, 1fr); gap: 12px; margin-bottom: 24px; }\n"
            << "  .card { background: #1e293b; border: 1px solid #334155; border-radius: 12px; padding: 14px; }\n"
            << "  .card-label { font-size: 11px; color: #94a3b8; text-transform: uppercase; }\n"
            << "  .card-val { font-size: 18px; font-weight: 700; margin-top: 6px; color: #38bdf8; }\n"
            << "  .panel { background: #1e293b; border: 1px solid #334155; border-radius: 12px; padding: 20px; margin-bottom: 24px; }\n"
            << "  table { width: 100%; border-collapse: collapse; font-size: 13px; margin-top: 12px; }\n"
            << "  th, td { padding: 10px 12px; text-align: left; border-bottom: 1px solid #334155; }\n"
            << "  th { color: #94a3b8; font-size: 11px; text-transform: uppercase; }\n"
            << "  .win { color: #34d399; font-weight: 600; }\n"
            << "</style>\n</head>\n<body>\n<div class='container'>\n"
            << "  <div class='header'>\n"
            << "    <div><h1 style='margin:0;font-size:22px;'>QuantBench Backtesting GUI Report</h1>\n"
            << "    <p style='margin:4px 0 0;color:#94a3b8;font-size:13px;'>Strategy: " << res.strategyName << "</p></div>\n"
            << "    <span class='badge'>C++ OOP Engine Verified</span>\n"
            << "  </div>\n"
            << "  <div class='cards'>\n"
            << "    <div class='card'><div class='card-label'>Initial Capital</div><div class='card-val'>$" << res.initialCapital << "</div></div>\n"
            << "    <div class='card'><div class='card-label'>Final Equity</div><div class='card-val'>$" << res.finalEquity << "</div></div>\n"
            << "    <div class='card'><div class='card-label'>Net Profit</div><div class='card-val' style='color:#34d399;'>+$" << res.netProfit << "</div></div>\n"
            << "    <div class='card'><div class='card-label'>Win Rate</div><div class='card-val'>" << res.winRatePct << "%</div></div>\n"
            << "    <div class='card'><div class='card-label'>Max Drawdown</div><div class='card-val' style='color:#fbbf24;'>" << res.maxDrawdownPct << "%</div></div>\n"
            << "  </div>\n"
            << "  <div class='panel'>\n"
            << "    <h3 style='margin:0 0 12px;font-size:15px;'>Portfolio Equity Curve (Mark-to-Market)</h3>\n"
            << "    <svg viewBox='0 0 780 240' style='width:100%;height:240px;background:#0f172a;border-radius:8px;'>\n"
            << "      <line x1='40' y1='40' x2='740' y2='40' stroke='#1e293b' stroke-width='1'/>\n"
            << "      <line x1='40' y1='125' x2='740' y2='125' stroke='#1e293b' stroke-width='1'/>\n"
            << "      <line x1='40' y1='210' x2='740' y2='210' stroke='#334155' stroke-width='1'/>\n"
            << "      <polyline fill='none' stroke='#10b981' stroke-width='3' points='" << svgPoints << "'/>\n"
            << "    </svg>\n"
            << "  </div>\n"
            << "  <div class='panel'>\n"
            << "    <h3 style='margin:0;font-size:15px;'>Completed Trade History</h3>\n"
            << "    <table>\n"
            << "      <thead><tr><th>Trade ID</th><th>Entry Date</th><th>Exit Date</th><th>Entry Price</th><th>Exit Price</th><th>Quantity</th><th>Fees</th><th>Net P&L</th></tr></thead>\n"
            << "      <tbody>\n";

        for (size_t i = 0; i < res.trades.size(); i++) {
            const Trade& t = res.trades[i];
            out << "        <tr><td>#" << t.tradeId << "</td><td>" << t.entryDate << "</td><td>" << t.exitDate
                << "</td><td>$" << t.entryPrice << "</td><td>$" << t.exitPrice << "</td><td>" << t.quantity
                << "</td><td>$" << t.feesPaid << "</td><td class='win'>$" << t.netProfit << "</td></tr>\n";
        }

        out << "      </tbody>\n    </table>\n  </div>\n</div>\n</body>\n</html>\n";
        out.close();
    }
};

// =============================================================================
// BACKTEST ENGINE (Coordinates Data, Strategy, Queue & Portfolio)
// =============================================================================

class BacktestEngine {
public:
    BacktestResult run(const vector<Candle>& data, const Strategy& strategy,
                       double initialCapital, int tradeQuantity,
                       double brokerageRate, double slippageRate) {

        Portfolio portfolio(initialCapital);
        LinkedQueue<Order> orderQueue;
        vector<Trade> completedTrades;
        vector<string> dates;
        vector<double> closePrices;
        vector<double> equityCurve;

        int n = static_cast<int>(data.size());

        for (int i = 0; i < n; i++) {
            const Candle& today = data[i];

            // 1. Execute any order generated yesterday at TODAY'S OPEN price
            while (!orderQueue.isEmpty()) {
                Order pending = orderQueue.dequeue();

                if (pending.side == SIGNAL_BUY && !portfolio.hasOpenPosition()) {
                    double execPrice = today.open * (1.0 + slippageRate);
                    try {
                        portfolio.buy(pending.quantity, execPrice, brokerageRate, today.date);
                    } catch (const BacktestException& e) {
                        cout << "  [Order Rejected on " << today.date << "] " << e.getMessage() << "\n";
                    }
                } else if (pending.side == SIGNAL_SELL && portfolio.hasOpenPosition()) {
                    double execPrice = today.open * (1.0 - slippageRate);
                    try {
                        Trade t = portfolio.sell(execPrice, brokerageRate, today.date);
                        completedTrades.push_back(t);
                    } catch (const BacktestException& e) {
                        cout << "  [Order Rejected on " << today.date << "] " << e.getMessage() << "\n";
                    }
                }
            }

            // 2. Ask Strategy for a signal using data up to today (Polymorphic call)
            int signal = strategy.generateSignal(data, i);

            // 3. If BUY or SELL signal and there is a tomorrow, queue order for tomorrow's Open
            if ((signal == SIGNAL_BUY || signal == SIGNAL_SELL) && (i + 1 < n)) {
                orderQueue.enqueue(Order(signal, tradeQuantity, today.date));
            }

            // 4. Record end-of-day portfolio equity at today's Close price
            dates.push_back(today.date);
            closePrices.push_back(today.close);
            equityCurve.push_back(portfolio.getTotalEquity(today.close));
        }

        // 5. Force liquidate any open position on the final day's Close
        const Candle& lastDay = data.back();
        if (portfolio.hasOpenPosition()) {
            double finalExecPrice = lastDay.close * (1.0 - slippageRate);
            Trade finalTrade = portfolio.sell(finalExecPrice, brokerageRate, lastDay.date);
            completedTrades.push_back(finalTrade);
            equityCurve.back() = portfolio.getTotalEquity(lastDay.close);
        }

        // 6. Calculate Performance Metrics
        BacktestResult result;
        result.strategyName   = strategy.getName();
        result.initialCapital = initialCapital;
        result.finalEquity    = portfolio.getTotalEquity(lastDay.close);
        result.netProfit      = result.finalEquity - initialCapital;
        result.totalReturnPct = (result.netProfit / initialCapital) * 100.0;
        result.totalTrades    = static_cast<int>(completedTrades.size());
        result.winningTrades  = 0;

        for (size_t i = 0; i < completedTrades.size(); i++) {
            if (completedTrades[i].netProfit > 0) {
                result.winningTrades++;
            }
        }

        result.winRatePct = (result.totalTrades > 0)
            ? (static_cast<double>(result.winningTrades) / result.totalTrades) * 100.0
            : 0.0;

        double peak = initialCapital;
        double maxDrawdown = 0.0;
        for (size_t i = 0; i < equityCurve.size(); i++) {
            if (equityCurve[i] > peak) {
                peak = equityCurve[i];
            }
            double drawdown = ((peak - equityCurve[i]) / peak) * 100.0;
            if (drawdown > maxDrawdown) {
                maxDrawdown = drawdown;
            }
        }
        result.maxDrawdownPct = maxDrawdown;
        result.trades         = completedTrades;
        result.dates          = dates;
        result.closePrices    = closePrices;
        result.equityHistory  = equityCurve;

        return result;
    }
};

// =============================================================================
// HELPER TO CREATE SAMPLE CSV & PRINT RESULTS
// =============================================================================

void createSampleCsvIfMissing(string filename) {
    ifstream check(filename);
    if (check.is_open()) {
        check.close();
        return;
    }

    ofstream out(filename);
    out << "Date,Open,High,Low,Close,Volume\n"
        << "2025-01-01,100.0,102.5,99.0,101.0,15000\n"
        << "2025-01-02,101.0,103.0,98.5,100.0,16200\n"
        << "2025-01-03,100.0,101.5,97.0,98.0,17500\n"
        << "2025-01-04,98.0,99.5,95.0,96.0,19000\n"
        << "2025-01-05,96.0,98.0,94.0,95.0,21000\n"
        << "2025-01-06,95.0,97.5,93.5,94.0,18500\n"
        << "2025-01-07,94.0,96.0,92.0,93.0,20000\n"
        << "2025-01-08,93.0,99.0,92.5,98.0,25000\n"
        << "2025-01-09,98.0,104.0,97.5,103.0,28000\n"
        << "2025-01-10,103.0,109.0,102.0,108.0,31000\n"
        << "2025-01-11,108.0,113.5,107.0,112.0,29500\n"
        << "2025-01-12,112.0,117.0,110.5,116.0,33000\n"
        << "2025-01-13,116.0,121.0,115.0,120.0,35000\n"
        << "2025-01-14,120.0,124.5,118.5,123.0,32000\n"
        << "2025-01-15,123.0,126.0,120.0,121.0,27000\n"
        << "2025-01-16,121.0,122.0,114.0,115.0,30000\n"
        << "2025-01-17,115.0,116.5,108.0,109.0,34000\n"
        << "2025-01-18,109.0,110.0,102.0,103.0,36000\n"
        << "2025-01-19,103.0,105.0,98.0,99.0,29000\n"
        << "2025-01-20,99.0,103.0,97.5,102.0,24000\n"
        << "2025-01-21,102.0,108.0,101.0,107.0,26500\n"
        << "2025-01-22,107.0,114.0,106.5,113.0,30500\n"
        << "2025-01-23,113.0,119.5,112.0,118.0,32500\n"
        << "2025-01-24,118.0,125.0,117.5,124.0,36000\n"
        << "2025-01-25,124.0,129.0,123.0,128.0,38000\n";
    out.close();
}

void displayReport(const BacktestResult& res) {
    cout << "\n------------------------------------------------------------\n";
    cout << "  STRATEGY REPORT: " << res.strategyName << "\n";
    cout << "------------------------------------------------------------\n";
    cout << "  Initial Capital   : $" << res.initialCapital << "\n";
    cout << "  Final Equity      : $" << res.finalEquity << "\n";
    cout << "  Net Profit / Loss : $" << res.netProfit << " (" << res.totalReturnPct << "%)\n";
    cout << "  Total Trades      : " << res.totalTrades << "\n";
    cout << "  Winning Trades    : " << res.winningTrades << "\n";
    cout << "  Win Rate          : " << res.winRatePct << "%\n";
    cout << "  Maximum Drawdown  : " << res.maxDrawdownPct << "%\n";
    cout << "------------------------------------------------------------\n";

    cout << "  COMPLETED TRADES:\n";
    for (size_t i = 0; i < res.trades.size(); i++) {
        const Trade& t = res.trades[i];
        cout << "  Trade #" << t.tradeId
             << " | Buy: " << t.entryDate << " @ $" << t.entryPrice
             << " | Sell: " << t.exitDate << " @ $" << t.exitPrice
             << " | Qty: " << t.quantity
             << " | Net P&L: $" << t.netProfit << "\n";
    }
    cout << "============================================================\n";
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main() {
    cout << "============================================================\n";
    cout << "  QuantBench — C++ OOP Stock Market Backtesting Engine\n";
    cout << "============================================================\n";

    try {
        string csvFile = "sample_data.csv";
        createSampleCsvIfMissing(csvFile);

        // 1. Load CSV Data via Interface Pointer (Module 3 & 4)
        IDataSource* loader = new CsvDataSource();
        vector<Candle> candles = loader->load(csvFile);
        cout << "[INFO] Loaded " << candles.size() << " candles from " << csvFile << "\n";

        BacktestEngine engine;
        double initialCapital = 100000.0;
        int tradeQty          = 100;
        double brokerage      = 0.001;  // 0.1% fee
        double slippage       = 0.0005; // 0.05% slippage

        // 2. Run Strategy 1: SMA Crossover via Base Class Pointer (Runtime Polymorphism)
        Strategy* strategy1 = new SMACrossoverStrategy(3, 7);
        BacktestResult smaResult = engine.run(candles, *strategy1, initialCapital, tradeQty, brokerage, slippage);
        displayReport(smaResult);

        // 3. Save Text Report & Visual HTML GUI Dashboard (Module 4 Serialization)
        string reportFile = "quantbench_report.txt";
        string guiFile    = "quantbench_gui.html";
        ResultSerializer::save(smaResult, reportFile);
        ResultSerializer::saveGuiDashboard(smaResult, guiFile);

        BacktestResult loadedReport = ResultSerializer::load(reportFile);
        cout << "[INFO] Saved & verified report file '" << reportFile
             << "' (Restored Final Equity: $" << loadedReport.finalEquity << ")\n";
        cout << "[GUI]  Generated visual GUI dashboard: '" << guiFile
             << "' (Double-click this file to view your Equity Curve & Trade Table in any browser!)\n";

        // 4. Run Strategy 2: Buy and Hold using the exact same Engine (Polymorphism)
        Strategy* strategy2 = new BuyAndHoldStrategy();
        BacktestResult bhResult = engine.run(candles, *strategy2, initialCapital, tradeQty, brokerage, slippage);
        displayReport(bhResult);

        delete strategy1;
        delete strategy2;
        delete loader;

    } catch (const BacktestException& e) {
        cout << "[ERROR] " << e.getMessage() << "\n";
        return 1;
    }

    return 0;
}
