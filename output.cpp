#include <iostream>
#include <string>

int main() {
    std::cout << "test bool" << " " << testbool << std::endl;
    std::cout << "test int" << " " << testint << std::endl;
    std::cout << "test str 1" << " " << test2 << std::endl;
    std::cout << "test str 2" << " " << test1 << std::endl;
    if (testbool) {
        std::cout << "if statement is truee" << std::endl;
        if (! testboolf) {
            std::cout << "if statement is false.... " << std::endl;
            std::cout << testint + testint2 << std::endl;
        }
    }
    return 0;
}
