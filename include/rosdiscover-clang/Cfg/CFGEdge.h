#pragma once

#include <string>

#include <clang/AST/Stmt.h>

#include "CFGBlock.h"

namespace rosdiscover {
class CFGBlock;
class CFGEdge {
public:

  enum EdgeType {
      True,
      False,
      Normal,
      Unknown,
  };

  CFGEdge(
    CFGBlock* predecessor,
    CFGBlock* successor,
    EdgeType edgeType
  ) : predecessor(predecessor), successor(successor), edgeType(edgeType) {}

  CFGEdge(const CFGEdge &edge) : predecessor(edge.predecessor), successor(edge.successor), edgeType(edge.edgeType) {}

  ~CFGEdge(){}

  EdgeType getType() {
    return edgeType;
  }

  CFGBlock* getSuccessor() {
    return successor;
  }

  CFGBlock* getPredecessor() {
    return predecessor;
  }

private:
  CFGBlock* predecessor;
  CFGBlock* successor;
  EdgeType edgeType;
};

} // rosdiscover
