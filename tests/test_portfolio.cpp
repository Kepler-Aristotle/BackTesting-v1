#include <iostream>
#include <cassert>
#include <cmath>
#include "portfolio/Portfolio.h"
#include "structures/LinkedQueue.h"
#include "exceptions/CustomExceptions.h"

using namespace quantbench;

namespace {
bool approxEqual(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) < eps;
}
}

void runPortfolioTests() {
    std::cout << "[TEST SUITE] Running Portfolio & LinkedQueue<T> Tests...\n";

    // 1. Valid BUY & SELL with transaction fees
    {
        Portfolio p(10000.0);
        // Buy 10 shares @ $100 with 0.1% brokerage ($1000 + $1 fee = $1001 cost)
        p.buy(10, 100.0, 0.001, "2025-01-02");
        assert(p.getShares() == 10);
        assert(approxEqual(p.getCash(), 8999.0));
        assert(approxEqual(p.getEquity(110.0), 8999.0 + 1100.0));

        // Sell 10 shares @ $110 with 0.1% brokerage ($1100 - $1.10 fee = $1098.90 proceeds)
        Trade t = p.sell(10, 110.0, 0.001, "2025-01-05");
        assert(p.getShares() == 0);
        assert(approxEqual(t.getGrossPnL(), 100.0));
        assert(approxEqual(t.getTransactionFees(), 2.10));
        assert(approxEqual(t.getNetPnL(), 97.90));
        assert(approxEqual(p.getCash(), 10097.90));
        std::cout << "  ✓ Valid BUY, SELL, brokerage fees, and Equity calculation verified\n";
    }

    // 2. Insufficient funds rejection
    {
        Portfolio p(500.0);
        bool caught = false;
        try {
            p.buy(10, 100.0, 0.001, "2025-01-02");
        } catch (const OrderExecutionException&) {
            caught = true;
        }
        assert(caught && "Expected OrderExecutionException for insufficient funds");
        assert(p.getShares() == 0);
        assert(approxEqual(p.getCash(), 500.0));
        std::cout << "  ✓ Insufficient funds properly rejected without mutating cash\n";
    }

    // 3. Selling without an existing position
    {
        Portfolio p(10000.0);
        bool caught = false;
        try {
            p.sell(10, 100.0, 0.001, "2025-01-02");
        } catch (const OrderExecutionException&) {
            caught = true;
        }
        assert(caught && "Expected OrderExecutionException when selling with 0 shares");
        std::cout << "  ✓ Selling without an existing position rejected\n";
    }

    // 4. Custom LinkedQueue<T> FIFO, deep copy, and empty queue safety
    {
        LinkedQueue<int> q;
        assert(q.empty());
        q.enqueue(10);
        q.enqueue(20);
        q.enqueue(30);
        assert(q.size() == 3);
        assert(q.peek() == 10);

        // Deep copy verification
        LinkedQueue<int> copyQ(q);
        q.dequeue();
        assert(q.peek() == 20);
        assert(copyQ.peek() == 10); // Copy unaffected

        q.dequeue();
        q.dequeue();
        assert(q.empty());

        bool emptyCaught = false;
        try {
            q.dequeue();
        } catch (const EmptyQueueException&) {
            emptyCaught = true;
        }
        assert(emptyCaught && "Expected EmptyQueueException on empty dequeue");
        std::cout << "  ✓ LinkedQueue<T> FIFO, deep copy, and EmptyQueueException verified\n";
    }
}
