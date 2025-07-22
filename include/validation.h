#ifndef _VALIDATION_H_
#define _VALIDATION_H_

#include <stdint.h>
#include <string>
#include <set>

#include "graph.h"
#include "common.h"

//Multimap for node pair validation
namespace Validation {

typedef std::string edgeType_t;
typedef std::string nodeType_t;
typedef std::set<nodeType_t> children_t;

void init(); //workaround for boost v1.53 which doesn't support aggregate initalization

void eventSequenceCheck(vertex_t v, Graph& g, bool force); //check if event sequence is well behaved
void metaSequenceCheck(vertex_t v, Graph& g); //check if meta tree is well behaved
void neighbourhoodCheck(vertex_t v, Graph& g); //check if all outedge (nodetype/edgetype/childtype) tupels are valid and occurrence count is within valid bounds
void neighbourhoodCheckCpu(vertex_t v, Graph& g); // check that outedges of type defdst, altdst have targets on the same CPU.

  namespace Aux {
    void metaSequenceCheckAux(vertex_t v, vertex_t vcurrent, Graph& g, unsigned int recursionLvl = 0);
  }
}



#endif
