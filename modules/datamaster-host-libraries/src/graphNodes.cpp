#include "graphNodes.h"
#include "block.h"
#include "event.h"
#include "global.h"
#include "idformat.h"
#include "meta.h"

#include <cstdio>
#include <graph.h>
#include <stdint.h>

namespace
{

uint64_t GetSubIdByName( const myVertex& vertex, const std::string& propertyName )
{
  std::string id = "";
  if ( propertyName == "fid" )
    id = vertex.id_fid;
  else if ( propertyName == "gid" )
    id = vertex.id_gid;
  else if ( propertyName == "evtno" )
    id = vertex.id_evtno;
  else if ( propertyName == "sid" )
    id = vertex.id_sid;
  else if ( propertyName == "bpid" )
    id = vertex.id_bpid;
  else if ( propertyName == "beamin" )
    id = vertex.id_bin;
  else if ( propertyName == "bpcstart" )
    id = vertex.id_bpcstart;
  else if ( propertyName == "reqnobeam" )
    id = vertex.id_reqnob;
  else if ( propertyName == "vacc" )
    id = vertex.id_vacc;
  else if ( propertyName == "eventIdAttributes" )
    id = vertex.id_evtidatt;

  if ( id.empty() )
  {
    // If the property is not found, we can throw an exception or return a default value.
    // Here we throw an exception to indicate that the property was not found.
    throw std::runtime_error( "Property '" + propertyName + "' not found in vertex '" + vertex.name + "'" );
  }

  return s2u<uint64_t>( id );
}

void SetSubIdByName( const std::string& propertyName, uint64_t value, myVertex& vertex )
{
  if ( propertyName == "fid" )
    vertex.id_fid = std::to_string( value );
  else if ( propertyName == "gid" )
    vertex.id_gid = std::to_string( value );
  else if ( propertyName == "evtno" )
    vertex.id_evtno = std::to_string( value );
  else if ( propertyName == "sid" )
    vertex.id_sid = std::to_string( value );
  else if ( propertyName == "bpid" )
    vertex.id_bpid = std::to_string( value );
  else if ( propertyName == "beamin" )
    vertex.id_bin = std::to_string( value );
  else if ( propertyName == "bpcstart" )
    vertex.id_bpcstart = std::to_string( value );
  else if ( propertyName == "reqnobeam" )
    vertex.id_reqnob = std::to_string( value );
  else if ( propertyName == "vacc" )
    vertex.id_vacc = std::to_string( value );
  else if ( propertyName == "eventIdAttributes" )
    vertex.id_evtidatt = std::to_string( value );
  else
    throw std::runtime_error( "Unknown sub-ID property: '" + propertyName + "'" );
}

} // namespace

void SyncIdSubIds( myVertex& vertex )
{
  if ( vertex.id.empty() || vertex.id == DotStr::Misc::sUndefined64 )
  {
    vertex.id = MergeSubIDsIntoID( vertex );
  }
  else
  {
    SplitIDIntoSubIDs( vertex.id, vertex );
  }
}

std::string MergeSubIDsIntoID( const myVertex& vertex )
{
  char result[19] = { 0 }; // 16 characters for the hexadecimal string + "0x" prefix + null terminator

  const auto fid = s2u<uint64_t>( vertex.id_fid ) & ID_FID_MSK;
  if ( fid >= idFormats.size() )
    throw std::runtime_error( "bad format id (FID) " + std::to_string( fid ) + " field in Node '" + vertex.name + "'" );
  const auto& format = idFormats[fid];

  uint64_t id = 0;
  for ( const auto& prop : format )
  {
    uint64_t bitmaskForBitrange = ( 1 << prop.bits ) - 1; // Set # of prop.bits to 1
    uint64_t subIdValue         = GetSubIdByName( vertex, prop.s );
    id |= ( subIdValue & bitmaskForBitrange ) << prop.pos;
  }

  snprintf( result, sizeof( result ), "0x%016" PRIx64, id );
  return result;
}

void SplitIDIntoSubIDs( const std::string& id, myVertex& vertex )
{
  uint64_t idValue;
  try
  {
    sscanf( id.c_str(), "0x%" PRIx64, &idValue );
  }
  catch ( const std::exception& e )
  {
    throw std::runtime_error( "Invalid ID format: " + std::string( e.what() ) );
  }

  const auto fid = ( idValue >> ID_FID_POS ) & ID_FID_MSK;
  if ( fid >= idFormats.size() )
    throw std::runtime_error( "bad format id (FID) " + std::to_string( fid ) + " within ID field of Node '" +
                              vertex.name + "'" );

  const auto& format = idFormats[fid];
  for ( const auto& prop : format )
  {
    uint64_t bitmaskForBitrange = ( 1 << prop.bits ) - 1; // Set # of prop.bits to 1
    uint64_t subIdValue         = ( idValue >> prop.pos ) & bitmaskForBitrange;
    SetSubIdByName( prop.s, subIdValue, vertex );
  }
}

void GenerateQmeta( Graph& g, vertex_t v, int prio )
{
  // std::cout << "generating " << g[v].name << ", patname " << g[v].patName << " prio " << (int)prio << std::endl;

  const std::string nameBl =
      g[v].name + DotStr::Node::MetaGen::sQBufListTag + DotStr::Node::MetaGen::sQPrioPrefix[prio];
  const std::string nameB0 = g[v].name + DotStr::Node::MetaGen::sQBufTag + DotStr::Node::MetaGen::sQPrioPrefix[prio] +
                             DotStr::Node::MetaGen::s1stQBufSuffix;
  const std::string nameB1 = g[v].name + DotStr::Node::MetaGen::sQBufTag + DotStr::Node::MetaGen::sQPrioPrefix[prio] +
                             DotStr::Node::MetaGen::s2ndQBufSuffix;

  auto hashFunc = std::hash<std::string>();

  vertex_t vBl = boost::add_vertex(
      myVertex( nameBl, g[v].cpu, hashFunc( nameBl ), nullptr, dnt::sQInfo, DotStr::Misc::sHexZero ), g );
  vertex_t vB0 = boost::add_vertex(
      myVertex( nameB0, g[v].cpu, hashFunc( nameB0 ), nullptr, dnt::sQBuf, DotStr::Misc::sHexZero ), g );
  vertex_t vB1 = boost::add_vertex(
      myVertex( nameB1, g[v].cpu, hashFunc( nameB1 ), nullptr, dnt::sQBuf, DotStr::Misc::sHexZero ), g );

  g[vBl].patName = g[v].patName;
  g[vB0].patName = g[v].patName;
  g[vB1].patName = g[v].patName;

  boost::add_edge( v, vBl, myEdge( DotStr::Edge::TypeVal::sQPrio[prio] ), g );
  boost::add_edge( vBl, vB0, myEdge( DotStr::Edge::TypeVal::sMeta ), g );
  boost::add_edge( vBl, vB1, myEdge( DotStr::Edge::TypeVal::sMeta ), g );
}

void GenerateDstLst( Graph& g, vertex_t v, unsigned multiDst )
{

  const std::string prefix = g[v].name;
  unsigned          loops  = ( multiDst + 1 + DST_MAX - 1 ) /
                   DST_MAX; // add 1 to multidst. its the altdst count, but we need 1 more for a defdst.

  auto hashFunc = std::hash<std::string>();
  for ( unsigned i = 0; i < loops; i++ )
  {
    const std::string name = prefix + DotStr::Node::MetaGen::sDstListSuffix + "_" + std::to_string( i );
    vertex_t          vD   = boost::add_vertex(
        myVertex( name, g[v].cpu, hashFunc( name ), nullptr, dnt::sDstList, DotStr::Misc::sHexZero ), g );
    // FIXME add to grouptable
    g[vD].patName   = g[v].patName;
    edge_t thisEdge = ( boost::add_edge( vD, v, myEdge( DotStr::Edge::TypeVal::sDefDst ), g ) ).first;
  }
}

void GenerateBlockMeta( Graph& g, bool doGenerateDstLst )
{
  Graph::out_edge_iterator out_begin, out_end, out_cur;

  BOOST_FOREACH ( vertex_t v, vertices( g ) )
  {
    std::string cmp = g[v].type;
    if ( ( cmp == dnt::sBlockFixed ) || ( cmp == dnt::sBlockAlign ) || ( cmp == dnt::sBlock ) )
    {
      // std::cout << "Scanning Block " << g[v].name << std::endl;
      boost::tie( out_begin, out_end ) = out_edges( v, g );
      // check if it already has queue links / Destination List

      bool     genIl, genHi, genLo;
      bool     hasIl = false, hasHi = false, hasLo = false, hasDstLst = false;
      unsigned multiDst = 0;
      try
      {
        genIl = s2u<bool>( g[v].qIl );
        genHi = s2u<bool>( g[v].qHi );
        genLo = s2u<bool>( g[v].qLo );
      }
      catch ( const std::runtime_error& err )
      {
        throw std::runtime_error( "Parser error when processing node <" + g[v].name + ">. Cause: " + err.what() );
      }

      for ( out_cur = out_begin; out_cur != out_end; ++out_cur )
      {
        if ( g[*out_cur].type == DotStr::Node::TypeVal::sQPrio[PRIO_IL] )
          hasIl = true;
        if ( g[*out_cur].type == DotStr::Node::TypeVal::sQPrio[PRIO_HI] )
          hasHi = true;
        if ( g[*out_cur].type == DotStr::Node::TypeVal::sQPrio[PRIO_LO] )
          hasLo = true;
        if ( g[*out_cur].type == DotStr::Edge::TypeVal::sAltDst )
          multiDst++;
        if ( g[*out_cur].type == DotStr::Node::TypeVal::sDstList )
          hasDstLst = true;
      }

      // create requested Queues / Destination List
      if ( genIl && !hasIl )
      {
        GenerateQmeta( g, v, PRIO_IL );
      }
      if ( genHi && !hasHi )
      {
        GenerateQmeta( g, v, PRIO_HI );
      }
      if ( genLo && !hasLo )
      {
        GenerateQmeta( g, v, PRIO_LO );
      }

      if ( ( multiDst || genIl || hasIl || genHi || hasHi || genLo || hasLo ) && doGenerateDstLst )
      {
        GenerateDstLst( g, v, multiDst );
      }
    }
  }
}

void InitializeGraphNodes( Graph& g )
{
  std::string cmp;
  uint32_t    flags;

  GenerateBlockMeta( g, true ); // generate block meta data and queues

  BOOST_FOREACH ( vertex_t v, vertices( g ) )
  {
    const auto& name = g[v].name;
    if ( g[v].np != nullptr )
      continue;

    try
    {
      flags = ( ( s2u<bool>( g[v].bpEntry ) ) << NFLG_BP_ENTRY_LM32_POS ) |
              ( ( s2u<bool>( g[v].bpExit ) ) << NFLG_BP_EXIT_LM32_POS ) |
              ( ( s2u<bool>( g[v].patEntry ) ) << NFLG_PAT_ENTRY_LM32_POS ) |
              ( ( s2u<bool>( g[v].patExit ) ) << NFLG_PAT_EXIT_LM32_POS );
    }
    catch ( const std::runtime_error& err )
    {
      throw std::runtime_error( "Parser error when processing pattern/BP entry/exit tags of node <" + name +
                                ">. Cause: " + err.what() );
    }

    try
    {
      // add timing node data objects to vertices
      cmp = g[v].type;
      if ( cmp == dnt::sTMsg )
      {
        SyncIdSubIds( g[v] );
        g[v].np = ( node_ptr ) new TimingMsg( g[v].name,
                                              g[v].patName,
                                              g[v].bpName,
                                              g[v].hash,
                                              s2u<uint8_t>( g[v].cpu ),
                                              flags,
                                              s2u<uint64_t>( g[v].tOffs ),
                                              s2u<uint64_t>( g[v].id ),
                                              s2u<uint64_t>( g[v].par ),
                                              s2u<uint32_t>( g[v].tef ),
                                              s2u<uint32_t>( g[v].res ) );
      }
      else if ( cmp == dnt::sCmdNoop )
      {
        g[v].np = ( node_ptr ) new Noop( g[v].name,
                                         g[v].patName,
                                         g[v].bpName,
                                         g[v].hash,
                                         s2u<uint8_t>( g[v].cpu ),
                                         flags,
                                         s2u<uint64_t>( g[v].tOffs ),
                                         s2u<uint64_t>( g[v].tValid ),
                                         s2u<uint8_t>( g[v].prio ),
                                         s2u<uint32_t>( g[v].qty ),
                                         s2u<bool>( g[v].vabs ) );
      }
      else if ( cmp == dnt::sCmdFlow )
      {
        g[v].np = ( node_ptr ) new Flow( g[v].name,
                                         g[v].patName,
                                         g[v].bpName,
                                         g[v].hash,
                                         s2u<uint8_t>( g[v].cpu ),
                                         flags,
                                         s2u<uint64_t>( g[v].tOffs ),
                                         s2u<uint64_t>( g[v].tValid ),
                                         s2u<uint8_t>( g[v].prio ),
                                         s2u<uint32_t>( g[v].qty ),
                                         s2u<bool>( g[v].vabs ),
                                         s2u<bool>( g[v].perma ) );
      }
      else if ( cmp == dnt::sSwitch )
      {
        g[v].np = ( node_ptr ) new Switch( g[v].name,
                                           g[v].patName,
                                           g[v].bpName,
                                           g[v].hash,
                                           s2u<uint8_t>( g[v].cpu ),
                                           flags,
                                           s2u<uint64_t>( g[v].tOffs ) );
      }
      else if ( cmp == dnt::sOrigin )
      {
        g[v].np = ( node_ptr ) new Origin( g[v].name,
                                           g[v].patName,
                                           g[v].bpName,
                                           g[v].hash,
                                           s2u<uint8_t>( g[v].cpu ),
                                           flags,
                                           s2u<uint64_t>( g[v].tOffs ),
                                           s2u<uint8_t>( g[v].thread ) );
      }
      else if ( cmp == dnt::sStartThread )
      {
        g[v].np = ( node_ptr ) new StartThread( g[v].name,
                                                g[v].patName,
                                                g[v].bpName,
                                                g[v].hash,
                                                s2u<uint8_t>( g[v].cpu ),
                                                flags,
                                                s2u<uint64_t>( g[v].tOffs ),
                                                s2u<uint64_t>( g[v].startOffs ),
                                                s2u<uint32_t>( g[v].thread ) );
      }
      else if ( cmp == dnt::sCmdFlush )
      {
        g[v].np = ( node_ptr ) new Flush( g[v].name,
                                          g[v].patName,
                                          g[v].bpName,
                                          g[v].hash,
                                          s2u<uint8_t>( g[v].cpu ),
                                          flags,
                                          s2u<uint64_t>( g[v].tOffs ),
                                          s2u<uint64_t>( g[v].tValid ),
                                          s2u<uint8_t>( g[v].prio ),
                                          s2u<bool>( g[v].qIl ),
                                          s2u<bool>( g[v].qHi ),
                                          s2u<bool>( g[v].qLo ),
                                          s2u<bool>( g[v].vabs ),
                                          s2u<bool>( g[v].perma ),
                                          s2u<uint8_t>( g[v].frmIl ),
                                          s2u<uint8_t>( g[v].toIl ),
                                          s2u<uint8_t>( g[v].frmHi ),
                                          s2u<uint8_t>( g[v].toHi ),
                                          s2u<uint8_t>( g[v].frmLo ),
                                          s2u<uint8_t>( g[v].toLo ) );
      }
      else if ( cmp == dnt::sCmdWait )
      {
        g[v].np = ( node_ptr ) new Wait( g[v].name,
                                         g[v].patName,
                                         g[v].bpName,
                                         g[v].hash,
                                         s2u<uint8_t>( g[v].cpu ),
                                         flags,
                                         s2u<uint64_t>( g[v].tOffs ),
                                         s2u<uint64_t>( g[v].tValid ),
                                         s2u<uint8_t>( g[v].prio ),
                                         s2u<uint64_t>( g[v].tWait ),
                                         s2u<bool>( g[v].vabs ) );
      }
      else if ( cmp == dnt::sBlock )
      {
        g[v].np = ( node_ptr ) new BlockFixed( g[v].name,
                                               g[v].patName,
                                               g[v].bpName,
                                               g[v].hash,
                                               s2u<uint8_t>( g[v].cpu ),
                                               flags,
                                               s2u<uint64_t>( g[v].tPeriod ) );
      }
      else if ( cmp == dnt::sBlockFixed )
      {
        g[v].np = ( node_ptr ) new BlockFixed( g[v].name,
                                               g[v].patName,
                                               g[v].bpName,
                                               g[v].hash,
                                               s2u<uint8_t>( g[v].cpu ),
                                               flags,
                                               s2u<uint64_t>( g[v].tPeriod ) );
      }
      else if ( cmp == dnt::sBlockAlign )
      {
        g[v].np = ( node_ptr ) new BlockAlign( g[v].name,
                                               g[v].patName,
                                               g[v].bpName,
                                               g[v].hash,
                                               s2u<uint8_t>( g[v].cpu ),
                                               flags,
                                               s2u<uint64_t>( g[v].tPeriod ) );
      }
      else if ( cmp == dnt::sQInfo )
      {
        g[v].np = ( node_ptr ) new CmdQMeta(
            g[v].name, g[v].patName, g[v].bpName, g[v].hash, s2u<uint8_t>( g[v].cpu ), flags );
      }
      else if ( cmp == dnt::sDstList )
      {
        g[v].np = ( node_ptr ) new DestList(
            g[v].name, g[v].patName, g[v].bpName, g[v].hash, s2u<uint8_t>( g[v].cpu ), flags );
      }
      else if ( cmp == dnt::sQBuf )
      {
        g[v].np = ( node_ptr ) new CmdQBuffer(
            g[v].name, g[v].patName, g[v].bpName, g[v].hash, s2u<uint8_t>( g[v].cpu ), flags );
      }
      else if ( cmp == dnt::sGlobal )
      {
        g[v].np = ( node_ptr ) new Global(
            g[v].name, g[v].patName, g[v].bpName, g[v].hash, s2u<uint8_t>( g[v].cpu ), flags, g[v].section );
      }
      else if ( cmp == dnt::sMeta )
      {
        throw std::runtime_error( "Pure meta type not yet implemented" );
        return;
      }
      // FIXME try to get info from download
      else
      {
        throw std::runtime_error(
            "Node <" + g[v].name + ">'s type <" + cmp +
            "> is not supported!\nMost likely you forgot to set the type attribute or accidentally created the node by a typo in an edge definition." );
        return;
      }
    }
    catch ( const std::runtime_error& err )
    {
      throw std::runtime_error( "Failed to create data object for node <" + name + "> of type <" + cmp +
                                ">. Cause: " + err.what() );
    }
  }
}