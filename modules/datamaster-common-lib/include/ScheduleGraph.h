#pragma once

#include "datamaster-common-lib_export.h"

#include "Errors.h"
#include "ScheduleGraphTypes.h"

#include <memory>
#include <variant>

namespace carpeDM
{

constexpr uint32_t META  = 2;
constexpr uint32_t EVENT = 1000;
// 10 for now?! was 110, I do not know why we would need that many
constexpr uint32_t DST   = 110;
constexpr uint32_t DSTLL = 8;
constexpr uint32_t REF   = 3;

class DotGraph;

typedef uint32_t node_hash_t;

// We set the invalid node has to 0 - the probability of a node having a hash of 0 is very low "less than one in ten
// quintillion."
constexpr node_hash_t INVALID_NODE_HASH = 0;

enum class EdgeType
{
  listdst,
  defdst,
  altdst,
  baddefdst,
  target,
  switchdst,
  origindst,
  flowdst,
  flushovr,

  dynflowdst,
  resflowdst,
  domflowdst
};

enum class ReferenceEdgeType
{
  address,
  reference,
  reference2
};

class SimpleEdge
{
private:
  node_hash_t target;
  EdgeType    m_type;
};

class ReferenceEdge
{
private:
  node_hash_t       target;
  uint32_t          fieldHead;
  uint32_t          fieldTail;
  uint32_t          bitWidth;
  ReferenceEdgeType m_type;
};

struct __attribute__( ( packed ) ) NodeFlags
{
  union
  {
    uint32_t raw; // Raw flags as a 32-bit integer
    struct
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
    } content;               // Content of the flags, packed into a struct
  };
};

static_assert( sizeof( NodeFlags ) == sizeof( uint32_t ), "NodeFlags must be 4 bytes" );

/**
 * The data structure used by all nodes in the schedule graph.
 * It contains the node's name, pattern, beamproc, hash, CPU and flags.
 */
struct Node
{
  uint32_t    hash = 0;
  std::string name; // Node name

  // Mandatory
  uint8_t     cpu     = 0;
  std::string pattern = "";

  // Optional
  std::string beamproc           = "";
  node_hash_t defaultDestination = INVALID_NODE_HASH;
  // std::unique_ptr<AltDstList> altDest;

  // Node flags, packed into a 32-bit integer
  // How these are seet depends on the actual type of node
  // The node is encoded into the flags, which is the only mandatory field
  NodeFlags flags;
};

struct Event : public Node
{
  node_hash_t target = INVALID_NODE_HASH;
  uint64_t    tOffs;
};

struct TimingMessage : public Event
{
  uint64_t id;
  uint64_t par;

  // timing extension field - subnano second portion of toffs
  uint32_t tef;

  // reserved field - currently unused
  uint32_t res;

  std::vector<ReferenceEdgeType> references;
};

struct Switch : public Event
{
  std::vector<EdgeType> edges;

  // New default destination for the target node
  node_hash_t dest = INVALID_NODE_HASH;
};

struct Origin : public Event
{

  // threadIdx
  uint32_t thread;

  // New origin node for the target thread
  // Currerntly does not need to be entry or exist point
  // Mabye change later? As a developer we may still want to be able to use any node
  node_hash_t dest;
};

struct StartThread : public Event
{
  // thread start time - absolute time when to start the thread
  // @todo(mdennst): Needs to be renamed
  uint64_t startOffs;

  // threadIdx
  uint32_t thread;
};

struct Block : public Node
{
  // Block period in ns
  uint64_t tPeriod = 0;

  bool align = false;

  // Prio queue
  uint8_t rdIdxIl = 0;
  uint8_t rdIdxHi = 0;
  uint8_t rdIdxLo = 0;
  uint8_t wrIdxIl = 0;
  uint8_t wrIdxHi = 0;
  uint8_t wrIdxLo = 0;

  // List of possible alternative destinations for flow commands
  // EdgeContainer<DST> altDest;

  // EdgeContainer<REF> address;
  // EdgeContainer<REF> reference;
  // EdgeContainer<REF> reference2;
};

// Base Command struct (corresponds to Command class)
struct Command : public Event
{
  // time after this command becomes valid
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

  // interpret tvalid absolute or relative to current time sum
  bool absoluteValidTime;

  // target block
  node_hash_t commandTarget;
};

// NoOp Command struct (corresponds to Noop class)
struct NoOp : public Command
{
};

// Flow Command struct (corresponds to Flow class)
struct Flow : public Command
{
  // Change the defdst permanently or just once
  bool permanent;

  // new defdst of target block
  node_hash_t dest;
};

// Wait Command struct (corresponds to Wait class)
struct Wait : public Command
{
  // @todo(mdennst): check if a flag exists that allow to switch between relative and absolute wait time

  // wait time
  uint64_t tWait;
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

  // Flush overwrite
  // @todo(mdennst): Check what this is used for
  // Potentially: if set, the flush command does not go to the default successor, but to this node instead
  node_hash_t flushOvr;
};

// Meta node structs (for generated metadata)
struct Global : public Node
{
  // Section name for global settings
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

template <typename T>
decltype( auto ) GetFromNode( const ScheduleGraphNode& node, T Node::*member )
{
  return std::visit(
      [&]( const auto& n ) -> decltype( auto )
      {
        return n.*member;
      },
      node );
}

inline const std::string& GetNodeName( const ScheduleGraphNode& node )
{
  return GetFromNode( node, &Node::name );
}

inline const decltype( Node::hash ) GetNodeHash( const ScheduleGraphNode& node )
{
  return GetFromNode( node, &Node::hash );
}

class DATAMASTER_COMMON_LIB_EXPORT ScheduleGraph
{
public:
  static std::variant<ScheduleGraph, ConversionError> fromDotGraph( const DotGraph& dotGraph );
  ~ScheduleGraph() = default;

  inline const std::string& getName() const
  {
    return m_name;
  }

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
  ScheduleGraph( std::string                                 name,
                 std::vector<ScheduleGraphNode>&&            nodes,
                 std::unordered_map<uint32_t, std::string>&& nodeNames,
                 std::unordered_map<uint32_t, size_t>&&      nodeIndex );

private:
  std::unordered_map<uint32_t, std::string> m_nodeNames; // Maps node hash to node name

  std::unordered_map<uint32_t, size_t>      m_nodeIndex;    // Maps node hash to index in m_nodes
  std::unordered_map<uint32_t, std::string> m_nodePatterns; // Maps pattern name to pattern hash

  std::vector<ScheduleGraphNode> m_nodes;
  std::string                    m_name;
};
} // namespace carpeDM