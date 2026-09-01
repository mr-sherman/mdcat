#pragma once

#include <iostream>
#include <string>
#include <vector>

// A small, deliberately non-exhaustive markdown renderer aimed at making
// plain-text terminal output easier to read -- not at spec-complete CommonMark
// compliance. It understands the markdown constructs people actually use day
// to day: headers, emphasis, inline code, fenced code blocks, lists,
// blockquotes, horizontal rules, links, tables, and emoji shortcodes.
class MarkdownRenderer {
public:
    explicit MarkdownRenderer(std::ostream& out);

    // Reads all of `input` and writes the rendered result to the stream
    // supplied at construction time.
    void render(std::istream& input);

private:
    std::ostream& out_;
    int term_width_;

    void renderLines(const std::vector<std::string>& lines);

    bool isFenceLine(const std::string& line, std::string& lang) const;
    bool isHeaderLine(const std::string& line, int& level, std::string& text) const;
    bool isHorizontalRule(const std::string& line) const;
    bool isBlockquote(const std::string& line, std::string& text) const;
    bool isListItem(const std::string& line, std::string& indent, std::string& marker,
                     std::string& text, bool& ordered) const;
    bool isTableSeparator(const std::string& line) const;

    void renderHeader(int level, const std::string& text);
    void renderHorizontalRule();
    void renderBlockquote(const std::string& text);
    void renderListItem(const std::string& indent, const std::string& marker,
                         const std::string& text, bool ordered, int& ordinal);
    void renderCodeBlock(const std::vector<std::string>& lines, const std::string& lang);
    void renderParagraph(const std::string& text);
    size_t renderTable(const std::vector<std::string>& lines, size_t startIndex);

    // Applies inline formatting (bold, italic, inline code, links, emoji
    // shortcodes) and returns the ANSI-decorated string ready for printing.
    std::string formatInline(const std::string& text) const;

    // Visible-width helper that ignores ANSI escape sequences and treats
    // multi-byte UTF-8 sequences (including emoji) as the columns they
    // actually occupy on screen.
    static size_t displayWidth(const std::string& text);
};
