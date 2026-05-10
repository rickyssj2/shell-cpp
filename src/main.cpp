#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>

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
  std::unordered_set<std::string> commands;
  commands.insert("echo");
  commands.insert("exit");
  commands.insert("type");
  
  int flag = 1;
  while (flag) {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);
    input = strip(input);
    std::vector<std::string> tokens = split(input);
    std::string cmd = tokens[0];

    if (cmd == "exit") {
      flag = 0;
    } else if (cmd == "echo") {
      for (int i = 1; i < tokens.size(); i++) {
        std::cout << tokens[i] << " ";
      }
      std::cout << std::endl;
    } else if (cmd == "type") {
      std::string cmd2 = tokens[1];
      if (commands.find(cmd2) != commands.end())
        std::cout << cmd2 << " is a shell builtin" << std::endl;
      else
        std::cout << cmd2 << ": not found" << std::endl;
    } else {
      std::cout << cmd << ": command not found" << std::endl;
    }
  }
}
