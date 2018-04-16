#include "table.h"
#include "temp.h"
#include "graph.h"
#include "flowgraph.h"
#include "liveness.h"
#include "assem.h"

Live_moveList Live_MoveList(G_node src, G_node dst,
                            Live_moveList tail);
Temp_temp Live_gtemp(G_node n);

struct Livearray_{
    Temp_temp *array;
    int num;
};

typedef struct Livearray_* Livearray;

static G_table liveout = NULL;
static G_table livein = NULL;
static Temp_map numMap;


static Livearray LiveArray(int n){
  Livearray p = checked_malloc(sizeof(*p));
  p->array = checked_malloc(n * sizeof(Temp_temp));
  p->num = n;
  for(int i = 0; i != n; i++)
    p->array[i] = NULL;
  return p;
}


static void TempNum_enter(Temp_map m, Temp_temp t, int s) {
  assert(m && m->tab);
  int* p = checked_malloc(sizeof(int));
  *p = s;
  TAB_enter(m->tab,t,p);
}

static int* TempNum_look(Temp_map m, Temp_temp t) {
  int* s;
  assert(m && m->tab);
  s = TAB_look(m->tab, t);
  if (s) return s;
  else if (m->under)
    return TempNum_look(m->under, t);
  else
    return NULL;
}

static Temp_map GetTempNumMap(G_graph flow, int* num){
  int i = 0;
  Temp_map m = Temp_empty();
  G_nodeList nlist = G_nodes(flow);
  for(; nlist; nlist=nlist->tail){
    Temp_tempList use = FG_use(nlist->head);
    Temp_tempList def = FG_def(nlist->head);

    for(;use; use=use->tail) {
      if(TempNum_look(m,use->head))
        continue;
      TempNum_enter(m, use->head,i++);
    }

    for(;def; def=def->tail){
      if(TempNum_look(m,def->head))
        continue;
      TempNum_enter(m, def->head,i++);
    }
  }
  *num = i;
  return m;
}
struct Live_graph Live_liveness(G_graph flow){
  struct Live_graph res;
  res.moves = NULL;
  res.graph = G_Graph();
  int num = 0;
  liveout = G_empty();
  livein = G_empty();
  bool change = TRUE;
  numMap = GetTempNumMap(flow,&num);
  G_nodeList nlist = G_nodes(flow);
  G_nodeList iterlist = nlist;
  Temp_temp* temparray = checked_malloc(sizeof(Temp_temp)*num);

  // init live in/out
  for(iterlist = nlist; iterlist; iterlist = iterlist->tail){
    G_enter(liveout, iterlist->head, LiveArray(num));
    G_enter(livein, iterlist->head, LiveArray(num));
  }

  // anylysis livein/out
  while(change){
    change = FALSE;
    for(iterlist = nlist; iterlist; iterlist = iterlist->tail){
      G_nodeList succ = G_succ(iterlist->head);
      Livearray pout = G_look(liveout, iterlist->head);

      // out[n] = union of all in[s], s belong to all succs to n
      for(;succ; succ = succ->tail){
        Livearray pin = G_look(liveout, succ->head);
        for(int i = 0; i != pin->num; i++){
          if(pin->array[i] == NULL)
            continue;
          if(pout->array[i] == NULL) {
            change = TRUE;
            pout->array[i] = pin->array[i];
          }
          else
            assert(pout->array[i] == pin->array[i]);
        }
      }

      // in[n] = use[n] U (out[n] - def[n])
      Temp_tempList defs = FG_use(iterlist->head);
      Temp_tempList uses = FG_def(iterlist->head);
      Livearray pin = G_look(livein, iterlist->head);

      for(int i = 0; i != pout->num; i++)
        temparray[i] = pout->array[i];

      for(;defs; defs=defs->tail){
        int *p = TempNum_look(numMap,defs->head);
        temparray[*p] = NULL;
      }

      for(; uses; uses=uses->tail){
        int* p = TempNum_look(numMap, uses->head);
        temparray[*p] = uses->head;
      }

      for(int i = 0; i != num; i++){
        if(pin->array[i] != temparray[i])
          change = TRUE;
        pin->array[i] = temparray[i];
      }
    }
  }

  // build interference map
  G_node* temp_visited = checked_malloc(sizeof(G_node)*num);
  for(int i = 0; i != num; i++)
    temp_visited[i] = NULL;
  for(iterlist = nlist; iterlist; iterlist=iterlist->tail){
    AS_instr pas = G_nodeInfo(iterlist->head);
    Temp_tempList defs = FG_use(iterlist->head);
    Livearray pout = G_look(liveout, iterlist->head);
    for(; defs; defs=defs->tail) {
      int *p = TempNum_look(numMap,defs->head);
      if(temp_visited[*p] == NULL)
        temp_visited[*p] = G_Node(res.graph, defs->head);
      for (int i = 0; i != pout->num; i++) {
        if (pout->array[i]) {
          if (!temp_visited[i])
            temp_visited[i] = G_Node(res.graph, pout->array[i]);
          if(pas->kind == I_MOVE
                  && pas->u.MOVE.src->head == pout->array[i])
            continue;
          G_addEdge(temp_visited[*p], temp_visited[i]);
        }
      }
    }
  }

  return res;
}

static Temp_tempList lookupLiveMap(G_table t,
                                   G_node flownode) {
  Livearray p = G_look(t, flownode);
  Temp_tempList res = NULL;
  for(int i = 0; i != p->num; i++)
    if(p->array[i])
      res = Temp_TempList(p->array[i],res);
}
