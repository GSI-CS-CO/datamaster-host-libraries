#include <gtest/gtest.h>

#include <bit>
#include <cstdint>
#include <ftm_common.h>

#include "hugeGraph.h"

#include "DotParser.h"
#include "ScheduleGraph.h"

// Helper to parse and convert a DOT graph, returning the ConversionError message if any
static std::variant<carpeDM::ScheduleGraph, carpeDM::ParsingError, carpeDM::ConversionError>
TryConvertDotGraph( const std::string& dot )
{
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( dot );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    return std::get<carpeDM::ParsingError>( graph );
  }
  auto& dotGraph      = std::get<carpeDM::DotGraph>( graph );
  auto  scheduleGraph = carpeDM::ScheduleGraph::fromDotGraph( dotGraph );
  if ( std::holds_alternative<carpeDM::ConversionError>( scheduleGraph ) )
  {
    return std::get<carpeDM::ConversionError>( scheduleGraph );
  }
  return std::get<carpeDM::ScheduleGraph>( scheduleGraph );
}

std::string ConcatenateAttributes( const std::unordered_map<std::string, std::string>& attrs )
{
  std::string attributesConcatenated = "[";

  for ( const auto& [key, value] : attrs )
  {
    attributesConcatenated += key + "=\"" + value + "\" ";
  }

  attributesConcatenated += "]";
  return attributesConcatenated;
}

union NodeFlagsTester
{
  carpeDM::NodeFlags flags;
  uint32_t           raw;
};

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_Type )
{
  NodeFlagsTester tester    = {};
  tester.flags.content.type = NFLG_TYPE_MSK; // Set type to maximum value
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_TYPE_SMSK ) << "NodeFlags type offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_Paint_Lm32 )
{
  NodeFlagsTester tester            = {};
  tester.flags.content.painted_lm32 = NFLG_PAINT_LM32_MSK; // Set painted_host to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_PAINT_LM32_SMSK ) << "NodeFlags painted_host offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_Paint_Host )
{
  NodeFlagsTester tester            = {};
  tester.flags.content.painted_host = NFLG_PAINT_HOST_MSK; // Set painted_host to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_PAINT_HOST_SMSK ) << "NodeFlags painted_host offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_Sync )
{
  NodeFlagsTester tester    = {};
  tester.flags.content.sync = NFLG_ORIGIN_MSK; // Set sync to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_ORIGIN_SMSK ) << "NodeFlags sync offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_BeamProcessEntry )
{
  NodeFlagsTester tester       = {};
  tester.flags.content.bpentry = NFLG_BP_ENTRY_LM32_MSK; // Set bpentry to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_BP_ENTRY_LM32_SMSK ) << "NodeFlags bpentry offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_PatternEntry )
{
  NodeFlagsTester tester        = {};
  tester.flags.content.patentry = NFLG_PAT_ENTRY_LM32_MSK; // Set patentry to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_PAT_ENTRY_LM32_SMSK ) << "NodeFlags patentry offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_BeamProcessExit )
{
  NodeFlagsTester tester      = {};
  tester.flags.content.bpexit = NFLG_BP_EXIT_LM32_MSK; // Set bpexit to true
  memcpy( &tester.raw, &tester.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( tester.raw, NFLG_BP_EXIT_LM32_SMSK ) << "NodeFlags bpexit offset is incorrect";
}

TEST( ScheduleGraphFlags, NodeFlagsOffsetsAreCorrect_PatternExit )
{
  NodeFlagsTester tester       = {};
  tester.flags.content.patexit = NFLG_PAT_EXIT_LM32_MSK; // Set patexit to true
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
  std::string input       = R"graph(
digraph G {
A [type="Block" pattern="default" beamproc="default" cpu="0" flags="0x0" tPeriod="1000"];
B [type="tmsg", cpu="0",  pattern="SA_20240709145503389_DEFAULT", toffs="500000", id="0x112c0ff000000000", fid="1", gid="300", evtno="255", sid="0", bpid="0", reqnobeam="0", vacc="0", par="0x0000000000000000"];A -> B;
}
)graph";
  auto        parseResult = TryConvertDotGraph( input );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ScheduleGraph>( parseResult ) );
  auto& sg = std::get<carpeDM::ScheduleGraph>( parseResult );
  EXPECT_EQ( sg.getName(), "G" );
  const auto potentialNode = sg.getNodeByName( "B" );
  EXPECT_NE( potentialNode, nullptr );
  const auto& node = ( *potentialNode );

  EXPECT_TRUE( std::holds_alternative<carpeDM::TimingMessage>( node ) );
  auto firstTimingMessage = std::get<carpeDM::TimingMessage>( node );
  EXPECT_EQ( firstTimingMessage.cpu, 0 );
  // Test against 102 - the paint bit as it has not been set in the DOT graph as it never
  // ran on the lm32
  memcpy( &firstTimingMessage.flags.raw, &firstTimingMessage.flags, sizeof( carpeDM::NodeFlags ) );
  EXPECT_EQ( firstTimingMessage.flags.raw, 0x00000002 );
  EXPECT_EQ( firstTimingMessage.tOffs, 500000 );
  EXPECT_EQ( firstTimingMessage.pattern, "SA_20240709145503389_DEFAULT" );
  EXPECT_EQ( firstTimingMessage.beamproc, "" );
  EXPECT_EQ( firstTimingMessage.id, 0x112c0ff000000000 );
  EXPECT_EQ( firstTimingMessage.par, 0x0000000000000000 );
  EXPECT_EQ( firstTimingMessage.tef, 0 );
  EXPECT_EQ( firstTimingMessage.res, 0 );
}

class ScheduleGraphBlockValidation : public ::testing::TestWithParam<std::string>
{
};

static std::unordered_map<std::string, std::string> BLOCK_DEFAULT_ATTRIBUTES = {
  { "type", "Block" },
  { "pattern", "default" },
  { "cpu", "0" },
  { "tperiod", "1000" },
};

TEST_P( ScheduleGraphBlockValidation, BlockMissingAttributeCausesError )
{
  const auto& attr = GetParam();

  auto attrs = BLOCK_DEFAULT_ATTRIBUTES;
  attrs.erase( attr ); // Remove the attribute to test

  auto attributesConcatenated = ConcatenateAttributes( attrs );

  const std::string dotStr = "digraph G { A " + attributesConcatenated + " }";

  auto err = TryConvertDotGraph( dotStr );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ConversionError>( err ) );
  EXPECT_NE( std::get<carpeDM::ConversionError>( err ).message.find( attr ), std::string::npos );
}

INSTANTIATE_TEST_SUITE_P( BlockMissingAttributes,
                          ScheduleGraphBlockValidation,
                          ::testing::Values( "type", "pattern", "cpu", "tperiod" ) );

class ScheduleGraphTmsgValidation : public ::testing::TestWithParam<std::string>
{
};

static std::unordered_map<std::string, std::string> TMSG_DEFAULT_ATTRIBUTES = { { "id", "0x112c0ff000000000" },
                                                                                { "type", "tmsg" },
                                                                                { "cpu", "0" },
                                                                                { "pattern",
                                                                                  "SA_20240709145503389_DEFAULT" },
                                                                                { "toffs", "500000" },
                                                                                { "par", "0x0000000000000000" } };

TEST_P( ScheduleGraphTmsgValidation, TmsgMissingAttributeCausesError )
{
  const auto& attr = GetParam();

  auto attrs = TMSG_DEFAULT_ATTRIBUTES;
  attrs.erase( attr ); // Remove the attribute to test

  auto attributesConcatenated = ConcatenateAttributes( attrs );

  const std::string dotStr = "digraph G { A " + attributesConcatenated + " }";

  auto err = TryConvertDotGraph( dotStr );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ConversionError>( err ) );
  EXPECT_NE( std::get<carpeDM::ConversionError>( err ).message.find( attr ), std::string::npos );
}

INSTANTIATE_TEST_SUITE_P( TmsgMissingAttributes,
                          ScheduleGraphTmsgValidation,
                          ::testing::Values( "type", "cpu", "pattern", "toffs", "par", "id" ) );

static std::unordered_map<std::string, std::string> TMSG_SUBID_ATTRIBUTES_FID1 = {
  { "fid", "1" },  { "gid", "300" },     { "evtno", "255" }, { "sid", "0" },
  { "bpid", "0" }, { "reqnobeam", "0" }, { "vacc", "0" }
};

class ScheduleGraphTimingMessageSubIdValidation : public ::testing::TestWithParam<std::string>
{
};

TEST_P( ScheduleGraphTimingMessageSubIdValidation, TmsgWithMissingIdButSubidFormat )
{
  const auto& attr = GetParam();

  auto attrs = TMSG_DEFAULT_ATTRIBUTES;
  attrs.erase( "id" ); // Remove the id attribute to test
  attrs.insert( TMSG_SUBID_ATTRIBUTES_FID1.begin(), TMSG_SUBID_ATTRIBUTES_FID1.end() );
  attrs.erase( attr ); // Remove the attribute to test

  auto attributesConcatenated = ConcatenateAttributes( attrs );

  const std::string dotStr = "digraph G { A " + attributesConcatenated + " }";

  auto err = TryConvertDotGraph( dotStr );

  if ( std::holds_alternative<carpeDM::ConversionError>( err ) )
    EXPECT_NE( std::get<carpeDM::ConversionError>( err ).message.find( attr ), std::string::npos );
  else
    FAIL() << "Expected ConversionError, but got something else";
}

INSTANTIATE_TEST_SUITE_P( TmsgWithMissingIdButSubidFormat,
                          ScheduleGraphTimingMessageSubIdValidation,
                          ::testing::Values( "fid", "gid", "evtno", "sid", "bpid", "reqnobeam", "vacc" ) );

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

class ScheduleGraphCommandValidation : public ::testing::TestWithParam<std::tuple<std::string, std::string>>
{
};

static std::unordered_map<std::string, std::string> COMMAND_DEFAULT_ATTRIBUTES = { { "pattern", "default" },
                                                                                   { "cpu", "0" },
                                                                                   { "toffs", "500000" } };

static auto BUILD_COMMAND_ATTRIBUTES = []( std::string type )
{
  auto attrs    = COMMAND_DEFAULT_ATTRIBUTES;
  attrs["type"] = type;
  return attrs;
};

TEST_P( ScheduleGraphCommandValidation, CommandMissingAttributeCausesError )
{
  const auto& attr  = std::get<1>( GetParam() );
  const auto& type  = std::get<0>( GetParam() );
  auto        attrs = BUILD_COMMAND_ATTRIBUTES( type );
  attrs.erase( attr ); // Remove the attribute to test
  auto              attributesConcatenated = ConcatenateAttributes( attrs );
  const std::string dotStr                 = "digraph G { A " + attributesConcatenated + " }";
  auto              err                    = TryConvertDotGraph( dotStr );
  EXPECT_TRUE( std::holds_alternative<carpeDM::ConversionError>( err ) );
  EXPECT_NE( std::get<carpeDM::ConversionError>( err ).message.find( attr ), std::string::npos );
}

INSTANTIATE_TEST_SUITE_P( CommandMissingAttributes,
                          ScheduleGraphCommandValidation,
                          ::testing::Combine( ::testing::Values( "noop", "flow", "flush", "wait" ),
                                              ::testing::Values( "type", "pattern", "cpu", "toffs" ) ) );
