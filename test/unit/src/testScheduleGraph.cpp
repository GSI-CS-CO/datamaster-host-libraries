#include <gtest/gtest.h>

#include <bit>
#include <cstdint>
#include <ftm_common.h>

#include "hugeGraph.h"

#include "DotParser.h"
#include "ScheduleGraph.h"

union NodeFlagsTester
{
  carpeDM::NodeFlags flags;
  uint32_t           raw;
};

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_Type )
{
  NodeFlagsTester tester = {};
  tester.flags.type      = NFLG_TYPE_MSK; // Set type to maximum value
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_TYPE_SMSK ) << "NodeFlags type offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_Paint_Lm32 )
{
  NodeFlagsTester tester    = {};
  tester.flags.painted_lm32 = NFLG_PAINT_LM32_MSK; // Set painted_host to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_PAINT_LM32_SMSK ) << "NodeFlags painted_host offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_Paint_Host )
{
  NodeFlagsTester tester    = {};
  tester.flags.painted_host = NFLG_PAINT_HOST_MSK; // Set painted_host to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_PAINT_HOST_SMSK ) << "NodeFlags painted_host offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_Sync )
{
  NodeFlagsTester tester = {};
  tester.flags.sync      = NFLG_ORIGIN_MSK; // Set sync to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_ORIGIN_SMSK ) << "NodeFlags sync offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_BeamProcessEntry )
{
  NodeFlagsTester tester = {};
  tester.flags.bpentry   = NFLG_BP_ENTRY_LM32_MSK; // Set bpentry to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_BP_ENTRY_LM32_SMSK ) << "NodeFlags bpentry offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_PatternEntry )
{
  NodeFlagsTester tester = {};
  tester.flags.patentry  = NFLG_PAT_ENTRY_LM32_MSK; // Set patentry to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_PAT_ENTRY_LM32_SMSK ) << "NodeFlags patentry offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_BeamProcessExit )
{
  NodeFlagsTester tester = {};
  tester.flags.bpexit    = NFLG_BP_EXIT_LM32_MSK; // Set bpexit to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_BP_EXIT_LM32_SMSK ) << "NodeFlags bpexit offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_PatternExit )
{
  NodeFlagsTester tester = {};
  tester.flags.patexit   = NFLG_PAT_EXIT_LM32_MSK; // Set patexit to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_PAT_EXIT_LM32_SMSK ) << "NodeFlags patexit offset is incorrect";
}

TEST( ScheduleGraphFromDot, InvalidGraphCausesError )
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

  auto scheduleGraph = carpeDM::ScheduleGraph::fromDotGraph( dotGraph );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ConversionError>( scheduleGraph ) );
}

TEST( ScheduleGraphFromDot, ValidGraphCreatesScheduleGraph )
{
  std::string             input = R"graph(
digraph G {
A [type="Block" pattern="default" beamproc="default" cpu="0" flags="0x0" tPeriod="1000"];
B [type="Event" pattern="default" beamproc="default" cpu="0" flags="0x0" tPeriod="1000"];
A -> B;
}
)graph";
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( input );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph      = std::get<carpeDM::DotGraph>( graph );
  auto  scheduleGraph = carpeDM::ScheduleGraph::fromDotGraph( dotGraph );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ScheduleGraph>( scheduleGraph ) );
}

TEST( ScheduleGraphFromDot, HugeGraphParsedWithoutErrors )
{
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( HUGE_GRAPH );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    FAIL() << std::get<carpeDM::ParsingError>( graph ).message;
  }

  EXPECT_TRUE( std::holds_alternative<carpeDM::DotGraph>( graph ) );
  auto& dotGraph      = std::get<carpeDM::DotGraph>( graph );
  auto  scheduleGraph = carpeDM::ScheduleGraph::fromDotGraph( dotGraph );
  if ( std::holds_alternative<carpeDM::ConversionError>( scheduleGraph ) )
  {
    FAIL() << "Expected no error, but received:\n" << std::get<carpeDM::ConversionError>( scheduleGraph ).message;
  }
  EXPECT_TRUE( std::holds_alternative<carpeDM::ScheduleGraph>( scheduleGraph ) );
  auto& sg = std::get<carpeDM::ScheduleGraph>( scheduleGraph );
  EXPECT_EQ( sg.getName(), "G" );
  const auto potentialNode = sg.getNodeByName( "SA_20240709145503389_DEFAULT_000" );
  EXPECT_NE( potentialNode, nullptr );
  const auto& node = ( *potentialNode );

  EXPECT_TRUE( std::holds_alternative<carpeDM::TimingMessage>( node ) );
  auto firstTimingMessage = std::get<carpeDM::TimingMessage>( node );
  EXPECT_EQ( firstTimingMessage.name, "SA_20240709145503389_DEFAULT_000" );
  EXPECT_EQ( firstTimingMessage.cpu, 0 );
  // EXPECT_EQ( firstTimingMessage.flags, 0x00000102 );
  EXPECT_EQ( firstTimingMessage.tOffs, 500000 );
  EXPECT_EQ( firstTimingMessage.pattern, "SA_20240709145503389_DEFAULT" );
  EXPECT_EQ( firstTimingMessage.beamproc, "undefined" );
  EXPECT_EQ( firstTimingMessage.id, 0x112c0ff000000000 );
  EXPECT_EQ( firstTimingMessage.par, 0x0000000000000000 );
  EXPECT_EQ( firstTimingMessage.tef, 0 );
  EXPECT_EQ( firstTimingMessage.res, 0 );
}