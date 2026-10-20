#pragma once
#include <string>
#include <vector>
#include "lib_parser.h"

struct Gate {
    std::string name, type;
    std::vector<std::string> inputs;
    std::string output;
};

struct Netlist {
    std::vector<std::string> inputs, outputs;
    std::vector<Gate> gates;
};

Netlist parseNetlist(const std::string& path, const CellLib& lib);