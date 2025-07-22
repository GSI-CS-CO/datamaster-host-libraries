#include "graphHelper.h"

#include <block.h>
#include <graph.h>

namespace {
const std::string pattern = "pattern";
const std::string beamproc = "beamproc";
constexpr uint8_t cpu = 0;
constexpr uint32_t flags = 0;

} // namespace

myVertex makeNode(const std::string& name, const std::string& type) {
  uint32_t hash = std::hash<std::string>()(name + type);
  auto np = std::make_shared<BlockFixed>(name, pattern, beamproc, hash, cpu, flags);
  myVertex newNode;
  newNode.name = name;
  newNode.type = type;
  newNode.np = np;
  newNode.hash = hash;
  return newNode;
}

myEdge makeEdge(const std::string& type) {
  myEdge edge;
  edge.type = type;
  return edge;
}

Graph makeGraph(std::vector<TestVertexDescription> vertices, std::vector<TestEdgeDescription> edges) {
  Graph g;

  std::map<std::string, std::size_t> vertexIndexMap;
  for (const auto& vertex : vertices) {
    auto newNode = makeNode(vertex.name, vertex.type);
    auto newNodeIndex = g.m_vertices.size();
    g.m_vertices.push_back(newNode);
    g.added_vertex(newNodeIndex);
    vertexIndexMap[vertex.name] = newNodeIndex;
  }

  for (const auto& edge : edges) {
    auto firstVertexIt = vertexIndexMap.find(edge.source);
    auto secondVertexIt = vertexIndexMap.find(edge.target);
    assert(firstVertexIt != vertexIndexMap.end() && secondVertexIt != vertexIndexMap.end());

    myEdge newEdge = makeEdge(edge.type);
    std::size_t firstVertexIndex = firstVertexIt->second;
    std::size_t secondVertexIndex = secondVertexIt->second;
    add_edge(firstVertexIndex, secondVertexIndex, newEdge, g);
  }

  return g;
}

/**
 * Creates a graph with two nodes and an edge between them.
 * The start node is of type `startNodeType`, the end node is of type `endNodeType`, and the edge is of type `edgeType`.
 * The indices for the start node is 0 and for the end node is 1.
 */
Graph makeGraph(const char* startNodeType, const char* endNodeType, const char* edgeType) {
  Graph g =
      makeGraph({TestVertexDescription{"StartNode", startNodeType}, TestVertexDescription{"EndNode", endNodeType}},
                {TestEdgeDescription{"StartNode", "EndNode", edgeType}});

  return g;
}

Graph makeStarGraph(const char* centerNodeType, std::vector<LeafNodeDescription> leafNodeTypes) {
  std::vector<TestVertexDescription> vertices;
  std::vector<TestEdgeDescription> edges;

  vertices.push_back(TestVertexDescription{"CenterNode", centerNodeType});
  int32_t leafIndex = 0;
  for (std::size_t i = 0; i < leafNodeTypes.size(); ++i) {
    const auto& leaf = leafNodeTypes[i];
    for (int32_t j = 0; j < leaf.numLeaves; ++j) {
      std::string leafName = "LeafNode" + std::to_string(leafIndex);
      leafIndex++;
      vertices.push_back(TestVertexDescription{leafName.c_str(), leaf.type});
      edges.push_back(TestEdgeDescription{"CenterNode", leafName.c_str(), leaf.edgeType});
    }
  }

  return makeGraph(vertices, edges);
}