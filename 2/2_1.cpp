#include <iostream>
#include <vector>
#include <memory>

int main() {
    std::vector<std::unique_ptr<int>> v;
    v.push_back(std::make_unique<int>(10));
    v.push_back(std::make_unique<int>(20));
    v.push_back(std::make_unique<int>(30));

    // Пример с auto&& (работает как с lvalue, так и с rvalue)
    std::cout << "Using auto&&:" << std::endl;
    for (auto&& x : v) {
        std::cout << *x << std::endl;  // Важно: x может быть lvalue или rvalue
    }

    // Пример с auto& (работает только с lvalue)
    std::cout << "Using auto&:" << std::endl;
    for (auto& x : v) {
        std::cout << *x << std::endl;  // Важно: x всегда должен быть lvalue
    }

    // Пример передачи rvalue контейнера
    std::cout << "Using auto&& with rvalue vector:" << std::endl;
    for (auto&& x : std::vector<std::unique_ptr<int>>{
            std::make_unique<int>(40), std::make_unique<int>(50)}) {
        std::cout << *x << std::endl;  // Это скомпилируется, так как auto&& может принять rvalue
    }

    // Пример с auto& и rvalue (не скомпилируется)
    // std::cout << "Using auto& with rvalue vector:" << std::endl;
    // for (auto& x : std::vector<std::unique_ptr<int>>{
    //        std::make_unique<int>(60), std::make_unique<int>(70)}) {
    //    std::cout << *x << std::endl;  // Ошибка компиляции, т.к. нельзя привязать lvalue-ссылку к rvalue
    // }

    return 0;
}
