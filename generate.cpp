#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <tuple>


static const std::vector<std::string> MALE_NAMES = {
    "Александр","Алексей","Андрей","Артём","Борис",
    "Василий","Виктор","Владимир","Геннадий","Григорий",
    "Дмитрий","Евгений","Иван","Игорь","Илья",
    "Кирилл","Константин","Максим","Михаил","Никита",
    "Николай","Олег","Павел","Роман","Сергей",
    "Степан","Тимур","Фёдор","Юрий","Яков"
};

static const std::vector<std::string> FEMALE_NAMES = {
    "Александра","Алина","Анастасия","Анна","Валентина",
    "Валерия","Вера","Виктория","Галина","Дарья",
    "Екатерина","Елена","Жанна","Зоя","Ирина",
    "Карина","Кристина","Ксения","Людмила","Маргарита",
    "Марина","Мария","Надежда","Наталья","Нина",
    "Оксана","Ольга","Светлана","Татьяна","Юлия"
};

static const std::vector<std::string> MALE_PATRONYMICS = {
    "Александрович","Алексеевич","Андреевич","Борисович","Васильевич",
    "Викторович","Владимирович","Геннадьевич","Григорьевич","Дмитриевич",
    "Евгеньевич","Иванович","Игоревич","Ильич","Кириллович",
    "Константинович","Максимович","Михайлович","Николаевич","Олегович",
    "Павлович","Романович","Сергеевич","Степанович","Фёдорович"
};

static const std::vector<std::string> FEMALE_PATRONYMICS = {
    "Александровна","Алексеевна","Андреевна","Борисовна","Васильевна",
    "Викторовна","Владимировна","Геннадьевна","Григорьевна","Дмитриевна",
    "Евгеньевна","Ивановна","Игоревна","Ильинична","Кирилловна",
    "Константиновна","Максимовна","Михайловна","Николаевна","Олеговна",
    "Павловна","Романовна","Сергеевна","Степановна","Фёдоровна"
};

static const std::vector<std::string> SURNAMES_MALE = {
    "Иванов","Смирнов","Кузнецов","Попов","Васильев",
    "Петров","Соколов","Михайлов","Новиков","Фёдоров",
    "Морозов","Волков","Алексеев","Лебедев","Семёнов",
    "Егоров","Павлов","Козлов","Степанов","Николаев",
    "Орлов","Андреев","Макаров","Никитин","Захаров",
    "Зайцев","Соловьёв","Борисов","Яковлев","Григорьев"
};

// Возвращает женскую форму фамилии
static std::string toFemaleSurname(const std::string& male) {
    if (male.size() >= 2) {
        auto endsWith = [&](const std::string& suffix) {
            return male.size() >= suffix.size() &&
                male.compare(male.size() - suffix.size(), suffix.size(), suffix) == 0;
            };
        if (endsWith("ов"))  return male + "а";
        if (endsWith("ев"))  return male + "а";
        if (endsWith("ин"))  return male + "а";
        if (endsWith("ын"))  return male + "а";
    }
    return male + "а";
}

struct Date { int day, month, year; };

static bool isLeap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static int daysInMonth(int m, int y) {
    static const int days[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
    if (m == 2 && isLeap(y)) return 29;
    return days[m];
}

static Date randomDate(std::mt19937& rng, int yearMin, int yearMax) {
    std::uniform_int_distribution<int> yearDist(yearMin, yearMax);
    std::uniform_int_distribution<int> monthDist(1, 12);
    int y = yearDist(rng);
    int m = monthDist(rng);
    std::uniform_int_distribution<int> dayDist(1, daysInMonth(m, y));
    return { dayDist(rng), m, y };
}

static std::string formatDate(const Date& d) {
    std::ostringstream ss;
    ss << std::setfill('0')
        << std::setw(2) << d.day << '.'
        << std::setw(2) << d.month << '.'
        << d.year;
    return ss.str();
}


template<typename T>
static const T& pick(std::mt19937& rng, const std::vector<T>& v) {
    std::uniform_int_distribution<size_t> d(0, v.size() - 1);
    return v[d(rng)];
}


static std::string groomFIO(std::mt19937& rng) {
    return pick(rng, SURNAMES_MALE) + " " +
        pick(rng, MALE_NAMES) + " " +
        pick(rng, MALE_PATRONYMICS);
}

static std::string brideFIO(std::mt19937& rng) {
    return toFemaleSurname(pick(rng, SURNAMES_MALE)) + " " +
        pick(rng, FEMALE_NAMES) + " " +
        pick(rng, FEMALE_PATRONYMICS);
}


static Date birthDate(std::mt19937& rng, const Date& wedding) {
    int birthYearMax = wedding.year - 18;
    int birthYearMin = wedding.year - 60;
    return randomDate(rng, birthYearMin, birthYearMax);
}

int main() {
    constexpr int RECORDS = 100'000;
    constexpr int ZAGS_MAX = 50;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> zagsDist(1, ZAGS_MAX);

    std::ofstream out("zags_data.csv");
    if (!out) {
        std::cerr << "Не удалось открыть zags_data.csv для записи\n";
        return 1;
    }

    // Заголовок
    out << "groom_fio,groom_birth,bride_fio,bride_birth,marriage_date,zags_number\n";

    for (int i = 0; i < RECORDS; ++i) {
        Date wedding = randomDate(rng, 2000, 2024);
        Date groomDOB = birthDate(rng, wedding);
        Date brideDOB = birthDate(rng, wedding);
        int  zags = zagsDist(rng);

        out << groomFIO(rng) << ','
            << formatDate(groomDOB) << ','
            << brideFIO(rng) << ','
            << formatDate(brideDOB) << ','
            << formatDate(wedding) << ','
            << zags << '\n';
    }

    out.close();
    std::cout << "Готово: zags_data.csv (" << RECORDS << " записей)\n";
    return 0;
}