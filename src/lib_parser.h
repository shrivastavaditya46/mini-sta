#pragma once
#include <string>
#include <unordered_map>

using CellLib = std::unordered_map<std::string, double>;  // gate type -> delay (ns)

CellLib parseLib(const std::string& path);