#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdio>
#include "data/CsvDataSource.h"
#include "exceptions/CustomExceptions.h"

using namespace quantbench;

namespace {
void writeTempFile(const std::string& path, const std::string& content) {
    std::ofstream out(path);
    out << content;
}
}

void runCsvTests() {
    std::cout << "[TEST SUITE] Running CSV Data Layer Tests...\n";
    CsvDataSource loader;

    // 1. Valid CSV Loading
    {
        auto candles = loader.load("data/sample_data.csv");
        assert(candles.size() == 25);
        assert(candles.front().date == "2025-01-01");
        assert(candles.back().date == "2025-01-25");
        assert(candles[0].open == 100.0);
        std::cout << "  ✓ Valid CSV loaded (25 rows)\n";
    }

    // 2. Date-range overloaded load(filename, startDate, endDate)
    {
        auto filtered = loader.load("data/sample_data.csv", "2025-01-05", "2025-01-10");
        assert(filtered.size() == 6);
        assert(filtered.front().date == "2025-01-05");
        assert(filtered.back().date == "2025-01-10");
        std::cout << "  ✓ Overloaded date-range filtering verified\n";
    }

    // 3. Missing CSV file
    {
        bool caught = false;
        try {
            loader.load("data/non_existent_file_999.csv");
        } catch (const DataFileException&) {
            caught = true;
        }
        assert(caught && "Expected DataFileException for missing CSV");
        std::cout << "  ✓ Missing CSV throws DataFileException\n";
    }

    // 4. Malformed row (wrong column count / non-numeric)
    {
        const std::string badFile = "reports/tmp_bad_row.csv";
        writeTempFile(badFile, "Date,Open,High,Low,Close,Volume\n2025-01-01,100,105,abc,102,1000\n");
        bool caught = false;
        try {
            loader.load(badFile);
        } catch (const DataValidationException& e) {
            caught = (e.getRowNumber() == 2);
        }
        std::remove(badFile.c_str());
        assert(caught && "Expected DataValidationException at row 2 for malformed number");
        std::cout << "  ✓ Malformed row rejected with exact row number (row 2)\n";
    }

    // 5. Invalid OHLC values (High < Low or negative volume)
    {
        const std::string badOhlc = "reports/tmp_bad_ohlc.csv";
        writeTempFile(badOhlc, "Date,Open,High,Low,Close,Volume\n2025-01-01,100,95,98,96,1000\n");
        bool caught = false;
        try {
            loader.load(badOhlc);
        } catch (const DataValidationException&) {
            caught = true;
        }
        std::remove(badOhlc.c_str());
        assert(caught && "Expected DataValidationException for High < Open/Low");
        std::cout << "  ✓ Inconsistent OHLC values rejected\n";
    }

    // 6. Non-chronological or duplicate dates
    {
        const std::string badOrder = "reports/tmp_bad_order.csv";
        writeTempFile(badOrder,
            "Date,Open,High,Low,Close,Volume\n"
            "2025-01-02,100,105,98,103,1000\n"
            "2025-01-01,103,108,101,106,1200\n");
        bool caught = false;
        try {
            loader.load(badOrder);
        } catch (const DataValidationException& e) {
            caught = (e.getRowNumber() == 3);
        }
        std::remove(badOrder.c_str());
        assert(caught && "Expected DataValidationException for non-chronological date");
        std::cout << "  ✓ Non-chronological/duplicate dates rejected\n";
    }
}
