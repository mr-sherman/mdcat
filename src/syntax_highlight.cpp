#include "syntax_highlight.hpp"

#include <boost/algorithm/string.hpp>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

#include "terminal.hpp"

using term::ansi::bold;
using term::ansi::dim;
using term::ansi::fg_bright_cyan;
using term::ansi::fg_bright_green;
using term::ansi::fg_bright_magenta;
using term::ansi::fg_bright_yellow;
using term::ansi::italic;
using term::ansi::reset;

namespace syntax {

namespace {

struct LangProfile {
    bool recognized = false;
    std::string lineComment;
    std::string blockCommentOpen;
    std::string blockCommentClose;
    bool caseInsensitiveKeywords = false;
    std::unordered_set<std::string> keywords;
};

const std::unordered_set<std::string>& cLikeKeywords() {
    static const std::unordered_set<std::string> kw = {
        "alignas", "alignof", "and", "asm", "auto", "bool", "break", "case", "catch", "char",
        "char16_t", "char32_t", "class", "const", "constexpr", "const_cast", "continue",
        "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
        "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if",
        "inline", "int", "long", "mutable", "namespace", "new", "noexcept", "not", "nullptr",
        "operator", "or", "private", "protected", "public", "register", "reinterpret_cast",
        "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast",
        "struct", "switch", "template", "this", "thread_local", "throw", "true", "try",
        "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void",
        "volatile", "wchar_t", "while",
    };
    return kw;
}

const std::unordered_set<std::string>& javascriptKeywords() {
    static const std::unordered_set<std::string> kw = {
        "async", "await", "break", "case", "catch", "class", "const", "continue", "debugger",
        "default", "delete", "do", "else", "export", "extends", "false", "finally", "for",
        "function", "get", "if", "import", "in", "instanceof", "let", "new", "null", "of",
        "return", "set", "static", "super", "switch", "this", "throw", "true", "try", "typeof",
        "var", "void", "while", "with", "yield",
    };
    return kw;
}

const std::unordered_map<std::string, LangProfile>& profiles() {
    static const std::unordered_map<std::string, LangProfile> table = [] {
        std::unordered_map<std::string, LangProfile> t;

        LangProfile cpp;
        cpp.recognized = true;
        cpp.lineComment = "//";
        cpp.blockCommentOpen = "/*";
        cpp.blockCommentClose = "*/";
        cpp.keywords = cLikeKeywords();
        t["cpp"] = cpp;

        LangProfile python;
        python.recognized = true;
        python.lineComment = "#";
        python.keywords = {
            "False", "None",  "True",   "and",    "as",     "assert", "async",  "await",
            "break", "class", "continue", "def",  "del",    "elif",   "else",   "except",
            "finally", "for", "from",   "global", "if",     "import", "in",     "is",
            "lambda", "nonlocal", "not", "or",     "pass",   "raise",  "return", "try",
            "while", "with", "yield",
        };
        t["python"] = python;

        LangProfile javascript;
        javascript.recognized = true;
        javascript.lineComment = "//";
        javascript.blockCommentOpen = "/*";
        javascript.blockCommentClose = "*/";
        javascript.keywords = javascriptKeywords();
        t["javascript"] = javascript;

        LangProfile typescript = javascript;
        typescript.keywords.insert({"interface", "type", "implements", "enum", "namespace",
                                     "declare", "readonly", "abstract", "as", "is", "keyof",
                                     "infer", "never", "unknown", "any", "satisfies", "public",
                                     "private", "protected"});
        t["typescript"] = typescript;

        LangProfile bash;
        bash.recognized = true;
        bash.lineComment = "#";
        bash.keywords = {
            "if",     "then",    "else",   "elif",   "fi",     "for",    "while",  "until",
            "do",     "done",    "case",   "esac",   "function", "in",   "select", "time",
            "coproc", "return",  "exit",   "break",  "continue", "local", "export", "readonly",
            "declare", "typeset", "unset", "shift",  "eval",   "exec",   "source", "alias",
            "trap",
        };
        t["bash"] = bash;

        LangProfile go;
        go.recognized = true;
        go.lineComment = "//";
        go.blockCommentOpen = "/*";
        go.blockCommentClose = "*/";
        go.keywords = {
            "break", "case", "chan",  "const",  "continue", "default", "defer", "else",
            "fallthrough", "for", "func", "go",  "goto",   "if",       "import", "interface",
            "map",   "package", "range", "return", "select", "struct",   "switch", "type",
            "var",
        };
        t["go"] = go;

        LangProfile rust;
        rust.recognized = true;
        rust.lineComment = "//";
        rust.blockCommentOpen = "/*";
        rust.blockCommentClose = "*/";
        rust.keywords = {
            "as",    "async", "await", "break",  "const",  "continue", "crate", "dyn",
            "else",  "enum",  "extern", "false", "fn",     "for",      "if",    "impl",
            "in",    "let",   "loop",  "match",  "mod",    "move",     "mut",   "pub",
            "ref",   "return", "self", "Self",   "static", "struct",   "super", "trait",
            "true",  "type",  "union", "unsafe", "use",    "where",    "while",
        };
        t["rust"] = rust;

        LangProfile java;
        java.recognized = true;
        java.lineComment = "//";
        java.blockCommentOpen = "/*";
        java.blockCommentClose = "*/";
        java.keywords = {
            "abstract", "assert",   "boolean", "break",     "byte",      "case",   "catch",
            "char",     "class",    "const",   "continue",  "default",   "do",     "double",
            "else",     "enum",     "extends", "final",     "finally",   "float",  "for",
            "goto",     "if",       "implements", "import", "instanceof", "int",   "interface",
            "long",     "native",   "new",     "package",   "private",   "protected", "public",
            "return",   "short",    "static",  "strictfp",  "super",     "switch", "synchronized",
            "this",     "throw",    "throws",  "transient", "true",      "false",  "null",
            "try",      "void",     "volatile", "while",
        };
        t["java"] = java;

        LangProfile ruby;
        ruby.recognized = true;
        ruby.lineComment = "#";
        ruby.keywords = {
            "BEGIN",   "END",    "alias",  "and",    "begin",  "break",  "case",   "class",
            "def",     "defined?", "do",   "else",   "elsif",  "end",    "ensure", "false",
            "for",     "if",     "in",     "module", "next",   "nil",    "not",    "or",
            "redo",    "rescue", "retry",  "return",  "self",  "super",  "then",   "true",
            "undef",   "unless", "until",  "when",   "while",  "yield",
        };
        t["ruby"] = ruby;

        LangProfile json;
        json.recognized = true;
        json.keywords = {"true", "false", "null"};
        t["json"] = json;

        LangProfile yaml;
        yaml.recognized = true;
        yaml.lineComment = "#";
        yaml.keywords = {"true", "false", "null", "yes", "no", "on", "off"};
        t["yaml"] = yaml;

        LangProfile sql;
        sql.recognized = true;
        sql.lineComment = "--";
        sql.blockCommentOpen = "/*";
        sql.blockCommentClose = "*/";
        sql.caseInsensitiveKeywords = true;
        sql.keywords = {
            "SELECT", "FROM",    "WHERE",   "INSERT",   "INTO",    "VALUES",  "UPDATE", "SET",
            "DELETE", "CREATE",  "TABLE",   "ALTER",    "DROP",    "JOIN",    "INNER",  "LEFT",
            "RIGHT",  "OUTER",   "ON",      "GROUP",    "BY",      "ORDER",   "HAVING", "LIMIT",
            "AS",     "AND",     "OR",      "NOT",      "NULL",    "IS",      "IN",     "LIKE",
            "DISTINCT", "UNION", "ALL",     "EXISTS",   "CASE",    "WHEN",    "THEN",   "ELSE",
            "END",    "PRIMARY", "KEY",     "FOREIGN",  "REFERENCES", "DEFAULT", "UNIQUE", "INDEX",
            "VIEW",   "TRIGGER", "PROCEDURE", "FUNCTION", "RETURNS", "BEGIN",  "COMMIT", "ROLLBACK",
            "TRANSACTION",
        };
        t["sql"] = sql;

        return t;
    }();
    return table;
}

std::string normalizeLang(const std::string& lang) {
    static const std::unordered_map<std::string, std::string> aliases = {
        {"c", "cpp"},        {"c++", "cpp"},      {"cxx", "cpp"},     {"cc", "cpp"},
        {"h", "cpp"},        {"hpp", "cpp"},      {"js", "javascript"}, {"jsx", "javascript"},
        {"mjs", "javascript"}, {"cjs", "javascript"}, {"ts", "typescript"}, {"tsx", "typescript"},
        {"py", "python"},    {"py3", "python"},   {"sh", "bash"},     {"shell", "bash"},
        {"zsh", "bash"},     {"golang", "go"},    {"rs", "rust"},     {"rb", "ruby"},
        {"yml", "yaml"},
    };

    std::string lower = boost::algorithm::to_lower_copy(boost::algorithm::trim_copy(lang));
    auto it = aliases.find(lower);
    return it != aliases.end() ? it->second : lower;
}

const LangProfile& profileFor(const std::string& lang) {
    static const LangProfile unrecognized;
    std::string canonical = normalizeLang(lang);
    if (canonical.empty()) return unrecognized;
    auto it = profiles().find(canonical);
    return it != profiles().end() ? it->second : unrecognized;
}

bool isIdentStart(unsigned char c) { return std::isalpha(c) != 0 || c == '_'; }
bool isIdentChar(unsigned char c) { return std::isalnum(c) != 0 || c == '_'; }

}  // namespace

std::string highlight(const std::string& line, const std::string& lang) {
    const LangProfile& profile = profileFor(lang);
    if (!profile.recognized) {
        return std::string(fg_bright_yellow) + line + reset;
    }

    std::string result;
    result.reserve(line.size() * 2);
    size_t n = line.size();
    size_t i = 0;

    while (i < n) {
        if (!profile.lineComment.empty() &&
            line.compare(i, profile.lineComment.size(), profile.lineComment) == 0) {
            result += std::string(dim) + italic + line.substr(i) + reset;
            break;
        }

        if (!profile.blockCommentOpen.empty() &&
            line.compare(i, profile.blockCommentOpen.size(), profile.blockCommentOpen) == 0) {
            size_t closePos = line.find(profile.blockCommentClose, i + profile.blockCommentOpen.size());
            size_t end = (closePos == std::string::npos) ? n : closePos + profile.blockCommentClose.size();
            result += std::string(dim) + italic + line.substr(i, end - i) + reset;
            i = end;
            continue;
        }

        char c = line[i];

        if (c == '"' || c == '\'' || c == '`') {
            size_t j = i + 1;
            while (j < n && line[j] != c) {
                if (line[j] == '\\' && j + 1 < n) ++j;
                ++j;
            }
            if (j < n) ++j;  // include the closing quote, if any
            result += std::string(fg_bright_green) + line.substr(i, j - i) + reset;
            i = j;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            size_t j = i;
            while (j < n && (std::isalnum(static_cast<unsigned char>(line[j])) || line[j] == '.' ||
                              line[j] == '_')) {
                ++j;
            }
            result += std::string(fg_bright_cyan) + line.substr(i, j - i) + reset;
            i = j;
            continue;
        }

        if (isIdentStart(static_cast<unsigned char>(c))) {
            size_t j = i;
            while (j < n && isIdentChar(static_cast<unsigned char>(line[j]))) ++j;
            std::string word = line.substr(i, j - i);
            std::string key = profile.caseInsensitiveKeywords ? boost::algorithm::to_upper_copy(word) : word;
            if (profile.keywords.count(key)) {
                result += std::string(bold) + fg_bright_magenta + word + reset;
            } else {
                result += word;
            }
            i = j;
            continue;
        }

        result += c;
        ++i;
    }

    return result;
}

}  // namespace syntax
