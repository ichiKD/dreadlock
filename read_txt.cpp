#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ifstream file("example.txt"); // Open the file

    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) { // Read lines until the end of the file
        if(line != "")
            std::cout << line << std::endl; // Output the line
        else
            std::cout << "yes" << std::endl;
    }

    file.close(); // Close the file

    return 0;
}