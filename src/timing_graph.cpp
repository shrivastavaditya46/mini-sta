#include "timing_graph.h"
#include <queue>

int TimingGraph::node(const std::string& name) {
    auto it = id.find(name);
    if (it != id.end()) return it->second;
    int n = (int)names.size();
    id[name] = n;
    names.push_back(name);
    fanin.emplace_back();
    fanout.emplace_back();
    return n;
}

TimingGraph buildGraph(const Netlist& nl, const CellLib& lib) {
    TimingGraph g;
    for (const auto& n : nl.inputs)  g.node(n);
    for (const auto& n : nl.outputs) g.node(n);
    for (const Gate& gate : nl.gates) {
        int out = g.node(gate.output);
        double d = lib.at(gate.type);                 // validated earlier
        for (const auto& in : gate.inputs) {          // one edge per input pin
            int u = g.node(in);
            int e = (int)g.edges.size();
            g.edges.push_back({u, out, d, gate.name, gate.type});
            g.fanout[u].push_back(e);
            g.fanin[out].push_back(e);
        }
    }
    return g;
}

bool topoSort(const TimingGraph& g, std::vector<int>& order) {
    order.clear();
    std::vector<int> indeg(g.size());
    std::queue<int> q;
    for (int v = 0; v < g.size(); ++v) {
        indeg[v] = (int)g.fanin[v].size();
        if (indeg[v] == 0) q.push(v);
    }
    while (!q.empty()) {                              // Kahn's algorithm
        int v = q.front(); q.pop();
        order.push_back(v);
        for (int e : g.fanout[v])
            if (--indeg[g.edges[e].to] == 0) q.push(g.edges[e].to);
    }
    return (int)order.size() == g.size();             // fewer => cycle
}