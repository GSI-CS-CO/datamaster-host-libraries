#include <gtest/gtest.h>

#include <string>
#include <tuple>

#include "block.h"
#include "dotstr.h"
#include "validation.h"

using namespace testing;

namespace {
const std::string pattern = "pattern";
const std::string beamproc = "beamproc";
constexpr uint8_t cpu = 0;
constexpr uint32_t flags = 0;

struct TestVertexDescription {
  const char* name;
  const char* type;
};

struct TestEdgeDescription {
  const char* source;
  const char* target;
  const char* type;
};

myVertex makeNode(const std::string& name, const std::string& type) {
  uint32_t hash = std::hash<std::string>()(name + type);
  auto np = boost::make_shared<BlockFixed>(name, pattern, beamproc, hash, cpu, flags);
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
} // namespace

class DatamasterNoRuleValidation : public testing::Test,
                                   public WithParamInterface<std::tuple<const char*, const char*, const char*>> {};

INSTANTIATE_TEST_SUITE_P(
    AllCombinations, DatamasterNoRuleValidation,
    Combine(Values("tmsg", "noop", "flow", "switch", "origin", "flush", "wait", "start", "stop", "abort", "startthread",
                   "lock", "unlock", "asyncclear", "block", "blockalign", "qinfo", "listdst", "qbuf", "meta", "global"),
            Values("tmsg", "noop", "flow", "switch", "origin", "flush", "wait", "start", "stop", "abort", "startthread",
                   "lock", "unlock", "asyncclear", "block", "blockalign", "qinfo", "listdst", "qbuf", "meta", "global"),
            Values("listdst", "defdst", "altdst", "baddefdst", "target", "switchdst", "origindst", "flowdst",
                   "flushovr", "dynid", "dynpar0", "dynpar1", "dyntef", "dynres", "meta", "reference", "reference2",
                   "address", "write")));

TEST_P(DatamasterNoRuleValidation, TestInvalidCombinations) {
  auto [startNodeType, endNodeType, edgeType] = GetParam();
  auto it = Validation::cRules.get<Validation::Constellation>().find(boost::make_tuple(startNodeType, edgeType));
  const bool foundRule = (it != Validation::cRules.get<Validation::Constellation>().end());
  if (foundRule) {
    GTEST_SKIP();
    return;
  }

  auto g = makeGraph(startNodeType, endNodeType, edgeType);
  Validation::init();
  EXPECT_THROW(Validation::neighbourhoodCheck(0, g), std::runtime_error);
}

TEST(TestInvalidGraphs, TestInvalidNodeName) {
  // This test checks that a node with an invalid name throws an error.
  Graph g = makeGraph("invalidname", "tmsg", "defdst");
  Validation::init();
  EXPECT_THROW(Validation::neighbourhoodCheck(0, g), std::runtime_error);
}

TEST(TestInvalidGraphs, TestInvalidEdgeType) {
  // This test checks that a graph with an invalid edge type throws an error.
  Graph g = makeGraph("tmsg", "tmsg", "invalidedge");
  Validation::init();
  EXPECT_THROW(Validation::neighbourhoodCheck(0, g), std::runtime_error);
}

// TMSG

Graph makeValidTmsgGraph() { return makeGraph("tmsg", "tmsg", "defdst"); }

TEST(TestTmsgNode, ShouldFailWithoutDefDst) {
  // This test checks that a TMsg node without a DefDst edge throws an error.
  Graph g = makeGraph("tmsg", "tmsg", "dynpar0");
  Validation::init();
  EXPECT_THROW(Validation::neighbourhoodCheck(0, g), std::runtime_error);
}

TEST(TestTmsgNode, ShouldPassWithDefDst) {
  // This test checks that a TMsg node with a DefDst edge passes validation.
  Graph g = makeGraph("tmsg", "tmsg", "defdst");

  Validation::init();
  EXPECT_NO_THROW(Validation::neighbourhoodCheck(0, g));
}

TEST(TestTmsgNode, ShouldFailWithTooManyDefDst) {
  Graph g = makeGraph("tmsg", "tmsg", "defdst");
  auto n = makeNode("ExtraNode", "tmsg");
  auto e = makeEdge("defdst");

  auto newNodeIndex = g.m_vertices.size();
  g.m_vertices.push_back(n);
  g.added_vertex(newNodeIndex);

  add_edge(0, newNodeIndex, e, g); // Add an extra DefDst edge

  Validation::init();
  EXPECT_THROW(Validation::neighbourhoodCheck(0, g), std::runtime_error);
}

TEST(TestTmsgNode, ShouldFailWithTooManyDynPar0) {
  // This test checks that a TMsg node with too many DynPar0 edges throws an error.
  Graph g = makeValidTmsgGraph();

  auto n = makeNode("ExtraNode", "tmsg");
  auto nIndex = g.m_vertices.size();
  g.m_vertices.push_back(n);
  g.added_vertex(nIndex);

  auto n2 = makeNode("ExtraNode2", "tmsg");
  auto n2Index = g.m_vertices.size();
  g.m_vertices.push_back(n2);
  g.added_vertex(n2Index);

  auto e = makeEdge("dynpar0");
  auto e2 = makeEdge("dynpar0");
  add_edge(0, nIndex, e, g); // Add an extra DynPar0 edge
  add_edge(0, n2Index, e2, g);

  Validation::init();
  EXPECT_THROW(Validation::neighbourhoodCheck(0, g), std::runtime_error);
}

TEST(TestTmsgNode, ShouldFailWithTooManyDynPar1) {
  // This test checks that a TMsg node with too many DynPar1 edges throws an error.
  Graph g = makeGraph({{"StartNode", "tmsg"}, {"EndNode", "tmsg"}, {"ExtraNode", "tmsg"}, {"ExtraNode2", "tmsg"}},
                      {{"StartNode", "EndNode", "defdst"},
                       {"StartNode", "ExtraNode", "dynpar1"},
                       {"StartNode", "ExtraNode2", "dynpar1"}});

  Validation::init();
  EXPECT_THROW(Validation::neighbourhoodCheck(0, g), std::runtime_error);
}

TEST(TestTmsgNode, ShouldFailAdressingMetaNode) {
  auto g = makeGraph({{"StartNode", "tmsg"}, {"MetaNode", "meta"}}, {{"StartNode", "MetaNode", "defdst"}});
  Validation::init();
  EXPECT_THROW(Validation::neighbourhoodCheck(0, g), std::runtime_error);
}

/**
 *    cRules.insert(ConstellationRule(n::sTMsg,        e::sDefDst,      cNonMeta,  1, 1  ));
      cRules.insert(ConstellationRule(n::sTMsg,        e::sDynPar0,     cNonMeta,  0, 1  ));
      cRules.insert(ConstellationRule(n::sTMsg,        e::sDynPar1,     cNonMeta,  0, 1  ));
      cRules.insert(ConstellationRule(n::sTMsg,        e::sDyn[DYN_MODE_ADR], cNonMeta,  0, MaxOccurrance::REF  ));
      cRules.insert(ConstellationRule(n::sTMsg,        e::sDyn[DYN_MODE_REF], cNonMeta,  0, MaxOccurrance::REF  ));
      cRules.insert(ConstellationRule(n::sTMsg,        e::sDyn[DYN_MODE_REF2], cNonMeta,  0, MaxOccurrance::REF  ));
      cRules.insert(ConstellationRule(n::sTMsg,        e::sAdr,         cNonMeta,  0, MaxOccurrance::REF  ));
      cRules.insert(ConstellationRule(n::sTMsg,        e::sWrite,       cNonMeta,  0, 1  ));
 */

// TEST(TestTmsgNode, )
