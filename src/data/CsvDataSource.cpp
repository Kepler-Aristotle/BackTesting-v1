#include "data/CsvDataSource.h"
#include "exceptions/CustomExceptions.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace quantbench {

std::string CsvDataSource::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

Candle CsvDataSource::parseRow(const std::string& line, size_t rowNumber) {
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> fields;

    while (std::getline(ss, token, ',')) {
        fields.push_back(trim(token));
    }

    // Check if line ended with a trailing comma
    if (!line.empty() && line.back() == ',') {
        fields.push_back("");
    }

    if (fields.size() != 6) {
        throw DataValidationException(rowNumber, "Expected 6 columns (Date,Open,High,Low,Close,Volume), found " + std::to_string(fields.size()) + ".");
    }

    const std::string& dateStr = fields[0];
    if (dateStr.size() != 10 || dateStr[4] != '-' || dateStr[7] != '-') {
        throw DataValidationException(rowNumber, "Invalid date format '" + dateStr + "' (expected YYYY-MM-DD).");
    }

    double open = 0.0, high = 0.0, low = 0.0, close = 0.0;
    long long volume = 0;

    try {
        size_t idx = 0;
        open = std::stod(fields[1], &idx);
        if (idx != fields[1].size()) throw std::invalid_argument("Trailing characters in Open");

        high = std::stod(fields[2], &idx);
        if (idx != fields[2].size()) throw std::invalid_argument("Trailing characters in High");

        low = std::stod(fields[3], &idx);
        if (idx != fields[3].size()) throw std::invalid_argument("Trailing characters in Low");

        close = std::stod(fields[4], &idx);
        if (idx != fields[4].size()) throw std::invalid_argument("Trailing characters in Close");

        volume = std::stoll(fields[5], &idx);
        if (idx != fields[5].size()) throw std::invalid_argument("Trailing characters in Volume");
    } catch (const std::exception& e) {
        throw DataValidationException(rowNumber, std::string("Malformed numerical field: ") + e.what());
    }

    Candle candle(dateStr, open, high, low, close, volume);
    candle.validate(rowNumber);
    return candle;
}

std::vector<Candle> CsvDataSource::load(const std::string& filename) const {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw DataFileException(filename, "File does not exist or cannot be opened.");
    }

    std::vector<Candle> candles;
    std::string line;
    size_t rowNumber = 0;
    bool headerProcessed = false;
    std::string previousDate = "";

    while (std::getline(file, line)) {
        ++rowNumber;
        std::string trimmedLine = trim(line);
        if (trimmedLine.empty()) {
            continue;
        }

        if (!headerProcessed) {
            headerProcessed = true;
            // Check if the first non-empty row is a header row (starts with non-digit or "Date")
            std::string lowerHeader = trimmedLine;
            std::transform(lowerHeader.begin(), lowerHeader.end(), lowerHeader.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            if (lowerHeader.find("date") == 0) {
                continue;
            }
        }

        Candle candle = parseRow(trimmedLine, rowNumber);

        // Enforce strict chronological ordering (ascending dates, no duplicates)
        if (!previousDate.empty() && candle.date <= previousDate) {
            throw DataValidationException(
                rowNumber,
                "Non-chronological or duplicate date '" + candle.date + "' following '" + previousDate + "'."
            );
        }

        previousDate = candle.date;
        candles.push_back(candle);
    }

    if (candles.empty()) {
        throw DataValidationException("CSV file '" + filename + "' contains no valid historical candle rows.");
    }

    return candles;
}

std::vector<Candle> CsvDataSource::load(const std::string& filename,
                                        const std::string& startDate,
                                        const std::string& endDate) const {
    if (!startDate.empty() && !endDate.empty() && startDate > endDate) {
        throw DataValidationException("Invalid date range filter: startDate '" + startDate + "' is after endDate '" + endDate + "'.");
    }

    std::vector<Candle> allCandles = load(filename);
    std::vector<Candle> filtered;

    for (const auto& c : allCandles) {
        bool afterStart = startDate.empty() || (c.date >= startDate);
        bool beforeEnd = endDate.empty() || (c.date <= endDate);
        if (afterStart && beforeEnd) {
            filtered.push_back(c);
        }
    }

    if (filtered.empty()) {
        throw DataValidationException("No candles matched the requested date range [" + startDate + " to " + endDate + "].");
    }

    return filtered;
}

} // namespace quantbench
