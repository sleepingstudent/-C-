#include <iostream>
void f(int&& x) {
++x;
}
int main() {
int a = 0;
f(std::move(a));
std::cout << a << std::endl;
}