#pragma once
#include <vector>
#include "timing_graph.h"

struct StaResult {
    std::vector<double> at, rt, slack;
    std::vector<int> predEdge;       // edge that produced the max AT (-1 for inputs)
    std::vector<int> endpoints;
    int worstEndpoint = -1;
    double wns = 0, tns = 0;
};

StaResult runSta(const TimingGraph& g, const std::vector<int>& order,
                 const std::vector<int>& outputs, double clock);

// Edge indices of the critical path, in forward order
std::vector<int> criticalPath(const TimingGraph& g, const StaResult& r);