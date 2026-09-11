#pragma once

#include <string>

// Best-effort, line-at-a-time syntax highlighting for fenced code blocks.
// Not a real lexer/parser -- just enough token recognition (keywords,
// strings, comments, numbers) for common languages to read better in a
// terminal, matching mdcat's "simple over spec-complete" philosophy.
namespace syntax {

// Applies ANSI highlighting to a single source line for the fenced code
// block language `lang` (the string after the opening ``` fence, e.g.
// "cpp", "python", "js"). `lang` is matched case-insensitively against a
// small set of common languages and aliases; unrecognized or empty tags
// fall back to the same flat, dimmed-yellow rendering mdcat used before
// syntax highlighting existed.
std::string highlight(const std::string& line, const std::string& lang);

}  // namespace syntax
