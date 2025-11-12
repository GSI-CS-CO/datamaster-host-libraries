#pragma once

#include "AST.h"

#include "DotParser.h"
#include "Errors.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace carpeDM2
{

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
using Field = std::variant<T, datalang::Program>;

typedef uint32_t node_hash_t;

struct Node
{
  node_hash_t                      hash;
  std::string                      nodeName;
  std::optional<datalang::Program> condition;
};

struct BlockNode : public Node
{
  bool            align;
  Field<uint64_t> tPeriod;
};

struct TimingMessageNode : public Node
{
  Field<uint64_t> id;
  Field<uint64_t> par;

  // timing extension field - subnano second portion of toffs
  Field<uint32_t> tef;

  // reserved field - currently unused
  Field<uint32_t> res;
};

enum class VariableScope
{
  Global,
  Local
};

struct VariableNode : public Node
{
  std::string       variableName;
  VariableScope     scope;
  datalang::Program updateExpression;
};

struct SwitchNode : public Node
{
  Field<uint32_t>          expression;
  std::vector<node_hash_t> cases;
};

using TaskNode = std::variant<BlockNode, TimingMessageNode, VariableNode, SwitchNode>;

enum class TaskNodeType
{
  Block,
  TimingMessage,
  Variable,
  Switch
};

class TaskGraph
{
public:
  /**
   * @brief Parse a TaskGraph from a DotGraph.
   * @param dotgraph The DotGraph to parse.
   * @return The parsed TaskGraph or a ConversionError.
   *
   * @todo: Factor out verification of node attributes to a separate function?
   */
  static std::variant<TaskGraph, carpeDM::ConversionError> parse( const carpeDM::DotGraph& dotgraph );

  inline const std::string& getName() const
  {
    return m_name;
  }

  inline const TaskNode* const getNodeByName( const std::string& name ) const
  {
    return getNodeByHash( fnv1a_hash( name ) );
  }

  inline const TaskNode* const getNodeByHash( node_hash_t hash ) const
  {
    auto it = m_nodeIndex.find( hash );
    if ( it != m_nodeIndex.end() )
    {
      return &m_nodes[it->second];
    }
    return nullptr;
  }

  inline const TaskNode* const getNodeByIndex( size_t index ) const
  {
    if ( index < m_nodes.size() )
    {
      return &m_nodes[index];
    }
    return nullptr;
  }

  inline const std::vector<TaskNode>& getNodes() const
  {
    return m_nodes;
  }

private:
  explicit TaskGraph( std::string                                 name,
                      std::vector<TaskNode>&&                     nodes,
                      std::unordered_map<uint32_t, std::string>&& nodeNames,
                      std::unordered_map<uint32_t, size_t>&&      nodeIndex,
                      node_hash_t                                 entryNode )
      : m_name( std::move( name ) )
      , m_nodes( std::move( nodes ) )
      , m_nodeNames( std::move( nodeNames ) )
      , m_nodeIndex( std::move( nodeIndex ) )
      , m_entryNode( entryNode )
  {
  }

private:
  std::string                               m_name;
  std::vector<TaskNode>                     m_nodes;
  std::unordered_map<uint32_t, std::string> m_nodeNames;
  std::unordered_map<uint32_t, size_t>      m_nodeIndex;
  node_hash_t                               m_entryNode;
};
} // namespace carpeDM2