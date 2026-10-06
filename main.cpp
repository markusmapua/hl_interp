#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>

enum class TokenType {
    Keyword, Symbol, Identifier, Number, StringLiteral
};

struct Token {
    TokenType type;
    std::string value;
};

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

bool generateSymbolsFile(const std::string& inputFilename, const std::string& outputFilename, std::vector<Token>& outTokens) {
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

            // 1. Strings
            if (c == '"') {
                std::string strVal {};
                i++;
                while (i < line.length() && line[i] != '"') {
                    strVal += line[i];
                    i++;
                }
                outTokens.push_back({TokenType::StringLiteral, strVal});
                i++; 
                continue;
            }

            if (std::isdigit(c)) {
                std::string numStr {};
                while (i < line.length() && (std::isdigit(line[i]) || line[i] == '.')) {
                    numStr += line[i];
                    i++;
                }
                outTokens.push_back({TokenType::Number, numStr});
                continue;
            }

            if (std::isalpha(c)) {
                std::string word {};
                while (i < line.length() && std::isalpha(line[i])) {
                    word += line[i];
                    i++;
                }

                bool isKeyword {false};
                for (const auto& res : reserved) {
                    if (word == res) {
                        outFile << word << '\n'; // Spec: Save to file
                        outTokens.push_back({TokenType::Keyword, word});
                        isKeyword = true;
                        break;
                    }
                }
                if (!isKeyword) {
                    outTokens.push_back({TokenType::Identifier, word});
                }
                continue;
            }

            std::string sym {};
            bool foundTwoChar {false};
            
            if (i + 1 < line.length()) {
                sym = std::string(1, c) + line[i+1];
                for (const auto& s : symbols) {
                    if (sym == s && sym.length() == 2) {
                        outFile << sym << '\n'; // Spec: Save to file
                        outTokens.push_back({TokenType::Symbol, sym});
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
                    outFile << sym << '\n'; // Spec: Save to file
                    outTokens.push_back({TokenType::Symbol, sym});
                    break;
                }
            }
            i++;
        }
    }
    return true;
}

bool getValue(const Token& t, const std::unordered_map<std::string, double>& memory, double& out) {
    if (t.type == TokenType::Number) {
        out = std::stod(t.value);
        return true;
    } else if (t.type == TokenType::Identifier) {
        auto it = memory.find(t.value);
        if (it != memory.end()) { 
            out = it->second; 
            return true; 
        }
    }
    return false;
}

bool parseExpression(const std::vector<Token>& tokens, size_t& i, const std::unordered_map<std::string, double>& memory, double& result) {
    if (i >= tokens.size() || !getValue(tokens[i], memory, result)) return false;
    i++;
    
    // Check if there is addition or subtraction (e.g., x = 3 + 2)
    if (i < tokens.size() && (tokens[i].value == "+" || tokens[i].value == "-")) {
        std::string op = tokens[i].value;
        i++;
        double rightVal;
        if (i >= tokens.size() || !getValue(tokens[i], memory, rightVal)) return false;
        
        if (op == "+") result += rightVal;
        else result -= rightVal;
        i++;
    }
    return true;
}

bool executeProgram(const std::vector<Token>& tokens) {
    std::unordered_map<std::string, double> memory {};
    size_t i {0};
    bool executeFlag {true};

    while (i < tokens.size()) {
        Token t = tokens[i];

        if (t.type == TokenType::Identifier) {

            if (i + 3 < tokens.size() && tokens[i+1].value == ":" && 
               (tokens[i+2].value == "integer" || tokens[i+2].value == "double") && tokens[i+3].value == ";") {
                if (executeFlag) memory[t.value] = 0.0;
                i += 4;
            } else if (i + 1 < tokens.size() && (tokens[i+1].value == ":=" || tokens[i+1].value == "=")) {
                std::string varName = t.value;
                i += 2;
                
                double result {0.0};
                if (!parseExpression(tokens, i, memory, result)) return false;
                
                if (i < tokens.size() && tokens[i].value == ";") {
                    if (executeFlag) memory[varName] = result;
                    i++;
                } else return false;
            } else return false;
            
            executeFlag = true;
        } 
 
        else if (t.type == TokenType::Keyword && t.value == "output") {
            if (i + 1 < tokens.size() && tokens[i+1].value == "<<") {
                i += 2;
                
                if (i < tokens.size() && tokens[i].type == TokenType::StringLiteral) {
                    if (executeFlag) std::cout << tokens[i].value << '\n';
                    i++;
                } else {
                    double result {0.0};
                    if (!parseExpression(tokens, i, memory, result)) return false;
                    if (executeFlag) std::cout << result << '\n';
                }
                
                if (i < tokens.size() && tokens[i].value == ";") {
                    i++;
                } else return false;
            } else return false;
            
            executeFlag = true;
        } 

        else if (t.type == TokenType::Keyword && t.value == "if") {
            if (i + 4 < tokens.size() && tokens[i+1].value == "(") {
                i += 2;
                double leftVal {}, rightVal {};
                
                if (!getValue(tokens[i], memory, leftVal)) return false;
                i++;
                
                std::string op = tokens[i].value;
                if (op != "<" && op != ">" && op != "==" && op != "!=") return false;
                i++;
                
                if (!getValue(tokens[i], memory, rightVal)) return false;
                i++;
                
                if (tokens[i].value != ")") return false;
                i++;
                
                if (executeFlag) {
                    if (op == "<") executeFlag = (leftVal < rightVal);
                    else if (op == ">") executeFlag = (leftVal > rightVal);
                    else if (op == "==") executeFlag = (leftVal == rightVal);
                    else if (op == "!=") executeFlag = (leftVal != rightVal);
                }
                continue;
            } else return false;
        } 
        
        else {
            return false;
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
    std::vector<Token> tokens {};

    if (generateNoSpacesFile(sourceFile, noSpacesFile)) {
        std::cout << "Successfully generated " << noSpacesFile << "\n";
    } else {
        return 1;
    }

    if (generateSymbolsFile(noSpacesFile, symbolsFile, tokens)) {
        std::cout << "Successfully generated " << symbolsFile << "\n";
    } else {
        return 1;
    }

    if (executeProgram(tokens)) {
        std::cout << "\nNO ERROR(S) FOUND\n";
    } else {
        std::cout << "ERROR\n";
    }

    return 0;
}
