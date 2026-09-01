#include "markdown_renderer.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <sstream>

#include "emoji_map.hpp"
#include "terminal.hpp"

using term::ansi::bold;
using term::ansi::dim;
using term::ansi::fg_bright_blue;
using term::ansi::fg_bright_cyan;
using term::ansi::fg_bright_green;
using term::ansi::fg_bright_magenta;
using term::ansi::fg_bright_yellow;
using term::ansi::fg_gray;
using term::ansi::italic;
using term::ansi::reset;
using term::ansi::underline;

namespace {

// Applies `transform` to every regex match in `input`, copying everything
// else through unchanged. Used instead of boost::regex_replace so emoji /
// code-span lookups can run arbitrary logic rather than a format string.
std::string replaceMatches(const std::string& input, const boost::regex& re,
                            const std::function<std::string(const boost::smatch&)>& transform) {
    std::string result;
    result.reserve(input.size());
    auto begin = boost::sregex_iterator(input.begin(), input.end(), re);
    auto end = boost::sregex_iterator();
    size_t lastPos = 0;
    for (auto it = begin; it != end; ++it) {
        const boost::smatch& m = *it;
        size_t pos = static_cast<size_t>(m.position(static_cast<size_t>(0)));
        result.append(input, lastPos, pos - lastPos);
        result += transform(m);
        lastPos = pos + static_cast<size_t>(m.length(static_cast<size_t>(0)));
    }
    result.append(input, lastPos, input.size() - lastPos);
    return result;
}

const char* headerColor(int level) {
    switch (level) {
        case 1: return fg_bright_cyan;
        case 2: return fg_bright_yellow;
        case 3: return fg_bright_green;
        case 4: return fg_bright_magenta;
        case 5: return fg_bright_blue;
        default: return fg_gray;
    }
}

// 1 for a normal-width codepoint, 2 for wide (CJK/emoji), 0 for zero-width
// combining marks and emoji modifiers.
int codepointWidth(uint32_t cp) {
    if (cp == 0 || cp == 0xFE0F || cp == 0x200D) return 0;
    if (cp >= 0x0300 && cp <= 0x036F) return 0;
    if ((cp >= 0x1100 && cp <= 0x115F) || (cp >= 0x2E80 && cp <= 0xA4CF) ||
        (cp >= 0xAC00 && cp <= 0xD7A3) || (cp >= 0xF900 && cp <= 0xFAFF) ||
        (cp >= 0xFF00 && cp <= 0xFF60) || (cp >= 0xFFE0 && cp <= 0xFFE6) ||
        (cp >= 0x1F300 && cp <= 0x1FAFF) || (cp >= 0x1F1E6 && cp <= 0x1F1FF) ||
        (cp >= 0x2600 && cp <= 0x27BF) || (cp >= 0x2B00 && cp <= 0x2BFF) ||
        (cp >= 0x20000 && cp <= 0x3FFFD)) {
        return 2;
    }
    return 1;
}

// Splits a markdown table row on unescaped '|' after trimming an optional
// leading/trailing pipe.
std::vector<std::string> splitTableRow(const std::string& line) {
    std::string trimmed = boost::algorithm::trim_copy(line);
    if (!trimmed.empty() && trimmed.front() == '|') trimmed.erase(0, 1);
    if (!trimmed.empty() && trimmed.back() == '|') trimmed.pop_back();

    std::vector<std::string> cells;
    std::string current;
    for (size_t i = 0; i < trimmed.size(); ++i) {
        if (trimmed[i] == '\\' && i + 1 < trimmed.size() && trimmed[i + 1] == '|') {
            current += '|';
            ++i;
        } else if (trimmed[i] == '|') {
            cells.push_back(boost::algorithm::trim_copy(current));
            current.clear();
        } else {
            current += trimmed[i];
        }
    }
    cells.push_back(boost::algorithm::trim_copy(current));
    return cells;
}

enum class Align { Left, Center, Right };

Align parseAlign(const std::string& sepCell) {
    std::string s = boost::algorithm::trim_copy(sepCell);
    bool left = !s.empty() && s.front() == ':';
    bool right = !s.empty() && s.back() == ':';
    if (left && right) return Align::Center;
    if (right) return Align::Right;
    return Align::Left;
}

}  // namespace

MarkdownRenderer::MarkdownRenderer(std::ostream& out) : out_(out), term_width_(term::width()) {}

size_t MarkdownRenderer::displayWidth(const std::string& text) {
    size_t width = 0;
    for (size_t i = 0; i < text.size();) {
        unsigned char c = static_cast<unsigned char>(text[i]);

        // Skip ANSI CSI escape sequences: ESC '[' ... final-byte in 0x40-0x7E.
        if (c == 0x1B && i + 1 < text.size() && text[i + 1] == '[') {
            size_t j = i + 2;
            while (j < text.size()) {
                unsigned char t = static_cast<unsigned char>(text[j]);
                ++j;
                if (t >= 0x40 && t <= 0x7E) break;
            }
            i = j;
            continue;
        }

        uint32_t cp = 0;
        size_t len = 1;
        if ((c & 0x80) == 0x00) {
            cp = c;
            len = 1;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < text.size()) {
            cp = (c & 0x1Fu) << 6 | (static_cast<unsigned char>(text[i + 1]) & 0x3Fu);
            len = 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < text.size()) {
            cp = (c & 0x0Fu) << 12 | (static_cast<unsigned char>(text[i + 1]) & 0x3Fu) << 6 |
                 (static_cast<unsigned char>(text[i + 2]) & 0x3Fu);
            len = 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < text.size()) {
            cp = (c & 0x07u) << 18 | (static_cast<unsigned char>(text[i + 1]) & 0x3Fu) << 12 |
                 (static_cast<unsigned char>(text[i + 2]) & 0x3Fu) << 6 |
                 (static_cast<unsigned char>(text[i + 3]) & 0x3Fu);
            len = 4;
        } else {
            cp = c;
            len = 1;
        }

        width += static_cast<size_t>(codepointWidth(cp));
        i += len;
    }
    return width;
}

std::string MarkdownRenderer::formatInline(const std::string& text) const {
    static const boost::regex codeSpan(R"(`([^`]+)`)");
    static const boost::regex bold_re(R"(\*\*([^*]+)\*\*|__([^_]+)__)");
    static const boost::regex italic_re(R"(\*([^*\s][^*]*?)\*|_([^_\s][^_]*?)_)");
    static const boost::regex link_re(R"(\[([^\]]*)\]\(([^)]*)\))");
    static const boost::regex emoji_re(R"(:([a-zA-Z0-9_+\-]+):)");

    // 1. Protect inline code spans behind placeholders so bold/italic/emoji
    // parsing never reaches into code content.
    std::vector<std::string> codeSpans;
    std::string work = replaceMatches(text, codeSpan, [&](const boost::smatch& m) {
        codeSpans.push_back(std::string(term::ansi::fg_bright_yellow) + m[1].str() + reset);
        return "\x01" + std::to_string(codeSpans.size() - 1) + "\x02";
    });

    // Emoji shortcodes are resolved before bold/italic: shortcode names such
    // as `white_check_mark` contain underscores that italic parsing would
    // otherwise misinterpret as emphasis markers.
    work = replaceMatches(work, emoji_re, [&](const boost::smatch& m) {
        const auto& map = emoji::shortcode_map();
        auto it = map.find(m[1].str());
        return it != map.end() ? it->second : m[0].str();
    });

    work = replaceMatches(work, bold_re, [&](const boost::smatch& m) {
        std::string content = m[1].matched ? m[1].str() : m[2].str();
        return std::string(bold) + content + reset;
    });

    work = replaceMatches(work, italic_re, [&](const boost::smatch& m) {
        std::string content = m[1].matched ? m[1].str() : m[2].str();
        return std::string(italic) + content + reset;
    });

    work = replaceMatches(work, link_re, [&](const boost::smatch& m) {
        std::string label = m[1].str();
        std::string url = m[2].str();
        if (label.empty()) label = url;
        return std::string(underline) + label + reset + " (" + dim + url + reset + ")";
    });

    // Restore protected code spans.
    for (size_t idx = 0; idx < codeSpans.size(); ++idx) {
        std::string placeholder = "\x01" + std::to_string(idx) + "\x02";
        size_t pos = work.find(placeholder);
        if (pos != std::string::npos) work.replace(pos, placeholder.size(), codeSpans[idx]);
    }

    return work;
}

bool MarkdownRenderer::isFenceLine(const std::string& line, std::string& lang) const {
    static const boost::regex re(R"(^\s{0,3}```\s*(\S*)\s*$)");
    boost::smatch m;
    if (boost::regex_match(line, m, re)) {
        lang = m[1].str();
        return true;
    }
    return false;
}

bool MarkdownRenderer::isHeaderLine(const std::string& line, int& level, std::string& text) const {
    static const boost::regex re(R"(^\s{0,3}(#{1,6})[ \t]+(.*)$)");
    boost::smatch m;
    if (!boost::regex_match(line, m, re)) return false;
    level = static_cast<int>(m[1].length());
    text = m[2].str();
    boost::algorithm::trim_right_if(text, [](char c) { return c == '#' || c == ' ' || c == '\t'; });
    return true;
}

bool MarkdownRenderer::isHorizontalRule(const std::string& line) const {
    static const boost::regex re(R"(^\s{0,3}((-[ \t]*){3,}|(\*[ \t]*){3,}|(_[ \t]*){3,})$)");
    return boost::regex_match(line, re);
}

bool MarkdownRenderer::isBlockquote(const std::string& line, std::string& text) const {
    static const boost::regex re(R"(^\s{0,3}>[ \t]?(.*)$)");
    boost::smatch m;
    if (!boost::regex_match(line, m, re)) return false;
    text = m[1].str();
    return true;
}

bool MarkdownRenderer::isListItem(const std::string& line, std::string& indent, std::string& marker,
                                   std::string& text, bool& ordered) const {
    static const boost::regex unordered(R"(^(\s*)([-*+])\s+(.*)$)");
    static const boost::regex ordered_re(R"(^(\s*)(\d+[.)])\s+(.*)$)");
    boost::smatch m;
    if (boost::regex_match(line, m, ordered_re)) {
        indent = m[1].str();
        marker = m[2].str();
        text = m[3].str();
        ordered = true;
        return true;
    }
    if (boost::regex_match(line, m, unordered)) {
        indent = m[1].str();
        marker = m[2].str();
        text = m[3].str();
        ordered = false;
        return true;
    }
    return false;
}

bool MarkdownRenderer::isTableSeparator(const std::string& line) const {
    static const boost::regex re(R"(^\s*\|?\s*:?-{1,}:?\s*(\|\s*:?-{1,}:?\s*)+\|?\s*$)");
    return boost::regex_match(line, re);
}

void MarkdownRenderer::renderHeader(int level, const std::string& text) {
    std::string colored = formatInline(text);
    const char* color = headerColor(level);
    std::string hashes(static_cast<size_t>(level), '#');

    out_ << bold << color << hashes << " " << colored << reset << "\n";

    if (level <= 2) {
        size_t width = static_cast<size_t>(level) + 1 + displayWidth(colored);
        width = std::min(width, static_cast<size_t>(std::max(term_width_, 1)));
        char rule = (level == 1) ? '=' : '-';
        out_ << color << std::string(width, rule) << reset << "\n";
    }
    out_ << "\n";
}

void MarkdownRenderer::renderHorizontalRule() {
    int w = std::max(term_width_, 1);
    out_ << fg_gray << std::string(static_cast<size_t>(w), '-') << reset << "\n\n";
}

void MarkdownRenderer::renderBlockquote(const std::string& text) {
    out_ << fg_gray << "\xe2\x94\x82 " << italic << formatInline(text) << reset << "\n";
}

void MarkdownRenderer::renderListItem(const std::string& indent, const std::string& marker,
                                       const std::string& text, bool ordered, int& /*ordinal*/) {
    std::string bullet = ordered ? (marker + " ") : "\xe2\x80\xa2 ";  // "• "
    out_ << indent << fg_bright_cyan << bullet << reset << formatInline(text) << "\n";
}

void MarkdownRenderer::renderCodeBlock(const std::vector<std::string>& lines, const std::string& lang) {
    (void)lang;
    auto repeat = [](const char* utf8Seq, int count) {
        std::string s;
        for (int i = 0; i < count; ++i) s += utf8Seq;
        return s;
    };

    out_ << dim << "\xe2\x94\x8c\xe2\x94\x80 code " << repeat("-", std::max(0, term_width_ - 8)) << reset
         << "\n";
    for (const auto& l : lines) {
        out_ << dim << "\xe2\x94\x82 " << reset << term::ansi::fg_bright_yellow << l << reset << "\n";
    }
    out_ << dim << "\xe2\x94\x94" << repeat("\xe2\x94\x80", std::max(0, term_width_ - 1)) << reset << "\n\n";
}

void MarkdownRenderer::renderParagraph(const std::string& text) { out_ << formatInline(text) << "\n"; }

size_t MarkdownRenderer::renderTable(const std::vector<std::string>& lines, size_t startIndex) {
    std::vector<std::string> headerCells = splitTableRow(lines[startIndex]);
    std::vector<std::string> sepCells = splitTableRow(lines[startIndex + 1]);

    size_t colCount = headerCells.size();
    std::vector<Align> aligns(colCount, Align::Left);
    for (size_t c = 0; c < colCount && c < sepCells.size(); ++c) aligns[c] = parseAlign(sepCells[c]);

    std::vector<std::vector<std::string>> rows;
    size_t last = startIndex + 1;
    for (size_t i = startIndex + 2; i < lines.size(); ++i) {
        std::string trimmed = boost::algorithm::trim_copy(lines[i]);
        if (trimmed.empty() || trimmed.find('|') == std::string::npos) break;
        rows.push_back(splitTableRow(lines[i]));
        last = i;
    }

    // Render inline formatting up front so width math sees the final glyphs.
    std::vector<std::string> renderedHeader;
    for (auto& c : headerCells) renderedHeader.push_back(formatInline(c));
    std::vector<std::vector<std::string>> renderedRows;
    for (auto& row : rows) {
        std::vector<std::string> renderedRow;
        for (size_t c = 0; c < colCount; ++c) renderedRow.push_back(formatInline(c < row.size() ? row[c] : ""));
        renderedRows.push_back(std::move(renderedRow));
    }

    std::vector<size_t> widths(colCount, 3);
    for (size_t c = 0; c < colCount; ++c) {
        widths[c] = std::max(widths[c], displayWidth(renderedHeader[c]));
        for (auto& row : renderedRows) widths[c] = std::max(widths[c], displayWidth(row[c]));
    }

    auto pad = [](const std::string& s, size_t width, Align align) {
        size_t w = displayWidth(s);
        size_t total = width > w ? width - w : 0;
        if (align == Align::Right) return std::string(total, ' ') + s;
        if (align == Align::Center) {
            size_t leftPad = total / 2;
            size_t rightPad = total - leftPad;
            return std::string(leftPad, ' ') + s + std::string(rightPad, ' ');
        }
        return s + std::string(total, ' ');
    };

    auto border = [&](const char* left, const char* mid, const char* right, const char* fill) {
        std::string line = left;
        for (size_t c = 0; c < colCount; ++c) {
            for (size_t k = 0; k < widths[c] + 2; ++k) line += fill;
            line += (c + 1 < colCount) ? mid : right;
        }
        out_ << fg_gray << line << reset << "\n";
    };

    border("\xe2\x94\x8c", "\xe2\x94\xac", "\xe2\x94\x90", "\xe2\x94\x80");

    out_ << fg_gray << "\xe2\x94\x82" << reset;
    for (size_t c = 0; c < colCount; ++c) {
        out_ << " " << bold << pad(renderedHeader[c], widths[c], aligns[c]) << reset << " " << fg_gray
             << "\xe2\x94\x82" << reset;
    }
    out_ << "\n";

    border("\xe2\x94\x9c", "\xe2\x94\xbc", "\xe2\x94\xa4", "\xe2\x94\x80");

    for (auto& row : renderedRows) {
        out_ << fg_gray << "\xe2\x94\x82" << reset;
        for (size_t c = 0; c < colCount; ++c) {
            out_ << " " << pad(row[c], widths[c], aligns[c]) << " " << fg_gray << "\xe2\x94\x82" << reset;
        }
        out_ << "\n";
    }

    border("\xe2\x94\x94", "\xe2\x94\xb4", "\xe2\x94\x98", "\xe2\x94\x80");
    out_ << "\n";

    return last;
}

void MarkdownRenderer::renderLines(const std::vector<std::string>& lines) {
    int ordinal = 0;
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        std::string lang;
        int level;
        std::string text, indent, marker;
        bool ordered;

        if (isFenceLine(line, lang)) {
            std::vector<std::string> block;
            size_t j = i + 1;
            std::string closeLang;
            while (j < lines.size() && !isFenceLine(lines[j], closeLang)) {
                block.push_back(lines[j]);
                ++j;
            }
            renderCodeBlock(block, lang);
            i = j;
            continue;
        }

        if (boost::algorithm::trim_copy(line).empty()) {
            out_ << "\n";
            continue;
        }

        if (isHeaderLine(line, level, text)) {
            renderHeader(level, text);
            continue;
        }

        if (isHorizontalRule(line)) {
            renderHorizontalRule();
            continue;
        }

        if (line.find('|') != std::string::npos && i + 1 < lines.size() && isTableSeparator(lines[i + 1])) {
            i = renderTable(lines, i);
            continue;
        }

        if (isBlockquote(line, text)) {
            renderBlockquote(text);
            continue;
        }

        if (isListItem(line, indent, marker, text, ordered)) {
            renderListItem(indent, marker, text, ordered, ordinal);
            continue;
        }

        renderParagraph(line);
    }
}

void MarkdownRenderer::render(std::istream& input) {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    renderLines(lines);
}
