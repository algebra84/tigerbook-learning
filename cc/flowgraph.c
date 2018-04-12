#include "util.h"
#include "temp.h"
#include "assem.h"
#include "graph.h"
#include "flowgraph.h"

Temp_tempList FG_def(G_node n);
Temp_tempList FG_use(G_node n);
bool FG_isMove(G_node n);
G_graph FG_AssemFlowGraph(AS_instrList il);