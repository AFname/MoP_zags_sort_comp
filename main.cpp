#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>

int dateToInt(const std::string& date) {
    int day = std::stoi(date.substr(0, 2));
    int month = std::stoi(date.substr(3, 2));
    int year = std::stoi(date.substr(6, 4));
    return year * 10000 + month * 100 + day;
}

struct ZagsRecord {
    std::string groom_fio;
    std::string groom_birth;
    std::string bride_fio;
    std::string bride_birth;
    std::string marriage_date;
    int         marriage_date_int;
    int         zags_number;

    int compare(const ZagsRecord& o) const {
        if (zags_number != o.zags_number)
            return zags_number < o.zags_number ? -1 : 1;

        if (marriage_date_int != o.marriage_date_int)
            return marriage_date_int < o.marriage_date_int ? -1 : 1;

        if (groom_fio != o.groom_fio)
            return groom_fio < o.groom_fio ? -1 : 1;

        return 0;
    }

    bool operator==(const ZagsRecord& o) const { return compare(o) == 0; }

    bool operator< (const ZagsRecord& o) const { return compare(o) < 0; }

    bool operator> (const ZagsRecord& o) const { return compare(o) > 0; }

    bool operator<=(const ZagsRecord& o) const { return compare(o) <= 0; }

    bool operator>=(const ZagsRecord& o) const { return compare(o) >= 0; }
};

void selectionSort(std::vector<ZagsRecord>& arr) {
    int n = arr.size();
    std::vector<int> idx(n);
    std::iota(idx.begin(), idx.end(), 0);

    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++)
            if (arr[idx[j]] < arr[idx[minIdx]])
                minIdx = j;
        if (minIdx != i)
            std::swap(idx[i], idx[minIdx]);
    }

    std::vector<ZagsRecord> sorted(n);
    for (int i = 0; i < n; i++) sorted[i] = arr[idx[i]];
    arr = std::move(sorted);
}


void shakerSort(std::vector<ZagsRecord>& arr) {
    int n = arr.size();
    std::vector<int> idx(n);
    std::iota(idx.begin(), idx.end(), 0);

    int left = 0, right = n - 1;
    while (left < right) {
        for (int i = left; i < right; i++)
            if (arr[idx[i]] > arr[idx[i + 1]])
                std::swap(idx[i], idx[i + 1]);
        right--;
        for (int i = right; i > left; i--)
            if (arr[idx[i]] < arr[idx[i - 1]])
                std::swap(idx[i], idx[i - 1]);
        left++;
    }

    std::vector<ZagsRecord> sorted(n);
    for (int i = 0; i < n; i++) sorted[i] = arr[idx[i]];
    arr = std::move(sorted);
}


void quickSort(std::vector<ZagsRecord>& arr, int left, int right) {
    if (left >= right) return;
    int i = left, j = right;
    ZagsRecord pivot = arr[(left + right) / 2];
    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) {
            std::swap(arr[i], arr[j]);
            i++; j--;
        }
    }
    if (left < j)  quickSort(arr, left, j);
    if (i < right) quickSort(arr, i, right);
}


std::vector<ZagsRecord> loadCSV(const std::string& filename, int maxRows = -1) {
    std::vector<ZagsRecord> result;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл \"" << filename << "\"\n";
        return result;
    }
    std::string line;
    std::getline(file, line);
    int rowsRead = 0;
    while (std::getline(file, line)) {
        if (maxRows != -1 && rowsRead >= maxRows) break;
        if (line.empty()) continue;
        std::istringstream ss(line);
        ZagsRecord rec;
        std::string token;
        std::getline(ss, rec.groom_fio, ',');
        std::getline(ss, rec.groom_birth, ',');
        std::getline(ss, rec.bride_fio, ',');
        std::getline(ss, rec.bride_birth, ',');
        std::getline(ss, rec.marriage_date, ',');
        std::getline(ss, token, ',');
        rec.zags_number = std::stoi(token);
        rec.marriage_date_int = dateToInt(rec.marriage_date);
        result.push_back(rec);
        ++rowsRead;
    }
    return result;
}


int main() {

    std::vector<ZagsRecord> fullData = loadCSV("zags_data.csv");
    if (fullData.empty()) {
        std::cerr << "Ошибка: не удалось загрузить zags_data.csv или файл пуст.\n";
        return 1;
    }

    const std::vector<int> sizes = {
        100, 250, 500, 750, 1000, 1500,
        2000, 3000, 5000, 7500, 10000,
        15000, 20000, 50000, 100000
    };

    std::ofstream out("results.csv");
    if (!out.is_open()) {
        std::cerr << "Ошибка: не удалось открыть results.csv для записи.\n";
        return 1;
    }
    out << "size,selection,shaker,quick,std\n";
    out << std::fixed << std::setprecision(2);

    for (int N : sizes) {
        if (N > static_cast<int>(fullData.size())) {
            std::cerr << "Пропуск N=" << N << ": датасет содержит только "
                << fullData.size() << " записей.\n";
            continue;
        }

        std::vector<ZagsRecord> base(fullData.begin(), fullData.begin() + N);

        auto measure = [&](auto sortFn) -> double {
            std::vector<ZagsRecord> copy = base;
            auto t0 = std::chrono::high_resolution_clock::now();
            sortFn(copy);
            auto t1 = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double, std::milli>(t1 - t0).count();
            };

        double tSel = measure([](std::vector<ZagsRecord>& v) { selectionSort(v); });
        double tShaker = measure([](std::vector<ZagsRecord>& v) { shakerSort(v); });
        double tQuick = measure([](std::vector<ZagsRecord>& v) { quickSort(v, 0, (int)v.size() - 1); });
        double tStd = measure([](std::vector<ZagsRecord>& v) { std::sort(v.begin(), v.end()); });

        std::cout << std::fixed << std::setprecision(2)
            << "N=" << N
            << ": sel=" << tSel << "ms"
            << " shaker=" << tShaker << "ms"
            << " quick=" << tQuick << "ms"
            << " std=" << tStd << "ms\n";

        out << N << "," << tSel << "," << tShaker << "," << tQuick << "," << tStd << "\n";
    }

    out.close();
    std::cout << "\nГотово. Результаты записаны в results.csv\n";
    return 0;
}
