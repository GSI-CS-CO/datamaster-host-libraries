#include <gtest/gtest.h>

#include "DotParser.h"
#include "TaskGraph.h"

namespace
{
// Helper to parse and convert a DOT graph, returning the ConversionError message if any
std::variant<carpeDM2::TaskGraph, carpeDM::ParsingError, carpeDM::ConversionError>
TryLoadTaskGraph( const std::string& dot )
{
  carpeDM::DotGraphParser parser;
  auto                    graph = parser.parseGraph( dot );
  if ( std::holds_alternative<carpeDM::ParsingError>( graph ) )
  {
    return std::get<carpeDM::ParsingError>( graph );
  }
  auto& dotGraph      = std::get<carpeDM::DotGraph>( graph );
  auto  scheduleGraph = carpeDM2::TaskGraph::parse( dotGraph );
  if ( std::holds_alternative<carpeDM::ConversionError>( scheduleGraph ) )
  {
    return std::get<carpeDM::ConversionError>( scheduleGraph );
  }
  return std::move( std::get<carpeDM2::TaskGraph>( scheduleGraph ) );
}
} // namespace

TEST( TaskGraphFromDot, ValidGraphCreatesTaskGraph )
{
  std::string input = R"graph(
    digraph SendSingleTimingMessage {
      A [ type="TimingMessage", id="1", par="0", tef="0", res="0" ];
      B [ type="Block", tPeriod="1000", align="true" ];
      A -> B
    }
  )graph";

  auto taskGraphOrError = TryLoadTaskGraph( input );
  EXPECT_TRUE( std::holds_alternative<carpeDM2::TaskGraph>( taskGraphOrError ) );
  const auto& taskGraph = std::get<carpeDM2::TaskGraph>( taskGraphOrError );
  EXPECT_EQ( taskGraph.getName(), "SendSingleTimingMessage" );

  EXPECT_EQ( taskGraph.getNodes().size(), 2 );

  const auto timingMessageNode = taskGraph.getNodeByName( "A" );
  EXPECT_NE( timingMessageNode, nullptr );
  const auto& timingMessage = ( *timingMessageNode );
  EXPECT_TRUE( std::holds_alternative<carpeDM2::TimingMessageNode>( timingMessage ) );
  auto& timingMessageAsTM = std::get<carpeDM2::TimingMessageNode>( timingMessage );
  EXPECT_EQ( timingMessageAsTM.nodeName, "A" );
  EXPECT_EQ( timingMessageAsTM.id, carpeDM2::Field<uint64_t>( std::in_place_type<uint64_t>, 1 ) );

  EXPECT_EQ( timingMessageAsTM.par, carpeDM2::Field<uint64_t>( uint64_t{ 0 } ) );
  EXPECT_EQ( timingMessageAsTM.tef, carpeDM2::Field<uint32_t>( uint32_t{ 0 } ) );
  EXPECT_EQ( timingMessageAsTM.res, carpeDM2::Field<uint32_t>( uint32_t{ 0 } ) );

  const auto blockNode = taskGraph.getNodeByName( "B" );
  EXPECT_NE( blockNode, nullptr );
  const auto& block = ( *blockNode );
  EXPECT_TRUE( std::holds_alternative<carpeDM2::BlockNode>( block ) );
  auto& blockAsBlock = std::get<carpeDM2::BlockNode>( block );
  EXPECT_EQ( blockAsBlock.nodeName, "B" );
  EXPECT_EQ( blockAsBlock.tPeriod, carpeDM2::Field<uint64_t>( 1000u ) );
  EXPECT_EQ( blockAsBlock.align, true );
}
