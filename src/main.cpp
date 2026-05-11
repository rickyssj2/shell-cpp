#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <cstdlib>
#include <filesystem>
#include <unistd.h>
#include <optional>
#include <sys/wait.h>
#include <sys/types.h> 

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

std::optional<std::string> get_executable (std::string& cmd) {
  const char* path_cstr = std::getenv("PATH");
  if (path_cstr == nullptr) {
    std::cerr << "PATH is not defined";
  }
  std::string path = path_cstr;
  std::vector<fs::path> dirs = split_path_list(path);
  for (fs::path dir: dirs) {
    fs::path candidate = dir/cmd;
    if (is_executable(candidate)) {
      return candidate.string();
    }
  }
  return std::nullopt;
}

int execute (const std::string& cmd, const std::string& exec_full_path, const std::vector<std::string>& args) {
  pid_t pid = fork();
  if (pid < 0) {
    std::perror("fork");
    return 1;
  }

  std::vector<char*> argv;
  argv.reserve(args.size() + 2);
  argv.push_back(const_cast<char*>(cmd.c_str()));
  for (const auto& s : args) {
      argv.push_back(const_cast<char*>(s.c_str()));
  }
  argv.push_back(nullptr);

  if (pid == 0) {
    // Child
    execv(exec_full_path.c_str(), argv.data());
  } else {
    // Parent
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        std::perror("waitpid");
        return 1;
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) {
        std::cerr << "Killed by signal " << WTERMSIG(status) << "\n";
        return 128 + WTERMSIG(status);
    }
    return 1;
  }
  return 1;
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

std::vector<std::string> parse (const std::string& str) {
  std::vector<std::string> tokens;
  std::string current;       // token being accumulated
  bool in_token = false;     // true once we've started a token (even via empty quotes)
  bool in_single_quote = false;
  bool in_double_quote = false;
  bool in_backslash = false;
  size_t i = 0;

  while (i < str.size()) {
    char c = str[i];

    if (in_single_quote) {
      if (c == '\'') {
        // closing quote — exit quote mode, stay in token
        in_single_quote = false;
      } else {
        // everything inside single quotes is literal (spaces, $, *, etc.)
        current += c;
      }
      ++i;
    } else if (in_backslash) {
      current += c;
      in_backslash = false;
      ++i;
    } else if (in_double_quote) {
      if (c == '"') {
        in_double_quote = false;
      } else if (c == '\\') {
        in_backslash = true;
      } else {
        current += c;
      }
      ++i;
    } else if (c == '"') {
      in_token = true;
      in_double_quote = true;
      ++i;

    } else if (c == '\'') {
      // opening quote — mark that a token has started (handles '' → empty concat)
      in_token = true;
      in_single_quote = true;
      ++i;
    } else if (c == '\\') {
      in_token = true;
      in_backslash = true;
      ++i;
    } else if (c == ' ' || c == '\t') {
      if (in_token) {
        if (!current.empty()) tokens.push_back(current);
        current.clear();
        in_token = false;
      }
      // skip consecutive whitespace outside quotes
      while (i < str.size() && (str[i] == ' ' || str[i] == '\t')) ++i;

    } else {
      in_token = true;
      current += c;
      ++i;
    }
  }

  // flush the last token
  if (in_token && !current.empty()) tokens.push_back(current);
  return tokens;
}

int main () {
  std::unordered_set<std::string> builtins;
  builtins.insert("echo");
  builtins.insert("exit");
  builtins.insert("type");
  builtins.insert("pwd");
  builtins.insert("cd");

  int flag = 1;
  while (flag) {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    std::cout << "$ ";

    std::string input;
    std::getline(std::cin, input);
    input = strip(input);

    std::vector<std::string> tokens = parse(input);
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

      if (builtins.find(cmd2) != builtins.end()) {
        std::cout << cmd2 << " is a shell builtin" << std::endl;
        continue;
      }
      
      if (auto exec = get_executable(cmd2)) {
        std::cout << cmd2 << " is " << exec.value() << std::endl;
        continue;
      }
      
      std::cout << cmd2 << ": not found" << std::endl;
    
    } else if (cmd == "pwd") {
      fs::path cwd = fs::current_path();
      std::cout << cwd.string() << std::endl;

    } else if (cmd == "cd") {
      std::string new_cwd = tokens[1];
      if (new_cwd == "~") {
        new_cwd = std::getenv("HOME");
      }
      if (chdir(new_cwd.c_str()) != 0) {
        std::cout << "cd: " << new_cwd << ": No such file or directory" << std::endl;
      }

    } else if (auto exec = get_executable(cmd)) {
      std::vector<std::string> args(tokens.begin() + 1, tokens.end());
      execute(cmd, exec.value(), args);

    } else {
      std::cout << cmd << ": command not found" << std::endl;
    }
  }
}
