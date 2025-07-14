#include <gtest/gtest.h>

#include "graphHelper.h"

#include <string>
#include <tuple>

#include "block.h"
#include "dotstr.h"
#include "validation.h"
#include <boost/graph/adjacency_list.hpp>

using namespace testing;

/**
 * @brief Represents an expectation that a node or edge type should occur at least a certain number of times.
 * The alternativeNodeType and alternativeEdgeType are used to specify what should be used
 * if occurances is 0 and we want to avoid the error that raises when no children are found.
 *
 * So alternativeNodeType and alternativeEdgeType must specify a valid combination that is allowed to occur once
 */
struct AtLeast
{
  int         occurances;
  std::string alternativeNodeType;
  std::string alternativeEdgeType;
};

struct Expectation
{
  enum ExpectationType
  {
    NOT_MORE_THAN,
    AT_LEAST
  };

  ExpectationType type;

  int         occurances;
  std::string alternativeNodeType;
  std::string alternativeEdgeType;
};

const auto DEFAULT_EXPECTATION = Expectation{ .type = Expectation::NOT_MORE_THAN, .occurances = 0 };

struct TestDefinition
{
  std::string from;
  std::string to;
  std::string over;
  Expectation expectation;
  TestDefinition() = delete;
  TestDefinition( std::string fromNode, std::string toNode, std::string edgeType, Expectation expectation )
      : from( std::move( fromNode ) )
      , to( std::move( toNode ) )
      , over( std::move( edgeType ) )
      , expectation( std::move( expectation ) )
  {
  }
};

class DatamasterRuleValidation
    : public testing::Test
    , public WithParamInterface<TestDefinition>
{
};

namespace
{

const std::vector<const char*> NODES = { "tmsg",   "noop",       "flow",  "switch",     "origin",      "flush",
                                         "wait",   "start",      "stop",  "abort",      "startthread", "lock",
                                         "unlock", "asyncclear", "block", "blockalign", "qinfo",       "listdst",
                                         "qbuf",   "meta",       "global" }; // namespace

const std::vector<const char*> EDGES = { "listdst",   "defdst",     "altdst",  "baddefdst", "target",
                                         "switchdst", "origindst",  "flowdst", "flushovr",  "dynid",
                                         "dynpar0",   "dynpar1",    "dyntef",  "dynres",    "meta",
                                         "reference", "reference2", "address", "write" };

const std::vector<const char*> NON_META_NODES = { "tmsg",  "noop", "flow",  "origin",     "startthread", "switch",
                                                  "flush", "wait", "block", "blockalign", "global" };

const std::vector<const char*> BLOCK_NODES = { "block", "blockalign" };

constexpr int32_t MAX_EDGE_COUNT_ALT_DST = 110;
constexpr int32_t MAX_EDGE_COUNT_REF_DST = 3;

const LeafNodeDescription MINIMAL_WORKING_GRAPH_LEAF = { .type = "tmsg", .edgeType = "defdst", .numLeaves = 1 };

std::string PrintExpectation( Expectation expectation )
{
  switch ( expectation.type )
  {
  case Expectation::NOT_MORE_THAN:
    return "NotMoreThan_" + std::to_string( expectation.occurances );
  case Expectation::AT_LEAST:
    return "AtLeast_" + std::to_string( expectation.occurances ) + "_" + expectation.alternativeNodeType + "_" +
           expectation.alternativeEdgeType;
  }

  return ""; // Should never happen, but the fucking compiler is dumb as shit and does not understand that
}

std::string getTestName( const testing::TestParamInfo<TestDefinition>& info )
{
  return info.param.from + "_" + info.param.to + "_" + info.param.over + "_" +
         PrintExpectation( info.param.expectation );
}

/**
 * @brief A builder class to create validation tests for the Datamaster.
 *
 * This class allows you to specify the parameters for the validation tests,
 * such as the source node type, destination node type, edge type, and the expected number
 * of occurrences. It builds a test case for each combination of these parameters.
 *
 * Usage:
 * 1. Create an instance of `ValidationTestBuilder` with a vector to hold the tests.
 * 2. Use the `From`, `To`, and `Over` methods to specify the source node type, destination node type, and edge type
 * respectively.
 * 3. Use `ExpectNotMoreThan` or `ExpectAtLeast` to set the expected number of occurrences.
 * 4. Call `Build` to create the test cases and add them to the vector
 */
class ValidationTestBuilder
{
public:
  struct Rule
  {
    std::vector<std::string> m_from;
    std::vector<std::string> m_to;
    std::vector<std::string> m_over;
    std::vector<Expectation> m_expectations;
  };

public:
  ValidationTestBuilder( std::vector<TestDefinition>& tests )
      : m_tests( tests )
      , m_currentRule()
  {
  }

  ValidationTestBuilder& From( const char* from )
  {
    m_currentRule.m_from.emplace_back( from );
    return *this;
  }

  ValidationTestBuilder& From( const std::vector<const char*>& from )
  {
    m_currentRule.m_from.insert( m_currentRule.m_from.end(), from.begin(), from.end() );
    return *this;
  }

  ValidationTestBuilder& To( const char* to )
  {
    m_currentRule.m_to.emplace_back( to );
    return *this;
  }

  ValidationTestBuilder& To( const std::vector<const char*>& to )
  {
    m_currentRule.m_to.insert( m_currentRule.m_to.end(), to.begin(), to.end() );
    return *this;
  }

  ValidationTestBuilder& Over( const char* over )
  {
    m_currentRule.m_over.emplace_back( std::move( over ) );
    return *this;
  }

  ValidationTestBuilder& Over( const std::vector<const char*>& over )
  {
    m_currentRule.m_over.insert( m_currentRule.m_over.end(), over.begin(), over.end() );
    return *this;
  }

  ValidationTestBuilder& Expect( Expectation expectation )
  {
    m_currentRule.m_expectations.emplace_back( std::move( expectation ) );
    return *this;
  }

  ValidationTestBuilder& ExpectNotMoreThan( int occurances )
  {
    m_currentRule.m_expectations.emplace_back(
        Expectation{ .type = Expectation::NOT_MORE_THAN, .occurances = occurances } );
    return *this;
  }

  ValidationTestBuilder&
  ExpectAtLeast( int occurances, std::string alternativeNodeType, std::string alternativeEdgeType )
  {
    assert( occurances > 0 && "Occurance for 0 should be tested via ExpectNotMoreThan 0." );
    m_currentRule.m_expectations.emplace_back( Expectation{ .type                = Expectation::AT_LEAST,
                                                            .occurances          = occurances,
                                                            .alternativeNodeType = std::move( alternativeNodeType ),
                                                            .alternativeEdgeType = std::move( alternativeEdgeType ) } );
    return *this;
  }

  void Submit()
  {
    if ( m_currentRule.m_from.empty() || m_currentRule.m_to.empty() || m_currentRule.m_over.empty() )
    {
      throw std::runtime_error( "ValidationTestBuilder: From, To and Over must be set before building the test." );
    }

    m_rules.emplace_back( std::move( m_currentRule ) );
  }

  std::vector<Expectation> findExpectations( const std::string& from, const std::string& to, const std::string& over )
  {
    std::vector<Expectation> expectations;
    for ( auto& rule : m_rules )
    {
      if ( std::find( rule.m_from.begin(), rule.m_from.end(), from ) != rule.m_from.end() &&
           std::find( rule.m_to.begin(), rule.m_to.end(), to ) != rule.m_to.end() &&
           std::find( rule.m_over.begin(), rule.m_over.end(), over ) != rule.m_over.end() )
      {
        std::copy( rule.m_expectations.begin(), rule.m_expectations.end(), std::back_inserter( expectations ) );
      }
    }

    return expectations;
  }

  void Build()
  {
    m_tests.reserve( NODES.size() * NODES.size() * EDGES.size() * 3 );
    for ( auto& fromNode : NODES )
    {
      for ( auto& toNode : NODES )
      {
        for ( auto& edgeType : EDGES )
        {
          auto expectations = findExpectations( fromNode, toNode, edgeType );
          if ( expectations.empty() )
          {
            expectations.push_back( DEFAULT_EXPECTATION );
          }

          for ( auto& expectation : expectations )
          {
            m_tests.emplace_back( TestDefinition{ fromNode, toNode, edgeType, expectation } );
          }
        }
      }
    }
  }

private:
  std::vector<TestDefinition>& m_tests;
  Rule                         m_currentRule;
  std::vector<Rule>            m_rules;
};

std::vector<DatamasterRuleValidation::ParamType> GenerateValidationTests()
{
  std::vector<DatamasterRuleValidation::ParamType> tests;
  ValidationTestBuilder                            builder( tests );
  builder.From( "tmsg" )
      .To( NON_META_NODES )
      .Over( { "defdst", "dynpar0", "dynpar1", "write" } )
      .ExpectNotMoreThan( 1 )
      .Submit();

  builder.From( "tmsg" )
      .To( NON_META_NODES )
      .Over( std::vector<const char*>{ "defdst" } )
      .ExpectAtLeast( 1, "tmsg", "defdst" )
      .Submit();

  builder.From( "tmsg" )
      .To( NON_META_NODES )
      .Over( { "address", "reference", "reference2" } )
      .ExpectNotMoreThan( 3 )
      .Submit();

  builder.From( "noop" ).To( NON_META_NODES ).Over( { "defdst" } ).ExpectNotMoreThan( 1 ).Submit();
  builder.From( "noop" ).To( BLOCK_NODES ).Over( "target" ).ExpectNotMoreThan( 1 ).Submit();

  builder.From( "flow" ).To( NON_META_NODES ).Over( "defdst" ).ExpectNotMoreThan( 1 ).Submit();
  builder.From( "flow" ).To( BLOCK_NODES ).Over( "target" ).ExpectNotMoreThan( 1 ).Submit();
  builder.From( "flow" ).To( NON_META_NODES ).Over( "flowdst" ).ExpectNotMoreThan( 1 ).Submit();

  builder.From( "switch" )
      .To( NON_META_NODES )
      .Over( std::vector<const char*>{ "defdst", "switchdst" } )
      .ExpectNotMoreThan( 1 )
      .Submit();
  builder.From( "switch" ).To( BLOCK_NODES ).Over( "target" ).ExpectNotMoreThan( 1 ).Submit();

  builder.From( "origin" )
      .To( NON_META_NODES )
      .Over( std::vector<const char*>{ "defdst", "origindst" } )
      .ExpectNotMoreThan( 1 )
      .Submit();
  builder.From( "origin" ).To( NON_META_NODES ).Over( "origindst" ).ExpectAtLeast( 1, "tmsg", "defdst" ).Submit();

  builder.From( "startthread" ).To( NON_META_NODES ).Over( "defdst" ).ExpectNotMoreThan( 1 ).Submit();

  builder.From( "flush" )
      .To( NON_META_NODES )
      .Over( std::vector<const char*>{ "defdst", "flushovr" } )
      .ExpectNotMoreThan( 1 )
      .Submit();
  builder.From( "flush" ).To( NON_META_NODES ).Over( "defdst" ).ExpectAtLeast( 1, "tmsg", "defdst" ).Submit();
  builder.From( "flush" ).To( BLOCK_NODES ).Over( "target" ).ExpectNotMoreThan( 1 ).Submit();

  builder.From( "wait" ).To( NON_META_NODES ).Over( "defdst" ).ExpectNotMoreThan( 1 ).Submit();
  builder.From( "wait" ).To( BLOCK_NODES ).Over( "target" ).ExpectNotMoreThan( 1 ).Submit();

  builder.From( BLOCK_NODES ).To( NON_META_NODES ).Over( "defdst" ).ExpectNotMoreThan( 1 ).Submit();
  builder.From( BLOCK_NODES )
      .To( NON_META_NODES )
      .Over( "altdst" )
      .ExpectNotMoreThan( MAX_EDGE_COUNT_ALT_DST )
      .Submit();

  builder.From( BLOCK_NODES )
      .To( NON_META_NODES )
      .Over( { "address", "reference", "reference2" } )
      .ExpectNotMoreThan( MAX_EDGE_COUNT_REF_DST )
      .Submit();

  builder.From( BLOCK_NODES ).To( "qinfo" ).Over( { "prioil", "priohi", "priolo" } ).ExpectNotMoreThan( 1 ).Submit();

  builder.From( "qinfo" ).To( "qbuf" ).Over( std::vector<const char*>{ "meta" } ).ExpectNotMoreThan( 2 ).Submit();
  builder.From( "qinfo" )
      .To( "qbuf" )
      .Over( std::vector<const char*>{ "meta" } )
      .ExpectAtLeast( 2, "qbuf", "meta" )
      .Submit();

  builder.From( "listdst" ).To( BLOCK_NODES ).Over( "defdst" ).ExpectNotMoreThan( 1 ).Submit();
  builder.From( "listdst" ).To( BLOCK_NODES ).Over( "defdst" ).ExpectAtLeast( 1, "", "" ).Submit();

  builder.Build();
  return tests;
}

} // namespace

void TestNotMoreThan( const TestDefinition& testDefinition )
{
  {
    auto g = makeStarGraph( testDefinition.from.c_str(),
                            { LeafNodeDescription{ .type      = testDefinition.to,
                                                   .edgeType  = testDefinition.over,
                                                   .numLeaves = testDefinition.expectation.occurances + 1 } } );
    EXPECT_THROW( Validation::neighbourhoodCheck( 0, g ), std::runtime_error );
  }

  if ( testDefinition.expectation.occurances > 0 )
  {
    auto g = makeStarGraph( testDefinition.from.c_str(),
                            { LeafNodeDescription{ .type      = testDefinition.to,
                                                   .edgeType  = testDefinition.over,
                                                   .numLeaves = testDefinition.expectation.occurances } } );
    EXPECT_NO_THROW( Validation::neighbourhoodCheck( 0, g ) );
  }
}

void TestAtLeast( const TestDefinition& testDefinition )
{
  auto invalid_occurances = testDefinition.expectation.occurances - 1;

  // If we expect at least one occurence, we have to test this with at least one other node as child, because
  // childless nodes may always raise an error.
  const bool testNodeNotAvailableShouldThrow = invalid_occurances == 0 &&
                                               !testDefinition.expectation.alternativeNodeType.empty() &&
                                               !testDefinition.expectation.alternativeEdgeType.empty();

  if ( testNodeNotAvailableShouldThrow )
  {
    auto g = makeStarGraph( testDefinition.from.c_str(),
                            { LeafNodeDescription{ .type      = testDefinition.expectation.alternativeNodeType,
                                                   .edgeType  = testDefinition.expectation.alternativeEdgeType,
                                                   .numLeaves = 1 } } );
    EXPECT_THROW( Validation::neighbourhoodCheck( 0, g ), std::runtime_error );
  }

  {
    auto g = makeStarGraph( testDefinition.from.c_str(),
                            { LeafNodeDescription{ .type      = testDefinition.to,
                                                   .edgeType  = testDefinition.over,
                                                   .numLeaves = testDefinition.expectation.occurances - 1 } } );
    EXPECT_THROW( Validation::neighbourhoodCheck( 0, g ), std::runtime_error );
  }

  {
    auto g = makeStarGraph( testDefinition.from.c_str(),
                            { LeafNodeDescription{ .type      = testDefinition.to,
                                                   .edgeType  = testDefinition.over,
                                                   .numLeaves = testDefinition.expectation.occurances } } );
    EXPECT_NO_THROW( Validation::neighbourhoodCheck( 0, g ) );
  }
}

INSTANTIATE_TEST_SUITE_P( Validation, DatamasterRuleValidation, ValuesIn( GenerateValidationTests() ), getTestName );

TEST_P( DatamasterRuleValidation, ValidCombinations )
{
  auto& testDefinition = GetParam();
  Validation::init();

  switch ( testDefinition.expectation.type )
  {
  case Expectation::NOT_MORE_THAN:
    TestNotMoreThan( testDefinition );
    break;
  case Expectation::AT_LEAST:
    TestAtLeast( testDefinition );
    break;
  }
}

TEST( Validation, InvalidNodeNameShouldThrow )
{
  // This test checks that a node with an invalid name throws an error.
  auto g = makeGraph( "invalidname", "tmsg", "defdst" );
  Validation::init();
  EXPECT_THROW( Validation::neighbourhoodCheck( 0, g ), std::runtime_error );
}

TEST( Validation, InvalidEdgeTypeShouldThrow )
{
  // This test checks that a graph with an invalid edge type throws an error.
  auto g = makeGraph( "tmsg", "tmsg", "invalidedge" );
  Validation::init();
  EXPECT_THROW( Validation::neighbourhoodCheck( 0, g ), std::runtime_error );
}
