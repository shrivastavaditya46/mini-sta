# mini-sta: A Static Timing Analysis Engine in C++

A command-line static timing analyzer (STA) for gate-level combinational netlists. It reads a netlist and a simple cell-delay library, builds a timing graph (a DAG), propagates arrival and required times through it, and reports slack, worst/total negative slack, and the critical path.

Static timing analysis is the step in a chip-design flow that checks whether a circuit meets its clock period *without simulating any input vectors*. Industry tools such as Synopsys PrimeTime and Cadence Tempus do this at the scale of billions of pins. This project implements the core algorithm from scratch at a small, readable scale.

> **Status:** `[TODO: update before publishing. Tick only what actually works.]`

---

## Table of Contents

1. [What it does](#what-it-does)
2. [How it works](#how-it-works)
3. [Tech stack](#tech-stack)
4. [Input formats](#input-formats)
5. [Build and run](#build-and-run)
6. [Example](#example)
7. [Project structure](#project-structure)
8. [Testing and validation](#testing-and-validation)
9. [Performance](#performance)
10. [Limitations](#limitations)
11. [Roadmap](#roadmap)
12. [What I learned](#what-i-learned)

---

## What it does

Given a gate-level netlist and a clock period, the tool answers: **"Does every signal reach every output before the clock edge, and if not, where is the problem?"**

**Core features (v1)**

- [ ] Parses a structural gate-level netlist (custom text format, see below)
- [ ] Parses a simplified cell-delay library
- [ ] Builds a timing graph (DAG) from the netlist
- [ ] Detects combinational loops (cycles) and reports them as errors
- [ ] Topological sort using Kahn's algorithm
- [ ] **Forward pass:** computes the arrival time (AT) at every net
- [ ] **Backward pass:** computes the required time (RT) at every net
- [ ] Computes **slack** (`RT - AT`) at every net
- [ ] Reports **WNS** (worst negative slack) and **TNS** (total negative slack)
- [ ] Extracts and prints the **critical path** pin by pin with per-stage delay
- [ ] Validates input and gives clear error messages (undefined gate type, multiple drivers, floating net, malformed lines)

**Stretch features**

- [ ] Report the *k* worst paths instead of just one
- [ ] Per-input arrival-time constraints
- [ ] Fan-out dependent delay (load-based delay model)
- [ ] Sequential support: register-to-register paths with setup checks
- [ ] Graphviz (`.dot`) export of the timing graph with the critical path highlighted

---

## How it works

### 1. Timing graph

Each **net** (signal) is a node. Each gate contributes one edge from every one of its input nets to its output net, weighted by the gate's delay from the library.

```
A ──┐
    ├─ AND2 (0.20) ──► n1 ──┐
B ──┘                       ├─ OR2 (0.25) ──► n2 ── INV (0.10) ──► Z
                      C ────┘
```

A valid combinational netlist is a directed acyclic graph. If the graph contains a cycle, the netlist has a combinational loop and static timing analysis is undefined, so the tool reports it and exits.

### 2. Topological order

Kahn's algorithm: compute the in-degree of every node, repeatedly pop nodes with in-degree 0, and append them to the order. If the final order has fewer nodes than the graph, a cycle exists. Time: **O(V + E)**.

### 3. Forward pass (arrival times)

Process nodes in topological order:

```
AT(primary input) = input arrival time (default 0)
AT(n) = max over all fan-in edges (u → n) of [ AT(u) + delay(u → n) ]
```

The `max` is what makes the analysis conservative: the latest-arriving input determines when the output settles. Each node also stores a **predecessor pointer** to the input that produced the max, which is used later to trace the critical path.

### 4. Backward pass (required times)

Process nodes in reverse topological order:

```
RT(primary output) = clock period
RT(n) = min over all fan-out edges (n → v) of [ RT(v) - delay(n → v) ]
```

### 5. Slack

```
slack(n) = RT(n) - AT(n)
```

- **slack ≥ 0:** timing met at this net
- **slack < 0:** timing violated; the signal arrives too late
- **WNS** = the minimum slack over all endpoints
- **TNS** = the sum of negative endpoint slacks (0 if none)

### 6. Critical path

The critical path is the path through the graph with the smallest slack. Starting from the endpoint with WNS, follow the stored predecessor pointers back to a primary input, then print the path in forward order.

### Complexity

Parsing is linear in input size. Topological sort and both passes are **O(V + E)**, so the whole analysis is linear in the number of gates and connections.

---

## Tech stack

| Area | Choice |
|---|---|
| Language | C++17 |
| Build | CMake (a plain `Makefile` is also fine) |
| Standard library | STL only (`vector`, `unordered_map`, `queue`, `string`); no external dependencies for the core |
| Platform | Linux (developed on `[TODO: Ubuntu / WSL2]`) |
| Compiler | GCC `[TODO: version]` with `-Wall -Wextra -O2` |
| Debugging | GDB, Valgrind (memory checks), AddressSanitizer |
| Testing | `[TODO: plain assert-based tests / Catch2 / GoogleTest]` |
| Tooling scripts | Python 3 (random netlist generator for scale tests) |
| Optional | Graphviz for visualizing the timing graph |
| Version control | Git |

---

## Input formats

Both formats are intentionally simple and line-based. Parsing real Verilog and Liberty files is a large project of its own, and this project focuses on the timing algorithm.

### Netlist (`.net`)

```
# comments start with '#'
INPUT  A B C
OUTPUT Z
GATE U1 AND2 A B n1
GATE U2 OR2  n1 C n2
GATE U3 INV  n2 Z
```

- `INPUT` lists primary inputs and `OUTPUT` lists primary outputs.
- `GATE <instance> <type> <in1> [in2 ...] <out>`: the **last** token is the output net, and all tokens before it (after the type) are inputs.

### Cell library (`.lib`)

```
# type   delay_ns
INV      0.10
NAND2    0.15
AND2     0.20
OR2      0.25
XOR2     0.30
```

Delays are a flat value per gate type in this version. A load-dependent model is on the roadmap.

### Clock period

Passed on the command line with `--clock <ns>`.

---

## Build and run

```bash
git clone https://github.com/[TODO: your-username]/mini-sta.git
cd mini-sta
mkdir build && cd build
cmake .. && make
```

```bash
./sta --netlist ../examples/simple.net --lib ../examples/simple.lib --clock 1.0
```

| Flag | Meaning |
|---|---|
| `--netlist <file>` | Netlist file |
| `--lib <file>` | Cell library file |
| `--clock <ns>` | Clock period in nanoseconds |
| `--paths <k>` | Report the k worst paths (stretch; default 1) |
| `--dot <file>` | Write a Graphviz graph (stretch) |

---

## Example

The numbers below were worked out by hand for the netlist in [Input formats](#input-formats) and are what the tool should produce. `[TODO: replace with your tool's actual output once it runs.]`

**Clock period = 1.0 ns**

```
=== Static Timing Report ===
Clock period : 1.000 ns
Endpoints    : 1
WNS          : 0.450 ns (MET)
TNS          : 0.000 ns

Critical path (slack = 0.450 ns)
  Point       Gate    Incr     Arrival
  A (input)   -       0.000    0.000
  n1          AND2    0.200    0.200
  n2          OR2     0.250    0.450
  Z           INV     0.100    0.550
  data required time           1.000
  slack (MET)                  0.450
```

**Clock period = 0.5 ns** (same netlist, violation):

```
WNS : -0.050 ns (VIOLATED)
TNS : -0.050 ns
```

Here the path takes 0.55 ns against a 0.5 ns budget. Input `B` ties with `A` on this path. The tool breaks ties deterministically, using the first input listed on the gate.

---

## Project structure

```
mini-sta/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp              # CLI parsing, orchestration
│   ├── netlist_parser.h/.cpp # netlist + validation
│   ├── lib_parser.h/.cpp     # cell library
│   ├── timing_graph.h/.cpp   # nodes, edges, topological sort, cycle check
│   ├── sta_engine.h/.cpp     # forward/backward pass, slack, critical path
│   └── report.h/.cpp         # formatted output
├── tests/
│   ├── test_parser.cpp
│   ├── test_graph.cpp
│   └── test_sta.cpp
├── examples/
│   ├── simple.net / simple.lib
│   ├── reconvergent.net      # reconvergent fan-out
│   └── loop.net              # combinational loop (should error)
└── scripts/
    └── gen_netlist.py        # random DAG generator for scale tests
```

---

## Testing and validation

- **Hand-verified cases:** small netlists where the expected AT/RT/slack and critical path were computed on paper.
- **Edge cases:**
  - combinational loop
  - undefined gate type
  - net driven by more than one gate
  - net with no driver (floating input)
  - gate with a single input
  - reconvergent fan-out, where the same net feeds two paths that rejoin
  - multiple outputs with different slacks
  - tied paths
- **Memory safety:** run clean under Valgrind and `-fsanitize=address,undefined`.
- **Cross-check:** `[TODO: optional. Compare results with OpenSTA on the same netlist, if you set it up.]`

```bash
cd build && ctest --output-on-failure
```

---

## Performance

`[TODO: fill in only with numbers you measured. Do not publish estimates.]`

Random netlists produced by `scripts/gen_netlist.py`:

| Gates | Edges | Parse (ms) | Analysis (ms) | Peak memory (MB) |
|---|---|---|---|---|
| 1,000 | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` |
| 10,000 | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` |
| 100,000 | `[TODO]` | `[TODO]` | `[TODO]` | `[TODO]` |

Measured on `[TODO: CPU, RAM, OS]`, compiled with `-O2`.

---

## Limitations

This is a learning-scale tool. It is **not** a replacement for production STA, and it deliberately simplifies:

- Combinational logic only (no flip-flops or clock-to-Q or setup/hold checks) unless the sequential stretch goal is done
- A single flat delay per gate type: no input slew, output load, or wire (RC) delay
- A single corner: no process/voltage/temperature variation, no min/max analysis
- No false-path or multicycle-path handling, and no clock-domain handling
- A custom netlist and library format, not real Verilog or Liberty

---

## Roadmap

1. Load-dependent delay (a function of fan-out)
2. Sequential timing: register-to-register paths and setup/hold
3. k-worst path reporting
4. A structural-Verilog front end
5. A reader for a subset of the Liberty format
6. Slew propagation and a table-based delay model

---



---

## Author

Aditya Shrivastava, B.Tech ECE, NIT Kurukshetra
`[TODO: LinkedIn / email]`

## License

MIT `[TODO: add a LICENSE file or remove this section]`
