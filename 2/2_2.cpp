#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <tuple>
#include <utility>

// Основной класс для форматирования
struct TPrettyPrinter {
    std::stringstream ss;  // Строковый поток для накопления строки

    // Метод для получения строки
    std::string Str() const {
        return ss.str();
    }

    // Функция форматирования для строк
    TPrettyPrinter& Format(const std::string& t) {
        ss << t;
        return *this;
    }

    // Функция форматирования для целых чисел (int)
    TPrettyPrinter& Format(const int& t) {
        ss << t;
        return *this;
    }

    // Функция форматирования для пар
    template<typename T1, typename T2>
    TPrettyPrinter& Format(const std::pair<T1, T2>& p) {
        ss << "(";
        Format(p.first).ss << ", ";
        Format(p.second).ss << ")";
        return *this;
    }

    // Функция форматирования для векторов
    template<typename T>
    TPrettyPrinter& Format(const std::vector<T>& v) {
        ss << "[";
        for (size_t i = 0; i < v.size(); ++i) {
            if (i > 0) ss << ", ";
            Format(v[i]);
        }
        ss << "]";
        return *this;
    }

    // Функция форматирования для множеств
    template<typename T>
    TPrettyPrinter& Format(const std::set<T>& s) {
        ss << "{";
        for (auto it = s.begin(); it != s.end(); ++it) {
            if (it != s.begin()) ss << ", ";
            Format(*it);
        }
        ss << "}";
        return *this;
    }

    // Функция форматирования для кортежей (tuple)
    template<typename... Args>
    TPrettyPrinter& Format(const std::tuple<Args...>& t) {
        ss << "(";
        std::apply([this](const auto&... args) {
            size_t n = 0;
            ((ss << (n++ ? ", " : "") << args), ...);
        }, t);
        ss << ")";
        return *this;
    }
};

// Шаблонная функция, вызывающая класс для форматирования
template<typename T>
std::string Format(const T& t) {
    return TPrettyPrinter().Format(t).Str();
}

int main() {
    std::tuple<std::string, int, int> t = {"xyz", 1, 2};
    std::vector<std::pair<int, int>> v = {{1, 4}, {5, 6}};

    std::string s1 = TPrettyPrinter().Format("vector: ").Format(v).Str();
    std::cout << s1 << std::endl;  // "vector: [(1, 4), (5, 6)]"

    std::string s2 = TPrettyPrinter().Format(t).Format(" ! ").Format(0).Str();
    std::cout << s2 << std::endl;  // "(xyz, 1, 2) ! 0"

    return 0;
}
