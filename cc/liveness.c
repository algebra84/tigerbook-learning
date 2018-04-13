#include "temp.h"
#include "graph.h"
#include "flowgraph.h"
#include "liveness.h"

Live_moveList Live_MoveList(G_node src, G_node dst,
                            Live_moveList tail);
Temp_temp Live_gtemp(G_node n);
struct Live_graph Live_liveness(G_graph flow);

static void enterLiveMap(G_table t, G_node flowNode,
                         Temp_tempList temps) {
  G_enter(t, flowNode, temps);

}
static Temp_tempList lookupLiveMap(G_table t,
                                   G_node flownode) {
  return (Temp_tempList)G_look(t, flownode);

}
