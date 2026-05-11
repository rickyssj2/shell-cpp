#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <cstdlib>
#include <filesystem>
#include <unistd.h>

#ifdef _WIN32
constexpr char PATH_LIST_SEP = ';';
#else
constexpr char PATH_LIST_SEP = ':';
#endif

namespace fs = std::filesystem;

std::vector<fs::path> split_path_list (const std::string& path_list) {
  std::vector<fs::path> paths;
  std::stringstream ss(path_list);
  std::string path;
  while (std::getline(ss, path, PATH_LIST_SEP))
    paths.emplace_back(path);
  return paths;
}

#ifdef _WIN32
bool has_executable_extension (const fs::path& p) {
  static const std::vector<std::string> exts = {".exe, .bat, .cmd, .com"};
  auto ext = p.extension().string();

  for (auto& e: exts) {
    if (_stricmp(e.c_str(), ext.c_str()) == 0) return true;
  }
  return false;
}
#endif

bool is_executable_posix (const fs::path& p){
  return access(p.c_str(), X_OK) == 0;
}

bool is_executable (const fs::path& p) {
  #ifdef _WIN32
    return fs::exists(p) && fs::is_regular_file(p) && has_executable_extension(p);
  #else
    return is_executable_posix(p);
  #endif
}

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

int main () {
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

      if (commands.find(cmd2) != commands.end()) {
        std::cout << cmd2 << " is a shell builtin" << std::endl;
        continue;
      }
      
      const char* path_cstr = std::getenv("PATH");
      if (path_cstr == nullptr) {
        std::cerr << "PATH is not defined";
      }
      std::string path = path_cstr;
      std::vector<fs::path> dirs = split_path_list(path);
      bool found = false;
      for (fs::path dir: dirs) {
        fs::path candidate = dir/cmd2;
        if (is_executable(candidate)) {
          std::cout << cmd2 << " is " << candidate << std::endl;
          found = true;
          break;
        }
      }
      
      if (!found) {
        std::cout << cmd2 << ": not found" << std::endl;
      }
      
    } else {
      std::cout << cmd << ": command not found" << std::endl;
    }
  }
}
