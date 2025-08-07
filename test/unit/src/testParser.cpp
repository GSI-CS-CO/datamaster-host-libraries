#include "gtest/gtest.h"

#include "DotParser.h"

#include <hugeGraph.h>

TEST( ScheduleGraphParsing, EmptyStrictGraph )
{
  std::string             input = "strict digraph G { }";
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );
  EXPECT_TRUE( dotGraph.vertices.empty() );
  EXPECT_TRUE( dotGraph.edges.empty() );
  EXPECT_EQ( dotGraph.properties.name, "G" );
  EXPECT_TRUE( dotGraph.properties.isStrict );
}

TEST( ScheduleGraphParsing, EmptyGraph )
{
  std::string             input = "digraph G { }";
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );
  EXPECT_TRUE( dotGraph.vertices.empty() );
  EXPECT_TRUE( dotGraph.edges.empty() );
  EXPECT_EQ( dotGraph.properties.name, "G" );
  EXPECT_FALSE( dotGraph.properties.isStrict );
}

TEST( ScheduleGraphParsing, GraphWithSingleNode )
{
  std::string             input = "digraph G { A }";
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );
  EXPECT_EQ( dotGraph.vertices.size(), 1 );
  EXPECT_EQ( dotGraph.vertices.count( "A" ), 1 );
  EXPECT_TRUE( dotGraph.edges.empty() );
  EXPECT_EQ( dotGraph.properties.name, "G" );
  EXPECT_FALSE( dotGraph.properties.isStrict );
}

TEST( ScheduleGraphParsing, GraphWithTwoNodes )
{
  std::string             input = "digraph G { A B }";
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );
  EXPECT_EQ( dotGraph.vertices.size(), 2 );
  EXPECT_EQ( dotGraph.vertices.count( "A" ), 1 );
  EXPECT_EQ( dotGraph.vertices.count( "B" ), 1 );
  EXPECT_TRUE( dotGraph.edges.empty() );
  EXPECT_EQ( dotGraph.properties.name, "G" );
  EXPECT_FALSE( dotGraph.properties.isStrict );
}

TEST( ScheduleGraphParsing, GraphWithEdge )
{
  std::string             input = "digraph G { A -> B }";
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );
  EXPECT_EQ( dotGraph.vertices.size(), 2 );
  EXPECT_EQ( dotGraph.vertices.count( "A" ), 1 );
  EXPECT_EQ( dotGraph.vertices.count( "B" ), 1 );
  EXPECT_EQ( dotGraph.edges.size(), 1 );
  EXPECT_EQ( dotGraph.edges[0].source, "A" );
  EXPECT_EQ( dotGraph.edges[0].target, "B" );
  EXPECT_EQ( dotGraph.properties.name, "G" );
  EXPECT_FALSE( dotGraph.properties.isStrict );
}

TEST( ScheduleGraphParsing, GraphWithNodeAttributes )
{
  std::string             input = "digraph G { A [label=\"Node A\"]; B [label=\"Node B\"] }";
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );
  EXPECT_EQ( dotGraph.vertices.size(), 2 );
  EXPECT_EQ( dotGraph.vertices.count( "A" ), 1 );
  EXPECT_EQ( dotGraph.vertices.count( "B" ), 1 );
  EXPECT_TRUE( dotGraph.edges.empty() );
  EXPECT_EQ( dotGraph.properties.name, "G" );
  EXPECT_FALSE( dotGraph.properties.isStrict );

  // Check attributes
  EXPECT_EQ( dotGraph.vertices["A"].attributes["label"], "Node A" );
  EXPECT_EQ( dotGraph.vertices["B"].attributes["label"], "Node B" );
}

TEST( ScheduleGraphParsing, GraphAttributesStoredCaseInsensitive )
{
  std::string             input = "digraph G { A [LABEL=\"Node A\"]; B [Label=\"Node B\"] }";
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );

  // Check that attributes are stored in lowercase
  EXPECT_EQ( dotGraph.vertices["A"].attributes["label"], "Node A" );
  EXPECT_EQ( dotGraph.vertices["B"].attributes["label"], "Node B" );
}

TEST( ScheduleGraphParsing, HugeGraph )
{

  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( HUGE_GRAPH );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );
  EXPECT_EQ( dotGraph.vertices.size(), 500 );
  EXPECT_EQ( dotGraph.edges.size(), 555 ); // No edges in this example

  // Random checks
  EXPECT_EQ( dotGraph.vertices["SIS18_FAST_RF_SYNC_COMMISSIONING_022"].attributes["pattern"],
             "SIS18_FAST_RF_SYNC_COMMISSIONING" );
  EXPECT_EQ( dotGraph.vertices["ESR_SL_ISO_BREAKPOINT_August24.C1.3_MANIP_WAIT_EXIT"].attributes["flags"],
             "0x00100007" );
  EXPECT_EQ( dotGraph.vertices["ESR_SL_ISO_BREAKPOINT_August24.C1.3_MANIP_WAIT_EXIT"].attributes["type"], "block" );
  EXPECT_EQ( dotGraph.vertices["ESR_SL_ISO_BREAKPOINT_August24.C1.3_MANIP_WAIT_EXIT"].attributes["tperiod"],
             "196000000" );

  EXPECT_EQ( dotGraph.properties.name, "G" );
  EXPECT_FALSE( dotGraph.properties.isStrict );
}

TEST( ScheduleGraphParsing, GraphWithLateNodeDefinition )
{
  constexpr std::string_view input = R"graph(
digraph G {
 A -> B;
 B [label="B" shape=box color=red];
})graph";

  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );
  EXPECT_EQ( dotGraph.vertices.size(), 2 );
  EXPECT_EQ( dotGraph.edges.size(), 1 );

  // Random checks
  EXPECT_EQ( dotGraph.vertices["B"].attributes["label"], "B" );
  EXPECT_EQ( dotGraph.vertices["B"].attributes["shape"], "box" );
  EXPECT_EQ( dotGraph.vertices["B"].attributes["color"], "red" );
}

TEST( ScheduleGraphParsing, NodeDefinitionExtendedViaMultipleDeclarations )
{
  constexpr std::string_view input = R"graph(
digraph G {
 A [label="A" shape=box];
  A [color=blue];
})graph";

  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph = std::get<carpeDM::DotGraph>( graph );
  EXPECT_EQ( dotGraph.vertices.size(), 1 );
  EXPECT_EQ( dotGraph.edges.size(), 0 );

  // Random checks
  EXPECT_EQ( dotGraph.vertices["A"].attributes["label"], "A" );
  EXPECT_EQ( dotGraph.vertices["A"].attributes["shape"], "box" );
  EXPECT_EQ( dotGraph.vertices["A"].attributes["color"], "blue" );
}

// Test: Unexpected end of input while parsing ID (empty graph)
TEST( ScheduleGraphParsing, UnexpectedEndOfInput_Empty )
{
  constexpr std::string_view input = "";

  carpeDM::DotGraphParser parser;
  auto                    result = parser.parseGraph( input );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ParsingError>( result ) );
}

// Test: Unexpected end of input while parsing attribute list (missing closing bracket)
TEST( ScheduleGraphParsing, UnexpectedEndOfInput_AttrList )
{
  constexpr std::string_view input = "digraph G { A [label=\"foo\" ";

  carpeDM::DotGraphParser parser;
  auto                    result = parser.parseGraph( input );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ParsingError>( result ) );
}

// Test: Unexpected end of input while parsing ID (missing node id)
TEST( ScheduleGraphParsing, UnexpectedEndOfInput_MissingNodeId )
{
  constexpr std::string_view input = "digraph G { -> B }";

  carpeDM::DotGraphParser parser;
  auto                    result = parser.parseGraph( input );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ParsingError>( result ) );
}

// Test: Unexpected end of input while parsing ID (unterminated quoted id)
TEST( ScheduleGraphParsing, UnexpectedEndOfInput_UnterminatedString )
{
  constexpr std::string_view input = "digraph G { \"foo }";

  carpeDM::DotGraphParser parser;
  auto                    result = parser.parseGraph( input );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ParsingError>( result ) );
}

// Test: Unexpected end of input while parsing attribute value (missing value)
TEST( ScheduleGraphParsing, UnexpectedEndOfInput_MissingAttrValue )
{
  constexpr std::string_view input = "digraph G { A [label= ";

  carpeDM::DotGraphParser parser;
  auto                    result = parser.parseGraph( input );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ParsingError>( result ) );
}

// Test: Unexpected end of input while parsing edge (missing target)
TEST( ScheduleGraphParsing, UnexpectedEndOfInput_MissingEdgeTarget )
{
  constexpr std::string_view input = "digraph G { A -> ";

  carpeDM::DotGraphParser parser;
  auto                    result = parser.parseGraph( input );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ParsingError>( result ) );
}