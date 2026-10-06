#include <iostream>
#include <fstream>
#include <string>

bool generateNoSpacesFile(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream inFile {inputFilename};
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open " << inputFilename << '\n';
        return false;
    }

    std::ofstream outFile {outputFilename};
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not create " << outputFilename << '\n';
        return false;
    }

    std::string line {};
    bool inString {false};

    while (std::getline(inFile, line)) {
        for (char c : line) {
            if (c == '"') {
                inString = !inString;
                outFile << c;
            } else if (c == ' ' && !inString) {
                continue; 
            } else {
                outFile << c;
            }
        }
        outFile << '\n'; 
    }

    return true;
}

int main() {
    std::string sourceFile {"prog1.hl"};
    std::string outputFile {"nospaces.txt"};

    if (generateNoSpacesFile(sourceFile, outputFile)) {
        std::cout << "Successfully generated " << outputFile << " without stripping string literals.\n";
    }

    return 0;
}
