#include "netlist_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

static std::runtime_error err(const std::string& p, int ln, const std::string& msg) {
    return std::runtime_error(p + ":" + std::to_string(ln) + ": " + msg);
}

Netlist parseNetlist(const std::string& path, const CellLib& lib) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("cannot open netlist: " + path);

    Netlist nl;
    std::unordered_map<std::string, std::string> driver;  // net -> who drives it
    std::string line;
    int ln = 0;

    while (std::getline(f, line)) {
        ++ln;
        auto hash = line.find('#');
        if (hash != std::string::npos) line.erase(hash);

        std::istringstream ss(line);
        std::vector<std::string> tok;
        for (std::string t; ss >> t;) tok.push_back(t);
        if (tok.empty()) continue;

        if (tok[0] == "INPUT" || tok[0] == "OUTPUT") {
            if (tok.size() < 2) throw err(path, ln, tok[0] + " needs at least one net");
            for (size_t i = 1; i < tok.size(); ++i) {
                if (tok[0] == "INPUT") {
                    if (driver.count(tok[i]))
                        throw err(path, ln, "net '" + tok[i] + "' has multiple drivers");
                    driver[tok[i]] = "primary input";
                    nl.inputs.push_back(tok[i]);
                } else {
                    nl.outputs.push_back(tok[i]);
                }
            }
        } else if (tok[0] == "GATE") {
            // GATE name type in1 [in2...] out  -> at least 5 tokens
            if (tok.size() < 5) throw err(path, ln, "malformed GATE line");
            Gate g;
            g.name = tok[1];
            g.type = tok[2];
            if (!lib.count(g.type)) throw err(path, ln, "undefined gate type '" + g.type + "'");
            g.inputs.assign(tok.begin() + 3, tok.end() - 1);  // everything between type and last
            g.output = tok.back();                            // last token = output
            if (driver.count(g.output))
                throw err(path, ln, "net '" + g.output + "' has multiple drivers");
            driver[g.output] = g.name;
            nl.gates.push_back(g);
        } else {
            throw err(path, ln, "unknown keyword '" + tok[0] + "'");
        }
    }

    // Checked at the end so declaration order doesn't matter
    for (const Gate& g : nl.gates)
        for (const auto& n : g.inputs)
            if (!driver.count(n))
                throw std::runtime_error("floating net '" + n + "' (no driver), used by " + g.name);
    for (const auto& o : nl.outputs)
        if (!driver.count(o)) throw std::runtime_error("output '" + o + "' has no driver");

    return nl;
}