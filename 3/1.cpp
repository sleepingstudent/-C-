#include <iostream>
#include <memory>

// Пользовательский аллокатор
template <typename T, typename A = T>
struct MyAllocator {
    typedef T value_type;

    MyAllocator() noexcept {}

    template <class U>
    MyAllocator(const MyAllocator<U>&) noexcept {}

    // Вызов кастомного оператора new из класса A
    T* allocate(std::size_t n) {
        std::cout << "Custom allocator allocate!" << std::endl;
        return static_cast<T*>(A::operator new(sizeof(T) * n));
    }

    // Вызов кастомного оператора delete из класса A
    void deallocate(T* p, std::size_t n) {
        std::cout << "Custom allocator deallocate!" << std::endl;
        return A::operator delete(p, sizeof(T) * n);
    }
};

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
    std::shared_ptr<A> sp = std::allocate_shared<A>(MyAllocator<A>());
    return 0;
}
