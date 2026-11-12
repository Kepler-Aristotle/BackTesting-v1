#ifndef QUANTBENCH_ORDER_H
#define QUANTBENCH_ORDER_H

#include <string>
#include <utility>

namespace quantbench {

/**
 * @brief Trading signal emitted by a Strategy.
 */
enum class Signal {
    BUY,
    SELL,
    HOLD
};

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderStatus {
    PENDING,
    EXECUTED,
    REJECTED,
    DISCARDED
};

/**
 * @brief Represents a simulated BUY or SELL order queued for next-candle Open execution.
 */
class Order {
private:
    static int nextOrderId_;

    int orderId_;
    OrderSide side_;
    int quantity_;
    std::string signalDate_;
    OrderStatus status_;
    std::string statusReason_;

public:
    Order()
        : orderId_(++nextOrderId_),
          side_(OrderSide::BUY),
          quantity_(0),
          signalDate_(""),
          status_(OrderStatus::PENDING),
          statusReason_("") {}

    Order(OrderSide side, int quantity, std::string signalDate)
        : orderId_(++nextOrderId_),
          side_(side),
          quantity_(quantity),
          signalDate_(std::move(signalDate)),
          status_(OrderStatus::PENDING),
          statusReason_("Queued for next candle open") {}

    Order(int explicitId, OrderSide side, int quantity, std::string signalDate,
          OrderStatus status, std::string reason = "")
        : orderId_(explicitId),
          side_(side),
          quantity_(quantity),
          signalDate_(std::move(signalDate)),
          status_(status),
          statusReason_(std::move(reason)) {
        if (explicitId > nextOrderId_) {
            nextOrderId_ = explicitId;
        }
    }

    int getOrderId() const noexcept { return orderId_; }
    OrderSide getSide() const noexcept { return side_; }
    int getQuantity() const noexcept { return quantity_; }
    const std::string& getSignalDate() const noexcept { return signalDate_; }
    OrderStatus getStatus() const noexcept { return status_; }
    const std::string& getStatusReason() const noexcept { return statusReason_; }

    void markExecuted(const std::string& executionDate) {
        status_ = OrderStatus::EXECUTED;
        statusReason_ = "Executed at open on " + executionDate;
    }

    void markRejected(const std::string& reason) {
        status_ = OrderStatus::REJECTED;
        statusReason_ = reason;
    }

    void markDiscarded(const std::string& reason) {
        status_ = OrderStatus::DISCARDED;
        statusReason_ = reason;
    }

    static void resetIdCounter(int start = 0) noexcept {
        nextOrderId_ = start;
    }

    static int getCurrentCounter() noexcept {
        return nextOrderId_;
    }

    static std::string sideToString(OrderSide side) {
        return side == OrderSide::BUY ? "BUY" : "SELL";
    }

    static std::string statusToString(OrderStatus status) {
        switch (status) {
            case OrderStatus::PENDING: return "PENDING";
            case OrderStatus::EXECUTED: return "EXECUTED";
            case OrderStatus::REJECTED: return "REJECTED";
            case OrderStatus::DISCARDED: return "DISCARDED";
        }
        return "UNKNOWN";
    }
};

inline int Order::nextOrderId_ = 0;

} // namespace quantbench

#endif // QUANTBENCH_ORDER_H
