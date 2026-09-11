#pragma once

#include <string>

// Best-effort pager integration: routes long rendered output through the
// user's pager instead of dumping it straight into a scrolling terminal.
namespace pager {

// Writes `content` to stdout. When stdout is an interactive terminal and
// `content` has more lines than fit on the screen, pipes it through $PAGER
// (or `less -R` if unset) instead of printing directly, so ANSI color codes
// survive. Falls back to a direct write when stdout isn't a terminal, the
// content fits on one screen, or no pager program can be found.
void display(const std::string& content);

}  // namespace pager
