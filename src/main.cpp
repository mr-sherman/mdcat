#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "markdown_renderer.hpp"
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

    po::options_description visible("Usage: mdcat <file.md>");
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

    if (vm.count("help") || !vm.count("file")) {
        std::cout << visible << "\n";
        return vm.count("help") ? 0 : 1;
    }

    std::filesystem::path path(vm["file"].as<std::string>());

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

    MarkdownRenderer renderer(std::cout);
    renderer.render(file);

    return 0;
}
