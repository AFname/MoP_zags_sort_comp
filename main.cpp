/**
 * @file main.cpp
 * @brief Сравнение алгоритмов сортировки на записях ЗАГС.
 *
 * Загружает CSV-файл с записями актов о заключении брака,
 * измеряет время работы четырёх алгоритмов сортировки
 * (выбором, шейкерной, быстрой и std::sort) на наборах
 * различного размера и сохраняет результаты в results.csv.
 */

#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>

 /**
  * @brief Преобразует дату из строки формата "DD.MM.YYYY" в целое число.
  *
  * Результирующее число строится по схеме YYYYMMDD, что позволяет
  * сравнивать даты простым целочисленным сравнением.
  *
  * @param date Строка с датой в формате "DD.MM.YYYY".
  * @return Целое число в формате YYYYMMDD.
  */
int dateToInt(const std::string& date) {
    int day = std::stoi(date.substr(0, 2));
    int month = std::stoi(date.substr(3, 2));
    int year = std::stoi(date.substr(6, 4));
    return year * 10000 + month * 100 + day;
}

/**
 * @brief Запись об акте о заключении брака из базы ЗАГС.
 *
 * Содержит персональные данные жениха и невесты, дату и номер
 * органа ЗАГС, зарегистрировавшего брак. Поддерживает полный
 * набор операторов сравнения; порядок сортировки:
 * номер ЗАГС → дата бракосочетания → ФИО жениха.
 *
 * @var ZagsRecord::groom_fio        ФИО жениха.
 * @var ZagsRecord::groom_birth      Дата рождения жениха (строка, формат DD.MM.YYYY).
 * @var ZagsRecord::bride_fio        ФИО невесты.
 * @var ZagsRecord::bride_birth      Дата рождения невесты (строка, формат DD.MM.YYYY).
 * @var ZagsRecord::marriage_date    Дата бракосочетания (строка, формат DD.MM.YYYY).
 * @var ZagsRecord::marriage_date_int Дата бракосочетания в числовом формате YYYYMMDD для быстрого сравнения.
 * @var ZagsRecord::zags_number      Номер органа ЗАГС.
 */
struct ZagsRecord {
    std::string groom_fio;
    std::string groom_birth;
    std::string bride_fio;
    std::string bride_birth;
    std::string marriage_date;
    int         marriage_date_int;
    int         zags_number;

    /**
     * @brief Трёхзначное сравнение двух записей.
     *
     * Сравнение выполняется последовательно по трём ключам:
     * номер ЗАГС, дата бракосочетания, ФИО жениха.
     *
     * @param o Запись, с которой производится сравнение.
     * @return -1, если текущая запись меньше; 1 — если больше; 0 — если равны.
     */
    int compare(const ZagsRecord& o) const {
        if (zags_number != o.zags_number)
            return zags_number < o.zags_number ? -1 : 1;

        if (marriage_date_int != o.marriage_date_int)
            return marriage_date_int < o.marriage_date_int ? -1 : 1;

        if (groom_fio != o.groom_fio)
            return groom_fio < o.groom_fio ? -1 : 1;

        return 0;
    }

    /** @brief Оператор равенства. @param o Правый операнд. @return true, если записи равны. */
    bool operator==(const ZagsRecord& o) const { return compare(o) == 0; }

    /** @brief Оператор «меньше». @param o Правый операнд. @return true, если текущая запись меньше. */
    bool operator< (const ZagsRecord& o) const { return compare(o) < 0; }

    /** @brief Оператор «больше». @param o Правый операнд. @return true, если текущая запись больше. */
    bool operator> (const ZagsRecord& o) const { return compare(o) > 0; }

    /** @brief Оператор «меньше или равно». @param o Правый операнд. @return true, если текущая запись не больше. */
    bool operator<=(const ZagsRecord& o) const { return compare(o) <= 0; }

    /** @brief Оператор «больше или равно». @param o Правый операнд. @return true, если текущая запись не меньше. */
    bool operator>=(const ZagsRecord& o) const { return compare(o) >= 0; }
};

/**
 * @brief Сортировка выбором с индексным массивом.
 *
 * Алгоритм работает с вектором индексов, избегая лишних перемещений
 * объектов на каждой итерации. По завершении переставляет элементы
 * в соответствии с построенной перестановкой индексов.
 *
 * Сложность: O(n^2) сравнений, O(n) обменов индексов.
 *
 * @param arr Вектор записей для сортировки; изменяется на месте.
 */
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

/**
 * @brief Шейкерная (двунаправленная пузырьковая) сортировка с индексным массивом.
 *
 * На каждой итерации выполняет два прохода: слева направо (продвигает
 * максимум к правой границе) и справа налево (продвигает минимум к левой
 * границе). Сужает рабочую область после каждого прохода.
 *
 * Сложность: O(n^2) сравнений и O(n^2) обменов индексов.
 *
 * @param arr Вектор записей для сортировки; изменяется на месте.
 */
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

/**
 * @brief Быстрая сортировка (схема Хоара) — рекурсивная реализация.
 *
 * В качестве опорного элемента (pivot) используется элемент из середины
 * диапазона. Разделяет подмассив на две части и рекурсивно сортирует
 * каждую. Элементы переставляются в исходном векторе напрямую, без
 * вспомогательного индексного массива.
 *
 * Средняя сложность: O(n log n); худший случай: O(n^2).
 *
 * @param arr   Вектор записей для сортировки; изменяется на месте.
 * @param left  Левая граница сортируемого диапазона (включительно).
 * @param right Правая граница сортируемого диапазона (включительно).
 */
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

/**
 * @brief Загружает записи ЗАГС из CSV-файла.
 *
 * Ожидаемый формат файла — заголовочная строка, затем строки данных
 * с полями, разделёнными запятыми, в порядке:
 * groom_fio, groom_birth, bride_fio, bride_birth, marriage_date, zags_number.
 *
 * @param filename Путь к CSV-файлу.
 * @param maxRows  Максимальное число считываемых строк данных;
 *                 -1 означает «без ограничений».
 * @return Вектор загруженных записей; пустой вектор при ошибке открытия файла.
 */
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

/**
 * @brief Точка входа программы.
 *
 * Выполняет следующие шаги:
 * -# Загружает полный датасет из файла zags_data.csv.
 * -# Для каждого размера выборки N из заданного списка:
 *    - Формирует подмножество из первых N записей.
 *    - Измеряет время (в миллисекундах) каждого из четырёх алгоритмов:
 *      сортировки выбором, шейкерной, быстрой и std::sort.
 *    - Выводит результаты в консоль и дописывает строку в results.csv.
 * -# Завершает работу, закрывая файл результатов.
 *
 * @return 0 при успешном завершении; 1 при ошибке ввода-вывода.
 */
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