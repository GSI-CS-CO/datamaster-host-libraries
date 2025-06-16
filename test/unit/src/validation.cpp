#include <gtest/gtest.h>

#include <string>
#include <tuple>

#include "block.h"
#include "dotstr.h"
#include "validation.h"

using namespace testing;

// Demonstrate some basic assertions.
TEST(ValidationTest, TestInit) { Validation::init(); }

class AllCombinationsParametrizedFixture
    : public testing::Test,
      public WithParamInterface<std::tuple<std::string, std::string, std::string>> {};

INSTANTIATE_TEST_SUITE_P(
    AllCombinations, AllCombinationsParametrizedFixture,
    Combine(Values("tmsg", "noop", "flow", "switch", "origin", "flush", "wait", "start", "stop", "abort", "startthread",
                   "lock", "unlock", "asyncclear", "block", "blockalign", "qinfo", "listdst", "qbuf", "meta", "global"),
            Values("tmsg", "noop", "flow", "switch", "origin", "flush", "wait", "start", "stop", "abort", "startthread",
                   "lock", "unlock", "asyncclear", "block", "blockalign", "qinfo", "listdst", "qbuf", "meta", "global"),
            Values("listdst", "defdst", "altdst", "baddefdst", "target", "switchdst", "origindst", "flowdst",
                   "flushovr", "dynid", "dynpar0", "dynpar1", "dyntef", "dynres", "meta", "reference", "reference2",
                   "address", "write")));

TEST_P(AllCombinationsParametrizedFixture, IsCorrectCombinationReturnsFalse) {
  auto [startNodeType, endNodeType, edgeType] = GetParam();

  const std::string pattern = "pattern";
  const std::string beamproc = "beamproc";
  uint8_t cpu = 0;
  uint32_t flags = 0;

  Graph g;
  myVertex startNode;
  startNode.name = "StartNode";
  startNode.type = startNodeType;
  startNode.hash = std::hash<std::string>()(startNode.name + startNodeType + edgeType + endNodeType);
  startNode.np = boost::make_shared<BlockFixed>(startNode.name, pattern, beamproc, startNode.hash, cpu, flags);

  myVertex endNode;
  endNode.name = "EndNode";
  endNode.type = endNodeType;
  endNode.hash = std::hash<std::string>()(endNode.name + endNodeType + edgeType + startNodeType);
  endNode.np = boost::make_shared<BlockFixed>(endNode.name, pattern, beamproc, endNode.hash, cpu, flags);

  myEdge edge;
  edge.type = edgeType;

  std::size_t startNodeIndex = g.m_vertices.size();
  g.m_vertices.push_back(startNode);
  std::size_t endNodeIndex = g.m_vertices.size();
  g.m_vertices.push_back(endNode);
  g.added_vertex(startNodeIndex);
  g.added_vertex(endNodeIndex);

  add_edge(startNodeIndex, endNodeIndex, edge, g);

  Validation::init();

  auto it = Validation::cRules.get<Validation::Constellation>().find(boost::make_tuple(startNodeType, edgeType));

  const bool foundRule = (it != Validation::cRules.get<Validation::Constellation>().end());

  const bool targetNodeValid = foundRule && it->children.count(endNodeType) > 0;

  if (!foundRule || !targetNodeValid) {
    EXPECT_THROW(Validation::neighbourhoodCheck(startNodeIndex, g), std::runtime_error);
  } else {
    Validation::neighbourhoodCheck(startNodeIndex, g);
  }
}
