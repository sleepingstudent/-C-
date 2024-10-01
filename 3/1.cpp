#include <iostream>
#include <memory>

// мой аллокатор
template<typename T>
struct MyAllocator {
    using value_type = T;

    MyAllocator() = default;

    template<typename U>
    MyAllocator(const MyAllocator<U>&) {}

    T* allocate(std::size_t n) {
        std::cout << "Custom allocator allocate!" << std::endl;
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) {
        std::cout << "Custom allocator deallocate!" << std::endl;
        ::operator delete(p);
    }
};

template<typename T, typename U>
bool operator==(const MyAllocator<T>&, const MyAllocator<U>&) { return true; }

template<typename T, typename U>
bool operator!=(const MyAllocator<T>&, const MyAllocator<U>&) { return false; }

class A {
public:
    // Перегружаем оператор new
    static void* operator new(std::size_t size) {
        std::cout << "operator new!" << std::endl;
        return ::operator new(size);
    }

    // Перегружаем оператор delete
    static void operator delete(void* p, std::size_t size) {
        std::cout << "operator delete!" << std::endl;
        ::operator delete(p);
    }
};

int main() {
    // Создаём объект A с помощью allocate_shared и пользовательского аллокатора
    auto sp = std::allocate_shared<A>(MyAllocator<A>());
    return 0;
}
