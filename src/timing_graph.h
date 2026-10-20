#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "netlist_parser.h"

struct Edge {
    int from, to;
    double delay;
    std::string gate, gateType;   // kept only for reporting
};

struct TimingGraph {
    std::vector<std::string> names;                // node id -> net name
    std::unordered_map<std::string, int> id;       // net name -> node id
    std::vector<Edge> edges;
    std::vector<std::vector<int>> fanin, fanout;   // lists of edge indices per node

    int node(const std::string& name);             // get-or-create
    int size() const { return (int)names.size(); }
};

TimingGraph buildGraph(const Netlist& nl, const CellLib& lib);
bool topoSort(const TimingGraph& g, std::vector<int>& order);  // false if cycle