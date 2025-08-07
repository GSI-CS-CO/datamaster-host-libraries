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

template <typename T>
constexpr T parseValue( const std::string& value )
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
  else
  {
    static_assert( std::is_same_v<T, uint64_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint8_t> ||
                       std::is_same_v<T, bool>,
                   "Unsupported type for parseValue" );
  }
}

template <typename T>
T parseOptionalValue( const std::unordered_map<std::string, std::string>& map, const std::string& key, T defaultValue )
{
  auto found = map.count( key ) != 0;
  if ( !found )
  {
    return defaultValue;
  }
  return parseValue<T>( map.at( key ) );
}

void ParseFlags( Node& attrs, const DotGraphVertex& vertex )
{
  auto bpentry  = parseOptionalValue<bool>( vertex.attributes, "bpentry", false );
  auto bpexit   = parseOptionalValue<bool>( vertex.attributes, "bpexit", false );
  auto patEntry = parseOptionalValue<bool>( vertex.attributes, "patentry", false );
  auto patExit  = parseOptionalValue<bool>( vertex.attributes, "patexit", false );
}

void ParseCommonCommandAttributes( Command& attrs, const DotGraphVertex& vertex )
{
  attrs.tValid  = parseValue<uint64_t>( vertex.attributes.at( "tvalid" ) );
  uint8_t  prio = parseValue<uint8_t>( vertex.attributes.at( "prio" ) );
  uint32_t qty  = parseValue<uint32_t>( vertex.attributes.at( "qty" ) );

  bool vabs = parseOptionalValue<bool>( vertex.attributes, "vabs", false );

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
  attrs.beamproc = vertex.attributes.at( "beamproc" );

  attrs.hash = fnv1a_hash( vertex.id );
  attrs.cpu  = parseValue<decltype( attrs.cpu )>( vertex.attributes.at( "cpu" ) );

  // Whats that beamproc?

  bool bpentry  = parseOptionalValue<bool>( vertex.attributes, "bpentry", false );
  bool bpexit   = parseOptionalValue<bool>( vertex.attributes, "bpexit", false );
  bool patentry = parseOptionalValue<bool>( vertex.attributes, "patentry", false );
  bool patexit  = parseOptionalValue<bool>( vertex.attributes, "patexit", false );

  bool qlo = parseOptionalValue<bool>( vertex.attributes, "qlo", false );
  bool qhi = parseOptionalValue<bool>( vertex.attributes, "qhi", false );
  bool qil = parseOptionalValue<bool>( vertex.attributes, "qil", false );
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

  tmsg.id = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "id" ), nullptr, 16 ) );

  tmsg.par = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "par" ) ) );
  tmsg.tef = static_cast<uint32_t>( std::stoul( vertex.attributes.at( "tef" ) ) );

  if ( vertex.attributes.find( "res" ) != vertex.attributes.end() )
  {
    tmsg.res = static_cast<uint32_t>( std::stoul( vertex.attributes.at( "res" ) ) );
  }
  else
  {
    tmsg.res = 0; // Default value if not present
  }

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
  flush.tOffs  = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "toffs" ) ) );
  flush.tValid = static_cast<uint64_t>( std::stoull( vertex.attributes.at( "tvalid" ) ) );

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
  case carpeDM::VertexType::BLockAlign:
    return ParseGraphNode<Block>( vertex, NODE_TYPE_BLOCK_ALIGN );
  case carpeDM::VertexType::Tmsg:
    return ParseGraphNode<TimingMessage>( vertex, NODE_TYPE_TMSG );
  case carpeDM::VertexType::Noop:
    return ParseGraphNode<NoOp>( vertex, NODE_TYPE_CNOOP );
  case carpeDM::VertexType::Switch:
    return ParseGraphNode<Switch>( vertex, NODE_TYPE_CSWITCH );
  case carpeDM::VertexType::Origin:
    return ParseGraphNode<Origin>( vertex, NODE_TYPE_ORIGIN );
  case carpeDM::VertexType::Flow:
    return ParseGraphNode<Flow>( vertex, NODE_TYPE_CFLOW );
  case carpeDM::VertexType::StartThread:
    return ParseGraphNode<StartThread>( vertex, NODE_TYPE_STARTTHREAD );
  case carpeDM::VertexType::Flush:
    return ParseGraphNode<Flush>( vertex, NODE_TYPE_CFLUSH );
  case carpeDM::VertexType::Wait:
    return ParseGraphNode<Wait>( vertex, NODE_TYPE_CWAIT );
  case carpeDM::VertexType::QInfo:
    return ParseGraphNode<CmdQMeta>( vertex, NODE_TYPE_QUEUE );
  case carpeDM::VertexType::ListDst:
    return ParseGraphNode<DestList>( vertex, NODE_TYPE_ALTDST );
  case carpeDM::VertexType::QBuf:
    return ParseGraphNode<CmdQBuffer>( vertex, NODE_TYPE_QBUF );
  case carpeDM::VertexType::Global:
    return ParseGraphNode<Global>( vertex, NODE_TYPE_GLOBAL );
  default:
    throw std::runtime_error( "Unsupported vertex type: " + vertex.attributes.at( "type" ) );
  }
}

bool IsCommandType( carpeDM::VertexType type )
{

  // Force compiler error for missing cases

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
  case carpeDM::VertexType::BLockAlign:
  case carpeDM::VertexType::Tmsg:
  case carpeDM::VertexType::Switch:
  case carpeDM::VertexType::Origin:
  case carpeDM::VertexType::StartThread:
  case carpeDM::VertexType::QInfo:
  case carpeDM::VertexType::ListDst:
  case carpeDM::VertexType::QBuf:
  case carpeDM::VertexType::Global:
    return false;
  } // explicitely no default case to enforce compiler errors

  return false;
}

bool IsEventType( carpeDM::VertexType type )
{
  // Force compiler error for missing cases

  switch ( type )
  {
  case carpeDM::VertexType::Tmsg:
    [[fallthrough]];
  case carpeDM::VertexType::Switch:
    [[fallthrough]];
  case carpeDM::VertexType::Origin:
    [[fallthrough]];
  case carpeDM::VertexType::StartThread:
    [[fallthrough]];
  case carpeDM::VertexType::Wait:
    return true;
  case carpeDM::VertexType::Block:
  case carpeDM::VertexType::BLockAlign:
  case carpeDM::VertexType::Noop:
  case carpeDM::VertexType::Flow:
  case carpeDM::VertexType::Flush:
  case carpeDM::VertexType::QInfo:
  case carpeDM::VertexType::ListDst:
  case carpeDM::VertexType::QBuf:
  case carpeDM::VertexType::Global:
    return false;
  } // explicitely no default case to enforce compiler errors

  return false;
}

std::optional<ConversionError>
VerifyNodeAttributes( const decltype( DotGraph::vertices )::value_type::second_type& vertex )
{
  constexpr std::string_view errorMsg( "Missing required attribute: {} in Vertex: {}" );

  if ( vertex.attributes.find( "pattern" ) == vertex.attributes.end() )
  {
    return ConversionError{ fmt::format( errorMsg, "pattern", vertex.id ) };
  }

  if ( vertex.attributes.find( "beamproc" ) == vertex.attributes.end() )
  {
    return ConversionError{ fmt::format( errorMsg, "beamproc", vertex.id ) };
  }

  if ( vertex.attributes.find( "cpu" ) == vertex.attributes.end() )
  {
    return ConversionError{ fmt::format( errorMsg, "cpu", vertex.id ) };
  }

  if ( vertex.attributes.find( "flags" ) == vertex.attributes.end() )
  {
    return ConversionError{ fmt::format( errorMsg, "flags", vertex.id ) };
  }

  if ( vertex.attributes.count( "type" ) == 0 )
  {
    return ConversionError{ fmt::format( errorMsg, "type", vertex.id ) };
  }

  // Check if type is valid
  auto potentialType =
      magic_enum::enum_cast<carpeDM::VertexType>( vertex.attributes.at( "type" ), magic_enum::case_insensitive );
  if ( !potentialType.has_value() )
  {
    return ConversionError{ "Invalid type value" };
  }

  const auto type = potentialType.value();

  if ( IsCommandType( type ) )
  {
    if ( vertex.attributes.find( "tvalid" ) == vertex.attributes.end() )
    {
      return ConversionError{ fmt::format( errorMsg, "tvalid", vertex.id ) };
    }
    else if ( vertex.attributes.count( "qty" ) == 0 )
    {
      return ConversionError{ fmt::format( errorMsg, "qty", vertex.id ) };
    }
    else if ( vertex.attributes.count( "prio" ) == 0 )
    {
      return ConversionError{ fmt::format( errorMsg, "prio", vertex.id ) };
    }
    else if ( vertex.attributes.count( "vabs" ) == 0 )
    {
      return ConversionError{ fmt::format( errorMsg, "vabs", vertex.id ) };
    }
    // chp is not required for all command types, so we don't check it here
  }

  if ( IsEventType( type ) )
  {
    if ( vertex.attributes.find( "toffs" ) == vertex.attributes.end() )
    {
      return ConversionError{ fmt::format( errorMsg, "toffs", vertex.id ) };
    }
  }

  if ( type == carpeDM::VertexType::Block || type == carpeDM::VertexType::BLockAlign )
  {
    if ( vertex.attributes.find( "tperiod" ) == vertex.attributes.end() )
    {
      return ConversionError{ fmt::format( errorMsg, "tperiod", vertex.id ) };
    }
    // rest of the attributes are optional for Block and BLockAlign
  }

  if ( type == carpeDM::VertexType::Origin || type == carpeDM::VertexType::StartThread )
  {
    if ( vertex.attributes.find( "thread" ) == vertex.attributes.end() )
    {
      return ConversionError{ fmt::format( errorMsg, "thread", vertex.id ) };
    }
  }

  if ( type == carpeDM::VertexType::Flush )
  {
    if ( vertex.attributes.find( "mode" ) == vertex.attributes.end() ||
         vertex.attributes.find( "frmil" ) == vertex.attributes.end() ||
         vertex.attributes.find( "toil" ) == vertex.attributes.end() ||
         vertex.attributes.find( "frmhi" ) == vertex.attributes.end() ||
         vertex.attributes.find( "tohi" ) == vertex.attributes.end() ||
         vertex.attributes.find( "frmlo" ) == vertex.attributes.end() ||
         vertex.attributes.find( "tolo" ) == vertex.attributes.end() )
    {
      return ConversionError{ fmt::format( errorMsg, "flush attributes", vertex.id ) };
    }
  }

  if ( type == carpeDM::VertexType::Global )
  {
    if ( vertex.attributes.find( "section" ) == vertex.attributes.end() )
    {
      return ConversionError{ fmt::format( errorMsg, "section", vertex.id ) };
    }
  }

  if ( carpeDM::VertexType::Tmsg == type )
  {
    if ( vertex.attributes.find( "id" ) == vertex.attributes.end() ||
         vertex.attributes.find( "par" ) == vertex.attributes.end() ||
         vertex.attributes.find( "tef" ) == vertex.attributes.end() )
    {
      return ConversionError{ fmt::format( errorMsg, "id/par/tef", vertex.id ) };
    }
    // res is optional, so we don't check it here
  }

  // Additional checks for specific types can be added here if needed

  return std::nullopt; // All checks passed
}

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
    if ( error.has_value() )
    {
      errors.push_back( *error );
      continue; // Skip this vertex if it has errors
    };
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