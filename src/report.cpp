#include "report.h"
#include <cstdio>

void printReport(const TimingGraph& g, const StaResult& r, double clock) {
    bool met = r.wns >= -1e-9;
    std::printf("=== Static Timing Report ===\n");
    std::printf("Clock period : %.3f ns\n", clock);
    std::printf("Endpoints    : %zu\n", r.endpoints.size());
    std::printf("WNS          : %.3f ns (%s)\n", r.wns, met ? "MET" : "VIOLATED");
    std::printf("TNS          : %.3f ns\n\n", r.tns);

    std::vector<int> path = criticalPath(g, r);
    if (path.empty()) return;

    std::printf("Critical path (slack = %.3f ns)\n", r.wns);
    std::printf("  %-12s %-7s %-8s %s\n", "Point", "Gate", "Incr", "Arrival");
    int start = g.edges[path.front()].from;
    std::printf("  %-12s %-7s %-8.3f %.3f\n",
                (g.names[start] + " (input)").c_str(), "-", 0.0, r.at[start]);
    for (int e : path) {
        const Edge& ed = g.edges[e];
        std::printf("  %-12s %-7s %-8.3f %.3f\n",
                    g.names[ed.to].c_str(), ed.gateType.c_str(), ed.delay, r.at[ed.to]);
    }
    std::printf("  data required time           %.3f\n", clock);
    std::printf("  slack (%s)%*s%.3f\n", met ? "MET" : "VIOLATED", met ? 17 : 12, "", r.wns);
}