#include <lzma.h>
#include "util.h"
#include "temp.h"
#include "assem.h"
#include "graph.h"
#include "flowgraph.h"

static G_table usetable;
static G_table deftable;


Temp_tempList FG_def(G_node n){
  return G_look(deftable, n);
}

Temp_tempList FG_use(G_node n){
  return G_look(usetable, n);
}

bool FG_isMove(G_node n);
G_graph FG_AssemFlowGraph(AS_instrList il){
  AS_instr insn;
  G_node node = NULL,prev_node = NULL;

  L_table jumptable;
  G_graph Cfg;

  //init table
  usetable = G_empty();
  deftable = G_empty();
  jumptable = L_empty();
  Cfg = G_Graph();

  // Traversal insns
  for(; il; il=il->tail){
    insn = il->head;
    node = G_Node(Cfg,insn);
    if(prev_node)
      G_addEdge(prev_node, node);
    switch(insn->kind){
      case I_OPER:{
        G_enter(usetable, node, insn->u.OPER.src);
        G_enter(deftable, node, insn->u.OPER.dst);
        if(insn->u.OPER.jumps) {
          Temp_label label = insn->u.OPER.jumps->labels->head;
          G_nodeList nlist = L_look(jumptable,label);
          L_enter(jumptable, label, G_NodeList(node,nlist));
        }
        break;
      }
      case I_LABEL:{
        G_nodeList nlist = L_look(jumptable,insn->u.LABEL.label);
        for(;nlist;nlist = nlist->tail)
          G_addEdge(nlist->head, node);
        break;
      }
      case I_MOVE:{
        // do nothing if src == dst
        if(insn->u.MOVE.src->head == insn->u.MOVE.dst->head)
          break;
        G_enter(usetable, node, insn->u.MOVE.src);
        G_enter(deftable, node, insn->u.MOVE.dst);
        break;
      }
      default:
        assert(0);
    }
    prev_node = node;
  }
  return Cfg;
}
