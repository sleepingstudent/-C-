#include <iostream>
#include <stdexcept>
#include <gtest/gtest.h>
#include <sstream>

class TMyException : public std::runtime_error {
public:
    TMyException() : std::runtime_error("Custom Exception") {}

    // Оператор << для записи сопровождающего текста
    template <typename T>
    TMyException& operator<<(const T& value) {
        std::ostringstream stream;
        stream << value;
        additionalInfo += stream.str();
        return *this;
    }

    const char* what() const noexcept override {
        return additionalInfo.c_str();
    }

private:
    std::string additionalInfo;
};

// Производные классы
class DerivedException1 : public TMyException {};
class DerivedException2 : public TMyException {};

TEST(CustomExceptionTest, Exception1) {
    EXPECT_THROW(throw DerivedException1() << "Exception 1 occurred", TMyException);
}

TEST(CustomExceptionTest, Exception2) {
    EXPECT_THROW(throw DerivedException2() << "Exception 2 occurred", TMyException);
}

int main(int argc, char** argv) {
    try {
        throw TMyException() << "Invalid argument x, got " << 42 << " but expected " << 100;
    } catch (const TMyException& ex) {
        std::cout << ex.what() << std::endl;
    }

    try {
        throw DerivedException1() << "Something went wrong in DerivedException1";
    } catch (const TMyException& ex) {
        std::cout << ex.what() << std::endl;
    }

    try {
        throw DerivedException2() << "Something went wrong in DerivedException2";
    } catch (const TMyException& ex) {
        std::cout << ex.what() << std::endl;
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
