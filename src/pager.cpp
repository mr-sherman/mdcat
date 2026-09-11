#include "pager.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#include "terminal.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
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

// Whitespace-only tokenizer -- deliberately no quoting support. $PAGER is
// expected to be a program name plus flags (e.g. "less -R", "bat --paging
// always"), not a shell pipeline; we never hand it to a shell (see below).
std::vector<std::string> splitArgs(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

std::string pagerCommand() {
    const char* env = std::getenv("PAGER");
    if (env && *env) return env;
    return "less -R";
}

#ifdef _WIN32

// Quotes a single argument per the MSVC CRT command-line convention (the
// same one CreateProcess's child expects), so spaces/quotes in an argument
// round-trip correctly. Shell metacharacters (; | & `` $()) are NOT given
// any special meaning here -- CreateProcess never invokes a shell, so they
// reach the child process as inert literal text.
std::string quoteWindowsArg(const std::string& arg) {
    if (!arg.empty() && arg.find_first_of(" \t\n\v\"") == std::string::npos) {
        return arg;
    }
    std::string result = "\"";
    for (size_t i = 0; i < arg.size();) {
        size_t backslashes = 0;
        while (i < arg.size() && arg[i] == '\\') {
            ++backslashes;
            ++i;
        }
        if (i == arg.size()) {
            result.append(backslashes * 2, '\\');
            break;
        } else if (arg[i] == '"') {
            result.append(backslashes * 2 + 1, '\\');
            result += '"';
            ++i;
        } else {
            result.append(backslashes, '\\');
            result += arg[i];
            ++i;
        }
    }
    result += '"';
    return result;
}

// Spawns argv[0] directly via CreateProcess (no shell involved) with its
// stdin connected to a pipe, writes `content` into that pipe, and waits for
// it to exit. Returns false (leaving `content` unwritten by us) if the
// program couldn't be found/started at all.
bool runPager(const std::vector<std::string>& argv, const std::string& content) {
    std::string cmdline;
    for (size_t i = 0; i < argv.size(); ++i) {
        if (i) cmdline += ' ';
        cmdline += quoteWindowsArg(argv[i]);
    }

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) return false;
    SetHandleInformation(writePipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = readPipe;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi{};
    std::vector<char> buf(cmdline.begin(), cmdline.end());
    buf.push_back('\0');

    BOOL ok = CreateProcessA(nullptr, buf.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);
    CloseHandle(readPipe);
    if (!ok) {
        CloseHandle(writePipe);
        return false;
    }

    DWORD written = 0;
    WriteFile(writePipe, content.data(), static_cast<DWORD>(content.size()), &written, nullptr);
    CloseHandle(writePipe);

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

#else

// Spawns argv[0] directly via fork+execvp (no shell involved) with its
// stdin connected to a pipe, writes `content` into that pipe, and waits for
// it to exit. Returns false (leaving `content` unwritten by us) if the
// program couldn't be found/started at all -- detected via the classic
// self-pipe trick: an O_CLOEXEC pipe that only ever receives data if
// execvp() fails.
bool runPager(const std::vector<std::string>& argv, const std::string& content) {
    int stdinPipe[2];
    if (pipe(stdinPipe) != 0) return false;

    int errPipe[2];
    if (pipe(errPipe) != 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        return false;
    }
    fcntl(errPipe[1], F_SETFD, FD_CLOEXEC);

    pid_t pid = fork();
    if (pid < 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(errPipe[0]);
        close(errPipe[1]);
        return false;
    }

    if (pid == 0) {
        close(stdinPipe[1]);
        close(errPipe[0]);
        dup2(stdinPipe[0], STDIN_FILENO);
        close(stdinPipe[0]);

        std::vector<char*> cargv;
        cargv.reserve(argv.size() + 1);
        for (auto& a : argv) cargv.push_back(const_cast<char*>(a.c_str()));
        cargv.push_back(nullptr);

        execvp(cargv[0], cargv.data());

        int err = errno;
        ssize_t written = write(errPipe[1], &err, sizeof(err));
        (void)written;
        _exit(127);
    }

    close(stdinPipe[0]);
    close(errPipe[1]);

    int err = 0;
    ssize_t n = read(errPipe[0], &err, sizeof(err));
    close(errPipe[0]);

    if (n > 0) {
        close(stdinPipe[1]);
        int status = 0;
        waitpid(pid, &status, 0);
        return false;
    }

    size_t off = 0;
    while (off < content.size()) {
        ssize_t w = write(stdinPipe[1], content.data() + off, content.size() - off);
        if (w <= 0) break;
        off += static_cast<size_t>(w);
    }
    close(stdinPipe[1]);

    int status = 0;
    waitpid(pid, &status, 0);
    return true;
}

#endif

}  // namespace

void display(const std::string& content) {
    if (!term::stdout_is_tty() || lineCount(content) <= term::height()) {
        std::cout << content;
        return;
    }

    std::vector<std::string> argv = splitArgs(pagerCommand());
    if (argv.empty() || !runPager(argv, content)) {
        std::cout << content;
    }
}

}  // namespace pager
