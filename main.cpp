#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>

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

bool generateSymbolsFile(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream inFile {inputFilename};
    if (!inFile.is_open()) return false;
    
    std::ofstream outFile {outputFilename};
    if (!outFile.is_open()) return false;

    std::string line {};

    std::vector<std::string> reserved {"integer", "double", "if", "output"};
    std::vector<std::string> symbols {":=", "<<", "==", "!=", "<", ">", "+", "-", ":", ";", "(", ")", "="};

    while (std::getline(inFile, line)) {
        size_t i {0};
        while (i < line.length()) {
            char c = line[i];

            if (c == '"') {
                i++;
                while (i < line.length() && line[i] != '"') i++;
                i++; 
                continue;
            }

            if (std::isdigit(c)) {
                while (i < line.length() && (std::isdigit(line[i]) || line[i] == '.')) i++;
                continue;
            }

            if (std::isalpha(c)) {
                std::string word {};
                while (i < line.length() && std::isalpha(line[i])) {
                    word += line[i];
                    i++;
                }

                for (const auto& res : reserved) {
                    if (word == res) {
                        outFile << word << '\n';
                        std::cout << "Lexer found keyword: " << word << '\n';
                        break;
                    }
                }
                continue;
            }

            std::string sym {};
            bool foundTwoChar {false};
            
            if (i + 1 < line.length()) {
                sym = std::string(1, c) + line[i+1];
                for (const auto& s : symbols) {
                    if (sym == s && sym.length() == 2) {
                        outFile << sym << '\n';
                        std::cout << "Lexer found symbol:  " << sym << '\n';
                        i += 2;
                        foundTwoChar = true;
                        break;
                    }
                }
            }
            if (foundTwoChar) continue;

            sym = std::string(1, c);
            for (const auto& s : symbols) {
                if (sym == s && sym.length() == 1) {
                    outFile << sym << '\n';
                    std::cout << "Lexer found symbol:  " << sym << '\n';
                    break;
                }
            }
            i++;
        }
    }
    return true;
}

int main() {
    std::cout << "Select a program to compile:\n";
    std::cout << "1. prog1.hl\n";
    std::cout << "2. prog2.hl\n";
    std::cout << "3. prog3.hl\n";
    std::cout << "Enter your choice (1-3): ";

    int choice {};
    std::cin >> choice;

    std::string sourceFile {};

    if (choice == 1) {
        sourceFile = "prog1.hl";
    } else if (choice == 2) {
        sourceFile = "prog2.hl";
    } else if (choice == 3) {
        sourceFile = "prog3.hl";
    } else {
        std::cerr << "Invalid choice. Exiting program.\n";
        return 1;
    }

    std::string noSpacesFile {"nospaces.txt"};
    std::string symbolsFile {"res_sym.txt"};

    if (generateNoSpacesFile(sourceFile, noSpacesFile)) {
        std::cout << "Successfully generated " << noSpacesFile << "\n";
    } else {
        return 1;
    }

    if (generateSymbolsFile(noSpacesFile, symbolsFile)) {
        std::cout << "Successfully generated " << symbolsFile << "\n";
    } else {
        return 1;
    }

    return 0;
}
