#ifndef QUANTBENCH_MAIN_WINDOW_H
#define QUANTBENCH_MAIN_WINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <vector>
#include <string>

#include "models/Candle.h"
#include "models/BacktestResult.h"
#include "EquityCurveWidget.h"

namespace quantbench {

/**
 * @brief Qt 6 Desktop MainWindow implementing Section 14 of the QuantBench specification.
 * Communicates strictly with the public interface of CsvDataSource, StrategyFactory, and BacktestEngine.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    std::string selectedCsvPath_;
    std::vector<Candle> loadedCandles_;

    // Data Section Widgets
    QPushButton* browseCsvBtn_;
    QLabel* filenameLabel_;
    QLabel* candleCountLabel_;
    QLabel* dateRangeLabel_;

    // Strategy & Parameters Widgets
    QComboBox* strategyCombo_;
    QSpinBox* shortSmaSpin_;
    QSpinBox* longSmaSpin_;
    QDoubleSpinBox* capitalSpin_;
    QSpinBox* quantitySpin_;
    QDoubleSpinBox* brokerageSpin_;
    QDoubleSpinBox* slippageSpin_;

    // Execution Section
    QPushButton* runBacktestBtn_;
    QLabel* statusLabel_;

    // Results Section
    QLabel* finalEquityLabel_;
    QLabel* netProfitLabel_;
    QLabel* totalReturnLabel_;
    QLabel* winRateLabel_;
    QLabel* maxDrawdownLabel_;

    // Trade History Table & Equity Curve
    QTableWidget* tradeTable_;
    EquityCurveWidget* equityChart_;

    void setupUi();
    void loadCsvFile(const std::string& path);
    void populateResults(const BacktestResult& result);

private slots:
    void onBrowseCsv();
    void onRunBacktest();

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;
};

} // namespace quantbench

#endif // QUANTBENCH_MAIN_WINDOW_H
