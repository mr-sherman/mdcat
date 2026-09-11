#include "pager.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "terminal.hpp"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace pager {

namespace {

int lineCount(const std::string& s) {
    int n = 0;
    for (char c : s) {
        if (c == '\n') ++n;
    }
    if (!s.empty() && s.back() != '\n') ++n;
    return n;
}

std::string firstToken(const std::string& s) {
    std::istringstream iss(s);
    std::string tok;
    iss >> tok;
    return tok;
}

bool commandExists(const std::string& program) {
    if (program.empty()) return false;
#ifdef _WIN32
    std::string check = "where " + program + " >NUL 2>&1";
#else
    std::string check = "command -v " + program + " >/dev/null 2>&1";
#endif
    return std::system(check.c_str()) == 0;
}

std::string pagerCommand() {
    const char* env = std::getenv("PAGER");
    if (env && *env) return env;
    return "less -R";
}

}  // namespace

void display(const std::string& content) {
    if (!term::stdout_is_tty() || lineCount(content) <= term::height()) {
        std::cout << content;
        return;
    }

    std::string cmd = pagerCommand();
    if (!commandExists(firstToken(cmd))) {
        std::cout << content;
        return;
    }

    FILE* pipe = popen(cmd.c_str(), "w");
    if (!pipe) {
        std::cout << content;
        return;
    }
    std::fwrite(content.data(), 1, content.size(), pipe);
    pclose(pipe);
}

}  // namespace pager
