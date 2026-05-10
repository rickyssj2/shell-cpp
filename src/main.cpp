#include <iostream>
#include <string>
#include <vector>
#include <sstream>

std::string strip (const std::string& str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, last - first + 1);
}

std::vector<std::string> split (const std::string& str, char delim = ' ') {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delim)) 
    tokens.emplace_back(token);
  return tokens;
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage
  int flag = 1;
  while (flag) {
    std::cout << "$ ";
    std::string input;
    std::cin >> input;
    input = strip(input);
    std::vector<std::string> tokens = split(input);
    std::string cmd = tokens[0];

    if (cmd == "exit") {
      flag = 0;
    } else if (cmd == "echo") {
      for (int i = 0; i < tokens.size(); i++) {
        std::cout << tokens[i] << " ";
      }
      std::cout << std::endl;
    } else {
      std::cout << cmd << ": command not found" << std::endl;
    }
  }
}
