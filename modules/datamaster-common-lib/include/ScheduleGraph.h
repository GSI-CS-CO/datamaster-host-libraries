#pragma once

#include "datamaster-common-lib_export.h"

#include "Errors.h"
#include "ScheduleGraphTypes.h"

#include <variant>

namespace carpeDM
{

class DotGraph;

typedef uint32_t node_hash_t;

// We set the invalid node has to 0 - the probability of a node having a hash of 0 is very low "less than one in ten
// quintillion."
constexpr node_hash_t INVALID_NODE_HASH = 0;

struct __attribute__( ( packed ) ) NodeFlags
{
  // Node type (e.g., Event, Block, etc.)
  uint32_t type : 8;

  // Node flag field bit defs - Paint bit - the lm32 has visited this node
  uint32_t painted_lm32 : 1;

  // Node flag field bit defs - paint bit - the host has visited this node - NOT IMPLEMENTED
  uint32_t painted_host : 1; // Node is painted on the host

  // Node flag field bit defs - sync bit - this node should only be started synchronous to another
  uint32_t sync : 1;

  uint32_t padding : 1;

  // Node is a beamproc entry point
  uint32_t bpentry : 1;

  // Node is a pattern entry point
  uint32_t patentry : 1;

  // Node is a beamproc exit point
  uint32_t bpexit : 1;

  // Node is a pattern exit point
  uint32_t patexit : 1;

  // Debug flag, used for debugging purposes
  uint32_t debug : 2;

  // Reserved for future use
  uint32_t _padding : 2;

  uint32_t specific : 5; // Type-specific bits (bits 28-31)

  constexpr NodeFlags()
      : type( 0 )
      , painted_lm32( 0 )
      , painted_host( 0 )
      , sync( 0 )
      , padding( 0 )
      , bpentry( 0 )
      , patentry( 0 )
      , bpexit( 0 )
      , patexit( 0 )
      , debug( 0 )
      , _padding( 0 )
      , specific( 0 )
  {
  }
};

static_assert( sizeof( NodeFlags ) == sizeof( uint32_t ), "NodeFlags must be 4 bytes" );

/**
 * The data structure used by all nodes in the schedule graph.
 * It contains the node's name, pattern, beamproc, hash, CPU and flags.
 */
struct Node
{
  std::string name     = "";
  std::string pattern  = "";
  std::string beamproc = "";
  uint32_t    hash     = 0;
  uint8_t     cpu      = 0;
  NodeFlags   flags    = {};

  node_hash_t defaultDestination = INVALID_NODE_HASH;
};

struct Event : public Node
{
  uint64_t tOffs;
};

// Timing Message struct (corresponds to TimingMsg class)
struct TimingMessage : public Event
{
  uint64_t id;
  uint64_t par;
  uint32_t tef;
  uint32_t res;
};

// Switch struct (corresponds to Switch class)
struct Switch : public Event
{
};

// Origin struct (corresponds to Origin class)
struct Origin : public Event
{
  uint32_t thread;
};

// StartThread struct (corresponds to StartThread class)
struct StartThread : public Event
{
  uint64_t startOffs;
  uint32_t thread;
};

// Block struct (corresponds to Block class)
struct Block : public Node
{
  uint64_t tPeriod = 0;
  uint8_t  rdIdxIl = 0;
  uint8_t  rdIdxHi = 0;
  uint8_t  rdIdxLo = 0;
  uint8_t  wrIdxIl = 0;
  uint8_t  wrIdxHi = 0;
  uint8_t  wrIdxLo = 0;
};

// Base Command struct (corresponds to Command class)
struct Command : public Event
{
  uint64_t tValid;

  // Concrete bitfield breakdown of the 'act' field
  struct ActField
  {
    uint32_t qty      : 20; // ACT_QTY_MSK (bits 0-19)
    uint32_t type     : 4;  // ACT_TYPE_MSK (bits 20-23)
    uint32_t prio     : 2;  // ACT_PRIO_MSK (bits 24-25)
    uint32_t vabs     : 1;  // ACT_VABS_MSK (bit 26)
    uint32_t chp      : 1;  // ACT_CHP_MSK (bit 27)
    uint32_t specific : 4;  // Type-specific bits (bits 28-31)
  } act;
};

// NoOp Command struct (corresponds to Noop class)
struct NoOp : public Command
{
  // act field breakdown for NoOp:
  // - qty: number of times to execute
  // - type: ACT_TYPE_NOOP (1)
  // - prio: priority queue (0=LO, 1=HI, 2=IL)
  // - vabs: whether tValid is absolute (0) or relative (1)
  // - chp: unused for NoOp (should be 0)
  // - specific: unused for NoOp (should be 0)
};

// Flow Command struct (corresponds to Flow class)
struct Flow : public Command
{
  // act field breakdown for Flow:
  // - qty: number of times to execute
  // - type: ACT_TYPE_FLOW (2)
  // - prio: priority queue (0=LO, 1=HI, 2=IL)
  // - vabs: whether tValid is absolute (0) or relative (1)
  // - chp: whether change is permanent (1) or temporary (0)
  // - specific: unused for Flow (should be 0)
};

// Wait Command struct (corresponds to Wait class)
struct Wait : public Command
{
  uint64_t tWait;

  // act field breakdown for Wait:
  // - qty: always 1 for Wait commands
  // - type: ACT_TYPE_WAIT (4)
  // - prio: priority queue (0=LO, 1=HI, 2=IL)
  // - vabs: whether tValid is absolute (0) or relative (1)
  // - chp: whether change is permanent (1) or temporary (0)
  // - specific[0]: ACT_WAIT_ABS - whether tWait is absolute (1) or relative to tPeriod (0)
  // - specific[1-3]: unused (should be 0)
};

// Flush Command struct (corresponds to Flush class)
struct Flush : public Command
{
  uint8_t mode;
  // from, to: range of queues to flush
  // - frmIl, toIl: range for IL queue
  // - frmHi, toHi: range for HI queue
  // - frmLo, toLo: range for LO queue
  // mode: 0=all queues, 1=IL only, 2=HI only, 3=LO only
  uint8_t frmIl, toIl;
  uint8_t frmHi, toHi;
  uint8_t frmLo, toLo;

  // act field breakdown for Flush:
  // - qty: always 1 for Flush commands
  // - type: ACT_TYPE_FLUSH (3)
  // - prio: priority queue (0=LO, 1=HI, 2=IL)
  // - vabs: whether tValid is absolute (0) or relative (1)
  // - chp: whether change is permanent (1) or temporary (0)
  // - specific[0-2]: ACT_FLUSH_PRIO - which queues to flush (bit 0=LO, bit 1=HI, bit 2=IL)
  // - specific[3]: unused (should be 0)
};

// Meta node structs (for generated metadata)
struct Global : public Node
{
  std::string section;
};

struct CmdQMeta : public Node
{
};

struct CmdQBuffer : public Node
{
};

struct DestList : public Node
{
};

// Union type for all schedule graph nodes (similar to ScheduleGraph.h)
using ScheduleGraphNode = std::variant<Block,
                                       TimingMessage,
                                       Switch,
                                       Origin,
                                       StartThread,
                                       Command,
                                       NoOp,
                                       Flow,
                                       Wait,
                                       Flush,
                                       Global,
                                       CmdQMeta,
                                       CmdQBuffer,
                                       DestList>;

inline uint32_t fnv1a_hash( const std::string& data )
{
  uint32_t hash  = 0x811C9DC5;
  uint32_t prime = 0x01000193;
  for ( char c : data )
  {
    hash ^= c;
    hash *= prime;
  }
  return hash;
}

class DATAMASTER_COMMON_LIB_EXPORT ScheduleGraph
{
public:
  ScheduleGraph( std::string name );
  ~ScheduleGraph() = default;

  inline const std::string& getName() const
  {
    return m_name;
  }

  void setNodes( std::vector<ScheduleGraphNode> nodes );

  static std::variant<ScheduleGraph, ConversionError> fromDotGraph( const DotGraph& dotGraph );

  inline const ScheduleGraphNode* const getNodeByName( const std::string& name ) const
  {
    return getNodeByHash( fnv1a_hash( name ) );
  }

  inline const ScheduleGraphNode* const getNodeByHash( node_hash_t hash ) const
  {
    auto it = m_nodeIndex.find( hash );
    if ( it != m_nodeIndex.end() )
    {
      return &m_nodes[it->second];
    }
    return nullptr;
  }

  inline const ScheduleGraphNode* const getNodeByIndex( size_t index ) const
  {
    if ( index < m_nodes.size() )
    {
      return &m_nodes[index];
    }
    return nullptr;
  }

  inline const std::vector<ScheduleGraphNode>& getNodes() const
  {
    return m_nodes;
  }

private:
  std::unordered_map<uint32_t, size_t> m_nodeIndex; // Maps node hash to index in m_nodes
  std::vector<ScheduleGraphNode>       m_nodes;
  std::string                          m_name;
};
} // namespace carpeDM