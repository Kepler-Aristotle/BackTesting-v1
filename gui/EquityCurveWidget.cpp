#include "EquityCurveWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <algorithm>

namespace quantbench {

EquityCurveWidget::EquityCurveWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumHeight(220);
}

void EquityCurveWidget::setEquityCurve(const std::vector<EquitySnapshot>& curve, double initialCapital) {
    curve_ = curve;
    initialCapital_ = initialCapital;
    update();
}

void EquityCurveWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();
    const int padLeft = 65;
    const int padRight = 20;
    const int padTop = 20;
    const int padBottom = 30;

    // Background fill
    painter.fillRect(rect(), QColor(15, 23, 42));

    const int plotW = w - padLeft - padRight;
    const int plotH = h - padTop - padBottom;
    if (plotW <= 10 || plotH <= 10) return;

    // Draw grid lines
    painter.setPen(QPen(QColor(51, 65, 85), 1, Qt::DashLine));
    for (int i = 0; i <= 4; ++i) {
        int y = padTop + (plotH * i) / 4;
        painter.drawLine(padLeft, y, padLeft + plotW, y);
    }

    if (curve_.empty()) {
        painter.setPen(QColor(148, 163, 184));
        painter.drawText(rect(), Qt::AlignCenter, "Load a CSV and click 'Run Backtest' to view Equity Curve");
        return;
    }

    double minEq = initialCapital_;
    double maxEq = initialCapital_;
    for (const auto& pt : curve_) {
        minEq = std::min(minEq, pt.totalEquity);
        maxEq = std::max(maxEq, pt.totalEquity);
    }
    double range = maxEq - minEq;
    if (range < 1.0) range = 1.0;

    // Draw Y-axis labels
    painter.setPen(QColor(148, 163, 184));
    painter.drawText(5, padTop + 10, QString("$%1").arg(maxEq, 0, 'f', 0));
    painter.drawText(5, padTop + plotH, QString("$%1").arg(minEq, 0, 'f', 0));

    // Draw Equity polyline
    QPainterPath path;
    const size_t n = curve_.size();
    for (size_t i = 0; i < n; ++i) {
        double x = padLeft + (n > 1 ? (static_cast<double>(i) * plotW) / static_cast<double>(n - 1) : plotW / 2.0);
        double y = padTop + plotH - ((curve_[i].totalEquity - minEq) / range) * plotH;
        if (i == 0) path.moveTo(x, y);
        else path.lineTo(x, y);
    }

    painter.setPen(QPen(QColor(16, 185, 129), 2.5));
    painter.drawPath(path);

    // Date range labels at bottom
    painter.setPen(QColor(148, 163, 184));
    painter.drawText(padLeft, h - 8, QString::fromStdString(curve_.front().date));
    painter.drawText(padLeft + plotW - 75, h - 8, QString::fromStdString(curve_.back().date));
}

} // namespace quantbench
