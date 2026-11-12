#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>

#include "data/CsvDataSource.h"
#include "strategies/StrategyFactory.h"
#include "engine/BacktestEngine.h"
#include "persistence/ResultSerializer.h"
#include "exceptions/CustomExceptions.h"

namespace quantbench {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("QuantBench — Historical Stock-Market Backtesting Engine");
    resize(1080, 760);
    setupUi();

    // Attempt to preload sample_data.csv if available
    try {
        loadCsvFile("data/sample_data.csv");
    } catch (...) {
        statusLabel_->setText("Ready. Please browse and select a historical OHLCV CSV file.");
    }
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);

    // 1. Header Section
    auto* headerTitle = new QLabel("<h2>QuantBench — Historical Stock-Market Backtesting Engine</h2>", this);
    auto* headerDesc = new QLabel("Modular C++17 OOP Simulator • Zero Look-Ahead Bias • Next-Candle Open Execution", this);
    mainLayout->addWidget(headerTitle);
    mainLayout->addWidget(headerDesc);

    auto* contentLayout = new QHBoxLayout();

    // Left Column: Data, Strategy, and Execution Controls
    auto* leftCol = new QVBoxLayout();

    // 2. Data Section
    auto* dataGroup = new QGroupBox("1. Historical Data Source", this);
    auto* dataLayout = new QGridLayout(dataGroup);
    browseCsvBtn_ = new QPushButton("Browse CSV...", this);
    filenameLabel_ = new QLabel("None", this);
    candleCountLabel_ = new QLabel("0", this);
    dateRangeLabel_ = new QLabel("-", this);

    dataLayout->addWidget(browseCsvBtn_, 0, 0, 1, 2);
    dataLayout->addWidget(new QLabel("Selected File:", this), 1, 0);
    dataLayout->addWidget(filenameLabel_, 1, 1);
    dataLayout->addWidget(new QLabel("Loaded Candles:", this), 2, 0);
    dataLayout->addWidget(candleCountLabel_, 2, 1);
    dataLayout->addWidget(new QLabel("Date Range:", this), 3, 0);
    dataLayout->addWidget(dateRangeLabel_, 3, 1);
    leftCol->addWidget(dataGroup);

    // 3. Strategy & Parameters Section
    auto* stratGroup = new QGroupBox("2. Strategy & Configuration", this);
    auto* stratLayout = new QGridLayout(stratGroup);

    strategyCombo_ = new QComboBox(this);
    strategyCombo_->addItem("SMA Crossover", "sma");
    strategyCombo_->addItem("Buy and Hold", "buy_and_hold");

    shortSmaSpin_ = new QSpinBox(this);
    shortSmaSpin_->setRange(1, 200);
    shortSmaSpin_->setValue(3);

    longSmaSpin_ = new QSpinBox(this);
    longSmaSpin_->setRange(2, 500);
    longSmaSpin_->setValue(7);

    capitalSpin_ = new QDoubleSpinBox(this);
    capitalSpin_->setRange(100.0, 100000000.0);
    capitalSpin_->setDecimals(2);
    capitalSpin_->setValue(100000.0);

    quantitySpin_ = new QSpinBox(this);
    quantitySpin_->setRange(1, 1000000);
    quantitySpin_->setValue(100);

    brokerageSpin_ = new QDoubleSpinBox(this);
    brokerageSpin_->setRange(0.0, 10.0);
    brokerageSpin_->setDecimals(3);
    brokerageSpin_->setSingleStep(0.05);
    brokerageSpin_->setValue(0.100); // 0.1%

    slippageSpin_ = new QDoubleSpinBox(this);
    slippageSpin_->setRange(0.0, 10.0);
    slippageSpin_->setDecimals(3);
    slippageSpin_->setSingleStep(0.01);
    slippageSpin_->setValue(0.050); // 0.05%

    stratLayout->addWidget(new QLabel("Strategy:", this), 0, 0);
    stratLayout->addWidget(strategyCombo_, 0, 1);
    stratLayout->addWidget(new QLabel("Short SMA Period:", this), 1, 0);
    stratLayout->addWidget(shortSmaSpin_, 1, 1);
    stratLayout->addWidget(new QLabel("Long SMA Period:", this), 2, 0);
    stratLayout->addWidget(longSmaSpin_, 2, 1);
    stratLayout->addWidget(new QLabel("Initial Capital ($):", this), 3, 0);
    stratLayout->addWidget(capitalSpin_, 3, 1);
    stratLayout->addWidget(new QLabel("Trading Quantity:", this), 4, 0);
    stratLayout->addWidget(quantitySpin_, 4, 1);
    stratLayout->addWidget(new QLabel("Brokerage (%):", this), 5, 0);
    stratLayout->addWidget(brokerageSpin_, 5, 1);
    stratLayout->addWidget(new QLabel("Slippage (%):", this), 6, 0);
    stratLayout->addWidget(slippageSpin_, 6, 1);
    leftCol->addWidget(stratGroup);

    // 4. Execution Section
    runBacktestBtn_ = new QPushButton("Run Backtest", this);
    runBacktestBtn_->setMinimumHeight(40);
    statusLabel_ = new QLabel("Ready.", this);
    statusLabel_->setWordWrap(true);
    leftCol->addWidget(runBacktestBtn_);
    leftCol->addWidget(statusLabel_);
    leftCol->addStretch();

    contentLayout->addLayout(leftCol, 1);

    // Right Column: Results, Equity Curve, Trade History Table
    auto* rightCol = new QVBoxLayout();

    // 5. Results Section
    auto* resultsGroup = new QGroupBox("3. Performance Metrics Summary", this);
    auto* resLayout = new QGridLayout(resultsGroup);

    finalEquityLabel_ = new QLabel("$0.00", this);
    netProfitLabel_ = new QLabel("$0.00", this);
    totalReturnLabel_ = new QLabel("0.00%", this);
    winRateLabel_ = new QLabel("0.00%", this);
    maxDrawdownLabel_ = new QLabel("0.00%", this);

    resLayout->addWidget(new QLabel("<b>Final Equity:</b>", this), 0, 0);
    resLayout->addWidget(finalEquityLabel_, 0, 1);
    resLayout->addWidget(new QLabel("<b>Net Profit:</b>", this), 0, 2);
    resLayout->addWidget(netProfitLabel_, 0, 3);
    resLayout->addWidget(new QLabel("<b>Total Return:</b>", this), 0, 4);
    resLayout->addWidget(totalReturnLabel_, 0, 5);
    resLayout->addWidget(new QLabel("<b>Win Rate:</b>", this), 1, 0);
    resLayout->addWidget(winRateLabel_, 1, 1);
    resLayout->addWidget(new QLabel("<b>Max Drawdown:</b>", this), 1, 2);
    resLayout->addWidget(maxDrawdownLabel_, 1, 3);
    rightCol->addWidget(resultsGroup);

    // 6. Equity Curve Chart
    auto* chartGroup = new QGroupBox("4. Historical Portfolio Equity Curve", this);
    auto* chartLayout = new QVBoxLayout(chartGroup);
    equityChart_ = new EquityCurveWidget(this);
    chartLayout->addWidget(equityChart_);
    rightCol->addWidget(chartGroup);

    // 7. Trade History Table
    auto* tableGroup = new QGroupBox("5. Completed Round-Trip Trade History", this);
    auto* tableLayout = new QVBoxLayout(tableGroup);
    tradeTable_ = new QTableWidget(0, 7, this);
    tradeTable_->setHorizontalHeaderLabels({
        "Trade ID", "Entry Date", "Exit Date", "Entry Price", "Exit Price", "Quantity", "Net P&L"
    });
    tradeTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tradeTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableLayout->addWidget(tradeTable_);
    rightCol->addWidget(tableGroup);

    contentLayout->addLayout(rightCol, 2);
    mainLayout->addLayout(contentLayout);
    setCentralWidget(central);

    connect(browseCsvBtn_, &QPushButton::clicked, this, &MainWindow::onBrowseCsv);
    connect(runBacktestBtn_, &QPushButton::clicked, this, &MainWindow::onRunBacktest);
}

void MainWindow::loadCsvFile(const std::string& path) {
    CsvDataSource loader;
    loadedCandles_ = loader.load(path);
    selectedCsvPath_ = path;

    filenameLabel_->setText(QString::fromStdString(path));
    candleCountLabel_->setText(QString::number(loadedCandles_.size()));
    dateRangeLabel_->setText(
        QString::fromStdString(loadedCandles_.front().date + " to " + loadedCandles_.back().date)
    );
    statusLabel_->setText("CSV validated and loaded successfully.");
}

void MainWindow::onBrowseCsv() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "Select Historical OHLCV CSV", "", "CSV Files (*.csv);;All Files (*)"
    );
    if (filePath.isEmpty()) return;

    try {
        loadCsvFile(filePath.toStdString());
    } catch (const QuantBenchException& e) {
        statusLabel_->setText(QString("Validation Error: ") + e.what());
        QMessageBox::warning(this, "CSV Loading Error", e.what());
    }
}

void MainWindow::onRunBacktest() {
    if (loadedCandles_.empty()) {
        QMessageBox::warning(this, "No Data", "Please load a valid historical CSV file first.");
        return;
    }

    try {
        BacktestConfig config;
        config.initialCapital = capitalSpin_->value();
        config.tradingQuantity = quantitySpin_->value();
        config.shortSmaPeriod = shortSmaSpin_->value();
        config.longSmaPeriod = longSmaSpin_->value();
        config.brokerageRate = brokerageSpin_->value() / 100.0;
        config.slippageRate = slippageSpin_->value() / 100.0;
        config.forceLiquidationAtEnd = true;

        std::string stratKey = strategyCombo_->currentData().toString().toStdString();
        auto strategy = StrategyFactory::createStrategy(stratKey, config);

        BacktestEngine engine;
        BacktestResult result = engine.run(loadedCandles_, *strategy, config);

        populateResults(result);
        ResultSerializer::save(result, "reports/gui_latest_report.txt");
        statusLabel_->setText(
            QString("Backtest completed (%1). Report saved to reports/gui_latest_report.txt.")
                .arg(QString::fromStdString(strategy->name()))
        );
    } catch (const QuantBenchException& e) {
        statusLabel_->setText(QString("Simulation Error: ") + e.what());
        QMessageBox::critical(this, "Backtest Error", e.what());
    }
}

void MainWindow::populateResults(const BacktestResult& result) {
    finalEquityLabel_->setText(QString("$%1").arg(result.finalEquity, 0, 'f', 2));
    netProfitLabel_->setText(QString("$%1").arg(result.netProfit, 0, 'f', 2));
    totalReturnLabel_->setText(QString("%1%").arg(result.totalReturnPct, 0, 'f', 2));
    winRateLabel_->setText(QString("%1% (%2/%3)")
        .arg(result.winRatePct, 0, 'f', 1)
        .arg(result.winningTrades)
        .arg(result.totalTrades));
    maxDrawdownLabel_->setText(QString("%1%").arg(result.maxDrawdownPct, 0, 'f', 2));

    tradeTable_->setRowCount(static_cast<int>(result.trades.size()));
    for (size_t i = 0; i < result.trades.size(); ++i) {
        const Trade& t = result.trades[i];
        int row = static_cast<int>(i);
        tradeTable_->setItem(row, 0, new QTableWidgetItem(QString::number(t.getTradeId())));
        tradeTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(t.getEntryDate())));
        tradeTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(t.getExitDate())));
        tradeTable_->setItem(row, 3, new QTableWidgetItem(QString("$%1").arg(t.getEntryPrice(), 0, 'f', 2)));
        tradeTable_->setItem(row, 4, new QTableWidgetItem(QString("$%1").arg(t.getExitPrice(), 0, 'f', 2)));
        tradeTable_->setItem(row, 5, new QTableWidgetItem(QString::number(t.getQuantity())));
        tradeTable_->setItem(row, 6, new QTableWidgetItem(QString("$%1").arg(t.getNetPnL(), 0, 'f', 2)));
    }

    equityChart_->setEquityCurve(result.equityCurve, result.initialCapital);
}

} // namespace quantbench
