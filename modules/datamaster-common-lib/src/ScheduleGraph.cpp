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
    [[fallthrough]];
  case carpeDM::VertexType::Flow:
    [[fallthrough]];
  case carpeDM::VertexType::Flush:
    [[fallthrough]];
  case carpeDM::VertexType::Wait:
    return true;
  case carpeDM::VertexType::Block:
  case carpeDM::VertexType::BlockAlign:
  case carpeDM::VertexType::Tmsg:
  case carpeDM::VertexType::Origin:
    return false;
  } // explicitely no default case to enforce compiler errors

#pragma gcc diagnostic pop
  return false;
}

bool IsEventType( carpeDM::VertexType type )
{
  // Force compiler error for missing cases

  if ( IsCommandType( type ) )
  {
    return true;
  }

  switch ( type )
  {
  case carpeDM::VertexType::Tmsg:
    [[fallthrough]];
  case carpeDM::VertexType::Origin:
    [[fallthrough]];
  case carpeDM::VertexType::Wait:
    return true;
  case carpeDM::VertexType::Block:
  case carpeDM::VertexType::BlockAlign:
  case carpeDM::VertexType::Noop:
  case carpeDM::VertexType::Flow:
  case carpeDM::VertexType::Flush:
    return false;
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

std::vector<ConversionError>
ExpectAttributesToBePresent( const decltype( DotGraph::vertices )::value_type::second_type& vertex,
                             const std::vector<std::string>&                                attributes )
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

std::vector<ConversionError>
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
    if ( vertex.attributes.find( "tperiod" ) == vertex.attributes.end() )
    {
      return { ConversionError{ fmt::format( errorMsg, "tperiod", vertex.id ) } };
    }
    // rest of the attributes are optional for Block and BLockAlign
  }

  if ( type == carpeDM::VertexType::Origin )
  {
    if ( vertex.attributes.find( "thread" ) == vertex.attributes.end() )
    {
      return { ConversionError{ fmt::format( errorMsg, "thread", vertex.id ) } };
    }
  }

  if ( carpeDM::VertexType::Tmsg == type )
  {
    auto verificationErrors = VerifyTimingMessageAttributes( vertex );
    std::move( verificationErrors.begin(), verificationErrors.end(), std::back_inserter( collectedErrors ) );
  }

  // Additional checks for specific types can be added here if needed

  return collectedErrors; // All checks passed
}

} // namespace

ScheduleGraph::ScheduleGraph( std::string name )
    : m_name( std::move( name ) )
{
}

void ScheduleGraph::setNodes( std::vector<ScheduleGraphNode> nodes )
{
  m_nodes = std::move( nodes );
  m_nodeIndex.clear();
  for ( size_t i = 0; i < m_nodes.size(); ++i )
  {
    auto hash = std::visit(
        []( const auto& node )
        {
          return fnv1a_hash( node.name );
        },
        m_nodes[i] );
    m_nodeIndex[hash] = i;
  }
}

std::variant<ScheduleGraph, ConversionError> ScheduleGraph::fromDotGraph( const DotGraph& dotGraph )
{
  ScheduleGraph graph( dotGraph.properties.name );

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

  if ( errors.size() > 0 )
  {
    auto combinedErrors = std::accumulate( errors.begin(),
                                           errors.end(),
                                           std::string{},
                                           []( const std::string& acc, const ConversionError& err )
                                           {
                                             return acc.empty() ? err.message : acc + ",\n" + err.message;
                                           } );

    return ConversionError{ fmt::format( "Errors found in DotGraph: {}", combinedErrors ) };
  }

  std::transform( dotGraph.vertices.begin(),
                  dotGraph.vertices.end(),
                  std::back_inserter( nodes ),
                  []( const decltype( DotGraph::vertices )::value_type& element )
                  {
                    return GenericParseGraphNode( element.second );
                  } );
  graph.setNodes( std::move( nodes ) );
  return graph;
}