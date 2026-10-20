#include "sta_engine.h"
#include <algorithm>
#include <limits>

StaResult runSta(const TimingGraph& g, const std::vector<int>& order,
                 const std::vector<int>& outputs, double clock) {
    const double INF = std::numeric_limits<double>::infinity();
    int n = g.size();
    StaResult r;
    r.at.assign(n, 0.0);
    r.rt.assign(n, INF);
    r.slack.assign(n, INF);
    r.predEdge.assign(n, -1);
    r.endpoints = outputs;

    // Forward pass: AT(n) = max over fan-in of AT(u) + delay
    for (int v : order) {
        if (g.fanin[v].empty()) continue;             // primary input: AT = 0
        double best = -INF;
        for (int e : g.fanin[v]) {
            double cand = r.at[g.edges[e].from] + g.edges[e].delay;
            if (cand > best) { best = cand; r.predEdge[v] = e; }  // strict > : first input wins ties
        }
        r.at[v] = best;
    }

    // Backward pass: RT(n) = min over fan-out of RT(v) - delay
    for (int o : outputs) r.rt[o] = clock;
    for (auto it = order.rbegin(); it != order.rend(); ++it) {
        int v = *it;
        for (int e : g.fanout[v])
            r.rt[v] = std::min(r.rt[v], r.rt[g.edges[e].to] - g.edges[e].delay);
    }

    for (int v = 0; v < n; ++v) r.slack[v] = r.rt[v] - r.at[v];

    // WNS / TNS over endpoints only
    r.wns = INF;
    for (int o : outputs) {
        if (r.slack[o] < r.wns) { r.wns = r.slack[o]; r.worstEndpoint = o; }
        if (r.slack[o] < 0) r.tns += r.slack[o];
    }
    return r;
}

std::vector<int> criticalPath(const TimingGraph& g, const StaResult& r) {
    std::vector<int> path;
    int v = r.worstEndpoint;
    while (v != -1 && r.predEdge[v] != -1) {          // walk predecessor pointers back
        int e = r.predEdge[v];
        path.push_back(e);
        v = g.edges[e].from;
    }
    std::reverse(path.begin(), path.end());
    return path;
}