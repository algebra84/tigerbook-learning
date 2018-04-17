#include "table.h"
#include "temp.h"
#include "graph.h"
#include "flowgraph.h"
#include "liveness.h"
#include "assem.h"



static G_table liveout = NULL;
static G_table livein = NULL;
static Temp_map Temp_Gnode_table = NULL;

Live_moveList Live_MoveList(G_node src, G_node dst,
                            Live_moveList tail){
  Live_moveList lm = checked_malloc(sizeof(*lm));
  lm->src = src;
  lm->dst = dst;
  lm->tail = tail;
  return lm;
}

Temp_temp Live_gtemp(G_node n){
  return (Temp_temp)G_nodeInfo(n);
}
// hashmap: key: Temp, value: GNode
static void TempGnode_enter(Temp_map m, Temp_temp t, G_node node) {
  assert(m && m->tab);
  TAB_enter(m->tab,t,node);
}

// hashmap: key: Temp, value: GNode
static G_node TempGnode_look(Temp_map m, Temp_temp t) {
  assert(m && m->tab);
  G_node node;
  node = TAB_look(m->tab, t);
  if (node)
    return node;
  else if (m->under)
    return TempGnode_look(m->under, t);
  else
    return NULL;
}

// hashMap:: key Gnode, value: Temp_tempList
static void enterLiveMap(G_table t, G_node flowNode,
                         Temp_tempList temps) {
  G_enter(t, flowNode, temps);
}

// hashMap:: key Gnode, value: Temp_tempList
static Temp_tempList lookupLiveMap(G_table t,
                                   G_node flownode) {
  return (Temp_tempList)G_look(t, flownode);
}


static bool T_inTemplist(Temp_temp t, Temp_tempList list){
  for(; list; list=list->tail)
    if(t == list->head)
      return TRUE;
  return FALSE;
}

struct Live_graph Live_liveness(G_graph flow){

  // init basic data
  struct Live_graph res;
  res.moves = NULL;
  res.graph = G_Graph();
  liveout = G_empty();
  livein = G_empty();
  Temp_Gnode_table = Temp_empty();
  bool change = TRUE;
  G_nodeList nlist = G_nodes(flow);
  G_nodeList iterlist = nlist;

  // anylysis livein/out
  while(change){
    change = FALSE;
    for(iterlist = nlist; iterlist; iterlist = iterlist->tail){
      G_nodeList succ = G_succ(iterlist->head);
      Temp_tempList tout = lookupLiveMap(liveout, iterlist->head);
      Temp_tempList tin = lookupLiveMap(livein, iterlist->head);

      // out[n] = union of all in[s], s belong to all succs to n
      for(;succ; succ = succ->tail){
        Temp_tempList tin = lookupLiveMap(livein, succ->head);
        for(; tin; tin = tin->tail){
          if(T_inTemplist(tin->head,tout))
            continue;
          tout = Temp_TempList(tin->head, tout);
          change = TRUE;
        }
      }
      enterLiveMap(liveout,iterlist->head, tout);

      // in[n] = use[n] U (out[n] - def[n])
      Temp_tempList defs = FG_def(iterlist->head);
      Temp_tempList uses = FG_use(iterlist->head);

      for(; uses; uses=uses->tail) {
        if (T_inTemplist(uses->head, tin))
          continue;
        tin = Temp_TempList(uses->head, tin);
        change = TRUE;
      }
      for(; tout; tout=tout->tail){
        if(T_inTemplist(tout->head, tin))
          continue;
        //exclude defs
        if(T_inTemplist(tout->head, defs))
          continue;
        tin = Temp_TempList(tout->head, tin);
        change = TRUE;
      }
      enterLiveMap(livein, iterlist->head, tin);

      // debug see liveout/livein
#ifdef __DEBUG__
      {
        Temp_tempList tout = lookupLiveMap(liveout, iterlist->head);
        Temp_tempList tin = lookupLiveMap(livein, iterlist->head);
        fprintf(stdout, "\n gnode: %d", G_Nodekey(iterlist->head));
        Temp_tempList debuglist = tout;
        fprintf(stdout, "\n liveout: ");
        for(; debuglist; debuglist=debuglist->tail)
          fprintf(stdout,"L%d, ", Temp_key(debuglist->head));
        fprintf(stdout, "\n livein: ");
        for(debuglist = tin; debuglist; debuglist=debuglist->tail)
          fprintf(stdout,"L%d, ", Temp_key(debuglist->head));
        fprintf(stdout, "\n");
      }
#endif
    }
  }

#ifdef __DEBUG__
  // debug see defs/uses
  for(iterlist = nlist; iterlist; iterlist=iterlist->tail){
    Temp_tempList defs = FG_def(iterlist->head);
    Temp_tempList uses = FG_use(iterlist->head);
    fprintf(stdout, "\nGnode: %d", G_Nodekey(iterlist->head));
    fprintf(stdout,"\ndefs: ");
    for(; defs; defs=defs->tail)
      fprintf(stdout, " %d, ", Temp_key(defs->head));
    fprintf(stdout,"\nuses: ");
    for(; uses; uses=uses->tail)
      fprintf(stdout, " %d, ", Temp_key(uses->head));
    fprintf(stdout, "\n___________\n");
  }
#endif

  // build interference map
  for(iterlist = nlist; iterlist; iterlist=iterlist->tail){
    AS_instr pas = G_nodeInfo(iterlist->head);
    Temp_tempList defs = FG_def(iterlist->head);
    Temp_tempList tout = lookupLiveMap(liveout, iterlist->head);
    for(; defs; defs=defs->tail) {
      G_node gdef = TempGnode_look(Temp_Gnode_table, defs->head);
      if(!gdef) {
        gdef = G_Node(res.graph, defs->head);
        TempGnode_enter(Temp_Gnode_table,defs->head, gdef);
      }
      for(; tout; tout=tout->tail){
        G_node gout = TempGnode_look(Temp_Gnode_table, tout->head);
        if(!gout) {
          gout = G_Node(res.graph, tout->head);
          TempGnode_enter(Temp_Gnode_table, tout->head, gout);
        }
        if(pas->kind == I_MOVE
           && pas->u.MOVE.src->head == tout->head) {
          res.moves = Live_MoveList(gout, gdef, res.moves);
          continue;
        }
        G_addEdge(gdef, gout);
      }
    }
  }

  return res;
}

