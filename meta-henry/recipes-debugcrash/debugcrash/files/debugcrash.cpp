#include <iostream>

int main() {
    std::cout << "Debug Crash Tool Starting..." << std::endl;
    
    int* ptr = nullptr;
    
    std::cout << "Attempting to read null pointer..." << std::endl;
    
    // Segmentation Fault
    std::cout << "Value: " << *ptr << std::endl; 
    
    return 0;
}
