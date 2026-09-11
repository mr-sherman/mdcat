#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "markdown_renderer.hpp"
#include "pager.hpp"
#include "terminal.hpp"

namespace po = boost::program_options;

namespace {

bool hasMarkdownExtension(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    boost::algorithm::to_lower(ext);
    return ext == ".md" || ext == ".markdown";
}

}  // namespace

int main(int argc, char** argv) {
    term::enable_console_features();

    po::options_description hidden;
    hidden.add_options()("file", po::value<std::string>(), "markdown file to render");

    po::positional_options_description positional;
    positional.add("file", 1);

    po::options_description visible("Usage: mdcat <file.md|->");
    visible.add_options()("help,h", "show this help message");

    po::options_description all;
    all.add(hidden).add(visible);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(all).positional(positional).run(), vm);
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "mdcat: " << e.what() << "\n\n" << visible << "\n";
        return 1;
    }

    if (vm.count("help")) {
        std::cout << visible << "\n";
        return 0;
    }

    // Reads from stdin when given explicitly (`mdcat -`, the conventional
    // stdin marker) or implicitly, when no file was given and stdin isn't an
    // interactive terminal (e.g. `git show HEAD:README.md | mdcat`). A bare
    // `mdcat` with nothing piped in falls through to the usage message below
    // rather than silently blocking on keyboard input.
    bool useStdin = false;
    std::filesystem::path path;

    if (vm.count("file")) {
        std::string arg = vm["file"].as<std::string>();
        if (arg == "-") {
            useStdin = true;
        } else {
            path = arg;
        }
    } else if (!term::stdin_is_tty()) {
        useStdin = true;
    } else {
        std::cout << visible << "\n";
        return 1;
    }

    std::ostringstream buffer;
    MarkdownRenderer renderer(buffer);

    if (useStdin) {
        renderer.render(std::cin);
    } else {
        if (!hasMarkdownExtension(path)) {
            std::cerr << "mdcat: '" << path.string() << "' is not a markdown file (expected .md or .markdown)\n";
            return 1;
        }

        std::error_code ec;
        if (!std::filesystem::exists(path, ec) || !std::filesystem::is_regular_file(path, ec)) {
            std::cerr << "mdcat: '" << path.string() << "' not found\n";
            return 1;
        }

        std::ifstream file(path, std::ios::binary);
        if (!file) {
            std::cerr << "mdcat: could not open '" << path.string() << "'\n";
            return 1;
        }
        renderer.render(file);
    }

    pager::display(buffer.str());

    return 0;
}
