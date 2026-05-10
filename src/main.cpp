#include <iostream>
#include <string>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage
  int flag = 1;
  while (flag) {
    std::cout << "$ ";
    std::string cmd;
    std::cin >> cmd;
    if (cmd == "exit") {
      flag = 0;
      break;
    } else {
      std::cout << cmd << ": command not found" << std::endl;
    }
  }
}
