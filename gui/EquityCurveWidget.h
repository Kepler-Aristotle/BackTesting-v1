#ifndef QUANTBENCH_EQUITY_CURVE_WIDGET_H
#define QUANTBENCH_EQUITY_CURVE_WIDGET_H

#include <QWidget>
#include <vector>
#include "models/BacktestResult.h"

namespace quantbench {

/**
 * @brief Custom Qt 6 Widget that renders the portfolio historical equity curve
 * using QPainter without requiring external chart add-ons.
 */
class EquityCurveWidget : public QWidget {
    Q_OBJECT

private:
    std::vector<EquitySnapshot> curve_;
    double initialCapital_ = 100000.0;

public:
    explicit EquityCurveWidget(QWidget* parent = nullptr);
    ~EquityCurveWidget() override = default;

    void setEquityCurve(const std::vector<EquitySnapshot>& curve, double initialCapital);

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace quantbench

#endif // QUANTBENCH_EQUITY_CURVE_WIDGET_H
