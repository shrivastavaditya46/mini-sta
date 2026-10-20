#include <iostream>
#include <string>
#include "lib_parser.h"
#include "netlist_parser.h"
#include "timing_graph.h"
#include "sta_engine.h"
#include "report.h"

static void usage() {
    std::cerr << "usage: sta --netlist <file> --lib <file> --clock <ns>\n";
}

int main(int argc, char** argv) {
    std::string netPath, libPath;
    double clock = -1;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--netlist" && i + 1 < argc) netPath = argv[++i];
        else if (a == "--lib" && i + 1 < argc) libPath = argv[++i];
        else if (a == "--clock" && i + 1 < argc) clock = std::stod(argv[++i]);
        else { usage(); return 1; }
    }
    if (netPath.empty() || libPath.empty() || clock <= 0) { usage(); return 1; }

    try {
        CellLib lib = parseLib(libPath);
        Netlist nl = parseNetlist(netPath, lib);
        TimingGraph g = buildGraph(nl, lib);

        std::vector<int> order;
        if (!topoSort(g, order)) {
            std::vector<char> seen(g.size(), 0);
            for (int v : order) seen[v] = 1;
            std::cerr << "error: combinational loop detected. Nets in or behind the loop:";
            for (int v = 0; v < g.size(); ++v)
                if (!seen[v]) std::cerr << " " << g.names[v];
            std::cerr << "\n";
            return 2;
        }

        std::vector<int> outs;
        for (const auto& o : nl.outputs) outs.push_back(g.id.at(o));
        StaResult r = runSta(g, order, outs, clock);
        printReport(g, r, clock);
        return r.wns >= -1e-9 ? 0 : 3;     // nonzero exit if timing violated
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}