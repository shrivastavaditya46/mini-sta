#include <cmath>
#include <cstdlib>
#include <iostream>
#include "sta_engine.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { \
    std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n"; ++failures; } } while (0)

static bool close(double a, double b) { return std::fabs(a - b) < 1e-9; }

int main() {
    CellLib lib = {{"AND2", 0.20}, {"OR2", 0.25}, {"INV", 0.10}};
    Netlist nl;
    nl.inputs = {"A", "B", "C"};
    nl.outputs = {"Z"};
    nl.gates = {{"U1", "AND2", {"A", "B"}, "n1"},
                {"U2", "OR2", {"n1", "C"}, "n2"},
                {"U3", "INV", {"n2"}, "Z"}};

    TimingGraph g = buildGraph(nl, lib);
    std::vector<int> order;
    bool acyclic = topoSort(g, order);
    CHECK(acyclic);

    int z = g.id.at("Z");
    StaResult r = runSta(g, order, {z}, 1.0);
    CHECK(close(r.at[z], 0.55));
    CHECK(close(r.wns, 0.45));

    StaResult r2 = runSta(g, order, {z}, 0.5);
    CHECK(close(r2.wns, -0.05));
    CHECK(close(r2.tns, -0.05));

    if (failures == 0) std::cout << "all tests passed\n";
    return failures == 0 ? 0 : 1;
}
