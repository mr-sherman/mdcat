#pragma once

#include <string>

// Small collection of ANSI escape helpers and cross-platform terminal
// utilities. Kept deliberately simple: mdcat targets any terminal that
// understands basic ANSI SGR codes (which includes modern Windows Terminal
// and PowerShell/cmd once virtual terminal processing is enabled).
namespace term {

namespace ansi {
constexpr const char* reset = "\033[0m";
constexpr const char* bold = "\033[1m";
constexpr const char* dim = "\033[2m";
constexpr const char* italic = "\033[3m";
constexpr const char* underline = "\033[4m";

constexpr const char* fg_red = "\033[31m";
constexpr const char* fg_green = "\033[32m";
constexpr const char* fg_yellow = "\033[33m";
constexpr const char* fg_blue = "\033[34m";
constexpr const char* fg_magenta = "\033[35m";
constexpr const char* fg_cyan = "\033[36m";
constexpr const char* fg_white = "\033[37m";
constexpr const char* fg_gray = "\033[90m";
constexpr const char* fg_bright_cyan = "\033[96m";
constexpr const char* fg_bright_yellow = "\033[93m";
constexpr const char* fg_bright_green = "\033[92m";
constexpr const char* fg_bright_magenta = "\033[95m";
constexpr const char* fg_bright_blue = "\033[94m";
constexpr const char* fg_bright_red = "\033[91m";
}  // namespace ansi

// Enables UTF-8 output and, on Windows, ANSI/VT100 escape processing on the
// console. Safe to call unconditionally on every platform.
void enable_console_features();

// Best-effort terminal width in columns. Falls back to 80 when it can't be
// determined (e.g. output is redirected to a file).
int width(int fallback = 80);

}  // namespace term
