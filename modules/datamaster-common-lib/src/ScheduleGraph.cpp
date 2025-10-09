#include "ScheduleGraph.h"

#include <cstring>
#include <string>
#include <variant>

#include "DotParser.h"
#include "ScheduleGraphTypes.h"
#include <magic_enum/magic_enum.hpp>

#include <fmt/core.h>

#include <ftm_common.h>

#include <numeric> // For std::accumulate
#include <stdexcept>

using namespace carpeDM;

namespace
{
const auto REQUIRED_NODE_ATTRIBUTES = std::vector<std::string>{ "pattern", "cpu", "type" };

template <typename T>
constexpr T ParseValue( const std::string& value )
{
  if constexpr ( std::is_same_v<T, uint64_t> )
  {
    return static_cast<uint64_t>( std::stoull( value ) );
  }
  else if constexpr ( std::is_same_v<T, uint32_t> )
  {
    return static_cast<uint32_t>( std::stoul( value ) );
  }
  else if constexpr ( std::is_same_v<T, uint8_t> )
  {
    return static_cast<uint8_t>( std::stoi( value ) );
  }
  else if constexpr ( std::is_same_v<T, bool> )
  {
    if ( value == "true" || value == "1" )
    {
      return true;
    }
    else if ( value == "false" || value == "0" )
    {
      return false;
    }
    return false;
  }
  else if constexpr ( std::is_same_v<T, std::string> )
  {
    return value;
  }
  else
  {
    static_assert( std::is_same_v<T, uint64_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint8_t> ||
                       std::is_same_v<T, bool>,
                   "Unsupported type for ParseValue" );
  }
}

template <typename T>
T ParseOptionalValue( const std::unordered_map<std::string, std::string>& map, const std::string& key, T defaultValue )
{
  auto found = map.count( key ) != 0;
  if ( !found )
  {
    return defaultValue;
  }
  return ParseValue<T>( map.at( key ) );
}

void ParseFlags( Node& attrs, const DotGraphVertex& vertex )
{
  auto bpentry  = ParseOptionalValue<bool>( vertex.attributes, "bpentry", false );
  auto bpexit   = ParseOptionalValue<bool>( vertex.attributes, "bpexit", false );
  auto patEntry = ParseOptionalValue<bool>( vertex.attributes, "patentry", false );
  auto patExit  = ParseOptionalValue<bool>( vertex.attributes, "patexit", false );

  attrs.flags.content.bpentry  = bpentry ? 1 : 0;
  attrs.flags.content.bpexit   = bpexit ? 1 : 0;
  attrs.flags.content.patentry = patEntry ? 1 : 0;
  attrs.flags.content.patexit  = patExit ? 1 : 0;
}

void ParseCommonCommandAttributes( Command& attrs, const DotGraphVertex& vertex )
{
  attrs.tValid  = ParseOptionalValue<uint64_t>( vertex.attributes, "tvalid", 0 );
  uint8_t  prio = ParseOptionalValue<uint8_t>( vertex.attributes, "prio", 0 );
  uint32_t qty  = ParseOptionalValue<uint32_t>( vertex.attributes, "qty", 0 );
  bool     vabs = ParseOptionalValue<bool>( vertex.attributes, "vabs", false );

  // What is chp?
  // bool     chp  = static_cast<bool>( std::stoul( vertex.attributes.at( "chp" ) ) );
  // attrs.act.chp     = chp ? 1 : 0;

  attrs.act.prio = prio & ACT_PRIO_MSK;
  attrs.act.qty  = qty & ACT_QTY_MSK;
  attrs.act.vabs = vabs ? 1 : 0;
}

void ParseCommonEventAttributes( Event& attrs, const DotGraphVertex& vertex )
{
  attrs.tOffs = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "toffs" ) ) );
}

void ParseCommonNodeAttributes( Node& attrs, const DotGraphVertex& vertex, uint32_t typeFlag )
{
  attrs.name     = vertex.id;
  attrs.pattern  = vertex.attributes.at( "pattern" );
  attrs.beamproc = ParseOptionalValue<std::string>( vertex.attributes, "beamproc", "" );

  attrs.hash = fnv1a_hash( vertex.id );
  attrs.cpu  = ParseValue<decltype( attrs.cpu )>( vertex.attributes.at( "cpu" ) );

  attrs.flags.raw = 0;
  memcpy( &attrs.flags.content, &attrs.flags.raw, sizeof( NodeFlags ) );
  ParseFlags( attrs, vertex );

  attrs.flags.content.type = typeFlag;

  // Default destination is INVALID_NODE_HASH
  attrs.defaultDestination = INVALID_NODE_HASH;
}

template <typename NodeType>
ScheduleGraphNode ParseGraphNode( const DotGraphVertex& vertex, uint32_t typeFlag ) = delete;

template <>
ScheduleGraphNode ParseGraphNode<Block>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  Block block;
  ParseCommonNodeAttributes( block, vertex, typeFlag );
  block.tPeriod = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "tperiod" ) ) );

  return block;
}

template <>
ScheduleGraphNode ParseGraphNode<TimingMessage>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  TimingMessage tmsg;
  ParseCommonNodeAttributes( tmsg, vertex, typeFlag );
  ParseCommonEventAttributes( tmsg, vertex );
  // e.g id="0x112c0ff000000000"

  tmsg.id  = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "id" ), nullptr, 16 ) );
  tmsg.par = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "par" ) ) );

  tmsg.tef = ParseOptionalValue<uint32_t>( vertex.attributes, "tef", 0 );
  tmsg.res = ParseOptionalValue<uint32_t>( vertex.attributes, "res", 0 );

  return tmsg;
}

template <typename T>
T ParseCommandNode( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  T cmd;
  ParseCommonNodeAttributes( cmd, vertex, typeFlag );
  ParseCommonEventAttributes( cmd, vertex );
  ParseCommonCommandAttributes( cmd, vertex );

  return cmd;
}

template <>
ScheduleGraphNode ParseGraphNode<NoOp>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  NoOp noop;
  ParseCommonNodeAttributes( noop, vertex, typeFlag );
  ParseCommonCommandAttributes( noop, vertex );

  return noop;
}

template <>
ScheduleGraphNode ParseGraphNode<Switch>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  Switch node;
  ParseCommonNodeAttributes( node, vertex, typeFlag );
  ParseCommonEventAttributes( node, vertex );
  return node;
}

template <>
ScheduleGraphNode ParseGraphNode<Origin>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  Origin origin;
  ParseCommonNodeAttributes( origin, vertex, typeFlag );
  origin.tOffs  = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "toffs" ) ) );
  origin.thread = static_cast<uint32_t>( std::stoul( vertex.attributes.at( "thread" ) ) );

  return origin;
}

template <>
ScheduleGraphNode ParseGraphNode<StartThread>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  StartThread startThread;
  ParseCommonNodeAttributes( startThread, vertex, typeFlag );
  startThread.tOffs     = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "toffs" ) ) );
  startThread.startOffs = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "startoffs" ) ) );
  startThread.thread    = static_cast<uint32_t>( std::stoul( vertex.attributes.at( "thread" ) ) );

  return startThread;
}

template <>
ScheduleGraphNode ParseGraphNode<Command>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  return ParseCommandNode<Command>( vertex, typeFlag );
}

template <>
ScheduleGraphNode ParseGraphNode<Flow>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  return ParseCommandNode<Flow>( vertex, typeFlag );
}

template <>
ScheduleGraphNode ParseGraphNode<Flush>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  Flush flush;
  ParseCommonNodeAttributes( flush, vertex, typeFlag );
  ParseCommonCommandAttributes( flush, vertex );
  flush.tOffs = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "toffs" ) ) );

  return flush;
}

template <>
ScheduleGraphNode ParseGraphNode<Wait>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  Wait wait;
  ParseCommonNodeAttributes( wait, vertex, typeFlag );
  ParseCommonCommandAttributes( wait, vertex );
  ParseCommonEventAttributes( wait, vertex );

  return wait;
}

template <>
ScheduleGraphNode ParseGraphNode<CmdQMeta>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  CmdQMeta meta;
  ParseCommonNodeAttributes( meta, vertex, typeFlag );
  // No additional attributes for CmdQMeta

  return meta;
}

template <>
ScheduleGraphNode ParseGraphNode<CmdQBuffer>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  CmdQBuffer buffer;
  ParseCommonNodeAttributes( buffer, vertex, typeFlag );
  // No additional attributes for CmdQBuffer

  return buffer;
}

template <>
ScheduleGraphNode ParseGraphNode<DestList>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  DestList listDst;
  ParseCommonNodeAttributes( listDst, vertex, typeFlag );
  // No additional attributes for DestList

  return listDst;
}

template <>
ScheduleGraphNode ParseGraphNode<Global>( const DotGraphVertex& vertex, uint32_t typeFlag )
{
  Global global;
  ParseCommonNodeAttributes( global, vertex, typeFlag );
  global.section = vertex.attributes.at( "section" );

  return global;
}

ScheduleGraphNode GenericParseGraphNode( const DotGraphVertex& vertex )
{
  auto potentialType =
      magic_enum::enum_cast<carpeDM::VertexType>( vertex.attributes.at( "type" ), magic_enum::case_insensitive );
  if ( !potentialType.has_value() )
  {
    throw std::runtime_error( "Invalid vertex type: " + vertex.attributes.at( "type" ) );
  }

  switch ( potentialType.value() )
  {
  case carpeDM::VertexType::Block:
    return ParseGraphNode<Block>( vertex, NODE_TYPE_BLOCK_FIXED );
  case carpeDM::VertexType::BlockAlign:
    return ParseGraphNode<Block>( vertex, NODE_TYPE_BLOCK_ALIGN );
  case carpeDM::VertexType::Tmsg:
    return ParseGraphNode<TimingMessage>( vertex, NODE_TYPE_TMSG );
  case carpeDM::VertexType::Noop:
    return ParseGraphNode<NoOp>( vertex, NODE_TYPE_CNOOP );
  case carpeDM::VertexType::Origin:
    return ParseGraphNode<Origin>( vertex, NODE_TYPE_ORIGIN );
  case carpeDM::VertexType::Flow:
    return ParseGraphNode<Flow>( vertex, NODE_TYPE_STARTTHREAD );
  case carpeDM::VertexType::Flush:
    return ParseGraphNode<Flush>( vertex, NODE_TYPE_CFLUSH );
  case carpeDM::VertexType::Wait:
    return ParseGraphNode<Wait>( vertex, NODE_TYPE_CWAIT );
  case carpeDM::VertexType::StartThread:
    return ParseGraphNode<StartThread>( vertex, NODE_TYPE_STARTTHREAD );
  default:
    throw std::runtime_error( "Unsupported vertex type: " + vertex.attributes.at( "type" ) );
  }
}

bool IsCommandType( carpeDM::VertexType type )
{

  // Force compiler error for missing cases

#pragma gcc diagnostic push
#pragma gcc diagnostic ignored "-Wswitch"
#pragma gcc diagnostic ignored "-Wswitch-enum"

  switch ( type )
  {
  case carpeDM::VertexType::Noop:
    return std::is_base_of<Command, NoOp>::value;
  case carpeDM::VertexType::Flow:
    return std::is_base_of<Command, Flow>::value;
  case carpeDM::VertexType::Flush:
    return std::is_base_of<Command, Flush>::value;
  case carpeDM::VertexType::Wait:
    return std::is_base_of<Command, Wait>::value;
  case carpeDM::VertexType::StartThread:
    return std::is_base_of<Command, StartThread>::value;
  case carpeDM::VertexType::Block:
    return std::is_base_of<Command, Block>::value;
  case carpeDM::VertexType::BlockAlign:
    return std::is_base_of<Command, Block>::value;
  case carpeDM::VertexType::Tmsg:
    return std::is_base_of<Command, TimingMessage>::value;
  case carpeDM::VertexType::Origin:
    return std::is_base_of<Command, Origin>::value;
  } // explicitely no default case to enforce compiler errors

#pragma gcc diagnostic pop
  return false;
}

bool IsEventType( carpeDM::VertexType type )
{
  if ( IsCommandType( type ) )
  {
    return true;
  }

  switch ( type )
  {
  case carpeDM::VertexType::Noop:
    return std::is_base_of<Event, NoOp>::value;
  case carpeDM::VertexType::Flow:
    return std::is_base_of<Event, Flow>::value;
  case carpeDM::VertexType::Flush:
    return std::is_base_of<Event, Flush>::value;
  case carpeDM::VertexType::Wait:
    return std::is_base_of<Event, Wait>::value;
  case carpeDM::VertexType::StartThread:
    return std::is_base_of<Event, StartThread>::value;
  case carpeDM::VertexType::Block:
    return std::is_base_of<Event, Block>::value;
  case carpeDM::VertexType::BlockAlign:
    return std::is_base_of<Event, Block>::value;
  case carpeDM::VertexType::Tmsg:
    return std::is_base_of<Event, TimingMessage>::value;
  case carpeDM::VertexType::Origin:
    return std::is_base_of<Event, Origin>::value;
  } // explicitely no default case to enforce compiler errors

  return false;
}

std::optional<ConversionError>
ExpectAttributeToBePresent( const decltype( DotGraph::vertices )::value_type::second_type& vertex,
                            const std::string&                                             attributeName )
{
  if ( vertex.attributes.find( attributeName ) == vertex.attributes.end() )
  {
    return ConversionError{ fmt::format( "Missing required attribute: {} in Vertex: {}", attributeName, vertex.id ) };
  }
  return std::nullopt; // Attribute is present
}

std::optional<ConversionError> ExpectAttributeToBePresent( const decltype( DotGraph::edges )::value_type& edge,
                                                           const std::string& attributeName )
{
  if ( edge.attributes.find( attributeName ) == edge.attributes.end() )
  {
    return ConversionError{ fmt::format(
        "Missing required attribute: {} in Edge: {}", attributeName, edge.source + " -> " + edge.target ) };
  }
  return std::nullopt; // Attribute is present
}

template <class T>
std::vector<ConversionError> ExpectAttributesToBePresent( const T& vertex, const std::vector<std::string>& attributes )
{
  std::vector<ConversionError> errors;
  for ( const auto& attr : attributes )
  {
    auto error = ExpectAttributeToBePresent( vertex, attr );
    if ( error.has_value() )
    {
      errors.push_back( *error );
    }
  }
  return errors;
}

std::optional<ConversionError> ConcatenateErrors( const std::vector<ConversionError>& errors )
{
  if ( errors.empty() )
  {
    return std::nullopt; // No errors to concatenate
  }

  const auto concatenatedMessage = std::accumulate( errors.begin(),
                                                    errors.end(),
                                                    std::string{},
                                                    []( const std::string& acc, const ConversionError& error )
                                                    {
                                                      return acc + ( acc.empty() ? "" : "\n" ) + error.message;
                                                    } );

  return ConversionError{ concatenatedMessage };
}

[[nodiscard]] auto VerifyCommandAttributes( const decltype( DotGraph::vertices )::value_type::second_type& vertex )
{
  return ExpectAttributesToBePresent( vertex, {} );
}

[[nodiscard]] auto VerifyEventAttributes( const decltype( DotGraph::vertices )::value_type::second_type& vertex )
{
  return ExpectAttributesToBePresent( vertex, { "toffs" } );
}

[[nodiscard]] auto
VerifyTimingMessageAttributes( const decltype( DotGraph::vertices )::value_type::second_type& vertex )
{
  auto errors = ExpectAttributesToBePresent( vertex, { "par" } );

  auto idPresentError = ExpectAttributeToBePresent( vertex, "id" );
  if ( idPresentError.has_value() )
  {
    auto subIdPresentError =
        ExpectAttributesToBePresent( vertex, { "sfid", "gid", "evtno", "sid", "bpid", "reqnobeam", "vacc" } );
    if ( subIdPresentError.size() > 0 )
    {
      errors.push_back(
          ConversionError{ fmt::format( "Missing required id or sub-id attributes in tmsg Vertex: {}", vertex.id ) } );
      std::move( subIdPresentError.begin(), subIdPresentError.end(), std::back_inserter( errors ) );
    }
  }

  return errors;
}

/**
 * Verify that the mandatory attributes of a node in the schedule graph exist.
 *
 * @param vertex The vertex to verify.
 */
[[nodiscard]] std::vector<ConversionError>
VerifyNodeAttributes( const decltype( DotGraph::vertices )::value_type::second_type& vertex )
{
  constexpr std::string_view errorMsg( "Missing required attribute: {} in Vertex: {}" );

  auto collectedErrors = ExpectAttributesToBePresent( vertex, std::vector<std::string>( REQUIRED_NODE_ATTRIBUTES ) );

  if ( !collectedErrors.empty() )
  {
    return collectedErrors;
  }

  auto typeValue = vertex.attributes.at( "type" );

  // Check if type is valid
  auto potentialType = magic_enum::enum_cast<carpeDM::VertexType>( typeValue, magic_enum::case_insensitive );
  if ( !potentialType.has_value() )
  {
    collectedErrors.push_back(
        ConversionError{ fmt::format( "Invalid type value: {} in Vertex: {}", typeValue, vertex.id ) } );
    return collectedErrors;
  }

  const auto type = potentialType.value();

  if ( IsCommandType( type ) )
  {
    auto errors = VerifyCommandAttributes( vertex );
    std::move( errors.begin(), errors.end(), std::back_inserter( collectedErrors ) );
  }

  if ( IsEventType( type ) )
  {
    auto errors = VerifyEventAttributes( vertex );
    std::move( errors.begin(), errors.end(), std::back_inserter( collectedErrors ) );
  }

  if ( type == carpeDM::VertexType::Block || type == carpeDM::VertexType::BlockAlign )
  {
    auto blockValidationErrors = ExpectAttributesToBePresent( vertex, { "tperiod" } );
    std::move( blockValidationErrors.begin(), blockValidationErrors.end(), std::back_inserter( collectedErrors ) );
  }
  else if ( type == carpeDM::VertexType::Origin )
  {
    auto originValidationErrors = ExpectAttributesToBePresent( vertex, { "thread" } );
    std::move( originValidationErrors.begin(), originValidationErrors.end(), std::back_inserter( collectedErrors ) );
  }
  else if ( type == carpeDM::VertexType::StartThread )
  {
    auto startThreadValidationErrors = ExpectAttributesToBePresent( vertex, { "thread", "startoffs" } );
    std::move(
        startThreadValidationErrors.begin(), startThreadValidationErrors.end(), std::back_inserter( collectedErrors ) );
  }
  else if ( carpeDM::VertexType::Tmsg == type )
  {
    auto verificationErrors = VerifyTimingMessageAttributes( vertex );
    std::move( verificationErrors.begin(), verificationErrors.end(), std::back_inserter( collectedErrors ) );
  }

  return collectedErrors; // All checks passed
}

/**
 * Verify that the mandatory attributes of an edge in the schedule graph exist.
 */
[[nodiscard]] std::vector<ConversionError> VerifyEdgeAttributes( const decltype( DotGraph::edges )::value_type& edge )
{
  return ExpectAttributesToBePresent( edge, { "type" } );
}

// Default implementation for setting an edge on a node type. By default we throw an error.
template <EdgeType EType>
std::optional<ConversionError>
SetEdge( ScheduleGraphNode& srcNode, uint32_t dstHash, const decltype( DotGraphEdge::attributes )& attributes )
{
  return std::visit(
      [&]( auto&& node ) -> ConversionError
      {
        return ConversionError{ fmt::format(
            "Edge type '{}' not supported for source node type '{}'", attributes.at( "type" ), node.name ) };
      },
      srcNode );
}

template <>
std::optional<ConversionError> SetEdge<EdgeType::defdst>( ScheduleGraphNode&                          srcNode,
                                                          uint32_t                                    dstHash,
                                                          const decltype( DotGraphEdge::attributes )& attributes )
{
  std::visit(
      [&]( auto&& node )
      {
        node.defaultDestination = dstHash;
      },
      srcNode );
  return std::nullopt;
}

[[nodiscard]] std::optional<ConversionError> SetEdge( ScheduleGraphNode&                          srcNode,
                                                      ScheduleGraphNode&                          dstNode,
                                                      const decltype( DotGraphEdge::attributes )& attributes )
{
  auto potentialType =
      magic_enum::enum_cast<carpeDM::EdgeType>( attributes.at( "type" ), magic_enum::case_insensitive );
  if ( !potentialType.has_value() )
  {
    return ConversionError{ fmt::format( "Invalid edge type: {} in Edge: {}",
                                         attributes.at( "type" ),
                                         GetNodeName( srcNode ) + " -> " + GetNodeName( dstNode ) ) };
  }
  switch ( potentialType.value() )
  {
  case EdgeType::defdst:
    return SetEdge<EdgeType::defdst>( srcNode, GetNodeHash( dstNode ), attributes );
  }

  return ConversionError{ fmt::format( "Unsupported edge type: {} in Edge: {}",
                                       attributes.at( "type" ),
                                       GetNodeName( srcNode ) + " -> " + GetNodeName( dstNode ) ) };
}

} // namespace

ScheduleGraph::ScheduleGraph( std::string                                 name,
                              std::vector<ScheduleGraphNode>&&            nodes,
                              std::unordered_map<uint32_t, std::string>&& nodeNames,
                              std::unordered_map<uint32_t, size_t>&&      nodeIndex )
    : m_name( std::move( name ) )
    , m_nodes( std::move( nodes ) )
    , m_nodeNames( std::move( nodeNames ) )
    , m_nodeIndex( std::move( nodeIndex ) )
{
}

std::variant<ScheduleGraph, ConversionError> ScheduleGraph::fromDotGraph( const DotGraph& dotGraph )
{
  std::vector<ScheduleGraphNode> nodes;
  std::vector<ConversionError>   errors;

  for ( const auto& [name, vertex] : dotGraph.vertices )
  {
    auto error = VerifyNodeAttributes( vertex );
    if ( error.size() > 0 )
    {
      std::move( error.begin(), error.end(), std::back_inserter( errors ) );
    }
  }

  for ( const auto& edge : dotGraph.edges )
  {
    auto error = VerifyEdgeAttributes( edge );
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

  size_t                                 index = 0;
  decltype( ScheduleGraph::m_nodeNames ) nodeNames;
  decltype( ScheduleGraph::m_nodeIndex ) nodeIndex;

  for ( const auto& [name, vertex] : dotGraph.vertices )
  {
    auto hash       = fnv1a_hash( vertex.id );
    nodeNames[hash] = vertex.id;
    nodeIndex[hash] = index++;
  }

  std::transform( dotGraph.vertices.begin(),
                  dotGraph.vertices.end(),
                  std::back_inserter( nodes ),
                  []( const decltype( DotGraph::vertices )::value_type& element )
                  {
                    return GenericParseGraphNode( element.second );
                  } );

  for ( const auto& edge : dotGraph.edges )
  {
    auto srcHash = fnv1a_hash( edge.source );
    auto dstHash = fnv1a_hash( edge.target );

    if ( nodeIndex.find( srcHash ) == nodeIndex.end() )
    {
      return ConversionError{ fmt::format( "Edge source node '{}' not found in vertices", edge.source ) };
    }
    if ( nodeIndex.find( dstHash ) == nodeIndex.end() )
    {
      return ConversionError{ fmt::format( "Edge target node '{}' not found in vertices", edge.target ) };
    }

    auto& srcNode        = nodes.at( nodeIndex.at( srcHash ) );
    auto& dstNode        = nodes.at( nodeIndex.at( dstHash ) );
    auto  potentialError = SetEdge( srcNode, dstNode, edge.attributes );
    if ( potentialError.has_value() )
    {
      errors.emplace_back( std::move( *potentialError ) );
    }
  }

  ScheduleGraph graph( dotGraph.properties.name, std::move( nodes ), std::move( nodeNames ), std::move( nodeIndex ) );
  return graph;
}