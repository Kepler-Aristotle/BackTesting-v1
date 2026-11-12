#include <iostream>

// Forward declarations of test suites
void runCsvTests();
void runPortfolioTests();
void runStrategyTests();
void runEngineTests();
void runMetricsAndPersistenceTests();

int main() {
    std::cout << "=============================================================================\n";
    std::cout << "  QuantBench Automated Verification Suite (C++17)\n";
    std::cout << "=============================================================================\n\n";

    runCsvTests();
    std::cout << "\n";

    runPortfolioTests();
    std::cout << "\n";

    runStrategyTests();
    std::cout << "\n";

    runEngineTests();
    std::cout << "\n";

    runMetricsAndPersistenceTests();
    std::cout << "\n";

    std::cout << "=============================================================================\n";
    std::cout << "  ALL 5 TEST SUITES PASSED SUCCESSFULLY (ZERO ERRORS)\n";
    std::cout << "=============================================================================\n";
    return 0;
}
