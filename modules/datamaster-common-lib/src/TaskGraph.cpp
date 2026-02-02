#include "TaskGraph.h"

#include <fmt/core.h>
#include <magic_enum/magic_enum.hpp>

#include "DotGraphParsingHelpers.h"

using namespace carpeDM2;
using namespace carpeDM;

namespace
{

[[nodiscard]] std::vector<ConversionError>
VerifyBlockNodeAttributes( const decltype( DotGraph::vertices )::value_type::second_type& vertex )
{
}

[[nodiscard]] std::vector<ConversionError>
VerifyNodeAttributes( const decltype( DotGraph::vertices )::value_type::second_type& vertex )
{
  constexpr std::string_view errorMsg( "Missing required attribute: {} in Vertex: {}" );

  auto collectedErrors = ExpectAttributesToBePresent( vertex, std::vector<std::string>{ "type" } );

  if ( !collectedErrors.empty() )
  {
    return collectedErrors;
  }

  auto typeValue     = vertex.attributes.at( "type" );
  auto potentialType = magic_enum::enum_cast<TaskNodeType>( typeValue, magic_enum::case_insensitive );

  if ( !potentialType.has_value() )
  {
    collectedErrors.push_back(
        ConversionError{ fmt::format( "Invalid type value: {} in Vertex: {}", typeValue, vertex.id ) } );
    return collectedErrors;
  }

  switch ( potentialType.value() )
  {
  case TaskNodeType::Block:
  {
    return VerifyBlockNodeAttributes( vertex );
  }
  }

  return collectedErrors;
}

} // namespace

std::variant<TaskGraph, ConversionError> TaskGraph::parse( const DotGraph& dotgraph )
{
  std::vector<ConversionError> errors;

  for ( const auto& [name, vertex] : dotgraph.vertices )
  {
    auto error = VerifyNodeAttributes( vertex );
    if ( error.size() > 0 )
    {
      std::move( error.begin(), error.end(), std::back_inserter( errors ) );
    }
  }

  auto concatenatedErrors = ConcatenateErrors( errors );
  if ( concatenatedErrors.has_value() )
  {
    return ConversionError{ fmt::format( "Errors found in DotGraph:\n{}", concatenatedErrors->message ) };
  }

  std::vector<TaskNode>                     nodes;
  std::unordered_map<uint32_t, std::string> nodeNames;
  std::unordered_map<uint32_t, size_t>      nodeIndex;
  uint32_t                                  entryNode;

  return TaskGraph(
      dotgraph.properties.name, std::move( nodes ), std::move( nodeNames ), std::move( nodeIndex ), entryNode );
}
