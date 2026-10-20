#include "lib_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

CellLib parseLib(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("cannot open lib file: " + path);

    CellLib lib;
    std::string line;
    int ln = 0;
    while (std::getline(f, line)) {
        ++ln;
        auto hash = line.find('#');                 // strip comments
        if (hash != std::string::npos) line.erase(hash);

        std::istringstream ss(line);
        std::string type;
        if (!(ss >> type)) continue;                // blank line

        double delay;
        if (!(ss >> delay) || delay < 0)
            throw std::runtime_error(path + ":" + std::to_string(ln) +
                                     ": expected '<type> <delay_ns>'");
        lib[type] = delay;
    }
    return lib;
}