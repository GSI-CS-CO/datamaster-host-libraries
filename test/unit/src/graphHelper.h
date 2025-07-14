#pragma once
#include <boost/make_shared.hpp>
#include <graph.h>
#include <memory>
#include <string>
#include <vector>

struct TestVertexDescription {
  const std::string name;
  const std::string type;
};

struct TestEdgeDescription {
  const std::string source;
  const std::string target;
  const std::string type;
};

struct TestGraphDescription {
  std::vector<TestVertexDescription> vertices;
  std::vector<TestEdgeDescription> edges;
};

struct LeafNodeDescription {
  const std::string type;
  const std::string edgeType;
  int32_t numLeaves;
};

myVertex makeNode(const std::string& name, const std::string& type);
myEdge makeEdge(const std::string& type);
Graph makeGraph(const char* startNodeType, const char* endNodeType, const char* edgeType);
Graph makeGraph(std::vector<TestVertexDescription> vertices, std::vector<TestEdgeDescription> edges);
Graph makeStarGraph(const char* centerNodeType, std::vector<LeafNodeDescription> leafNodeTypes);