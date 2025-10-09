#include "ScheduleNodeMemoryManager.h"

using carpeDM::DataLoadError;
using carpeDM::ScheduleNodeMemoryManager;

ScheduleNodeMemoryManager::ScheduleNodeMemoryManager( uint32_t maxNodes, uint32_t nodeSize, uint8_t alignment )
    : m_maxNodes( maxNodes )
    , m_nodeSize( nodeSize )
    , m_alignment( alignment )
    , m_memory( maxNodes * nodeSize, 0 )
    , m_allocated( maxNodes, false )
{
}

std::optional<DataLoadError> ScheduleNodeMemoryManager::loadData( const std::span<const uint8_t> data )
{
  if ( data.size() > m_memory.size() )
  {
    return DataLoadError{ "Data size exceeds allocated memory." };
  }

  std::copy( data.begin(), data.end(), m_memory.begin() );
  // null the rest
  std::fill( m_memory.begin() + data.size(), m_memory.end(), 0 );

  for ( size_t i = 0; i < m_memory.size(); i++ )
  {
    setAllocated( i, i < data.size() );
  }

  return std::nullopt;
}

uint32_t ScheduleNodeMemoryManager::allocateNode()
{
  for ( uint32_t i = 0; i < m_maxNodes; i++ )
  {
    if ( !m_allocated[i] )
    {
      setAllocated( i, true );
      return i;
    }
  }
  return INVALID_NODE_ID;
}

uint32_t ScheduleNodeMemoryManager::allocateNode( uint32_t preferredNodeId )
{
  if ( preferredNodeId < m_maxNodes && !m_allocated[preferredNodeId] )
  {
    setAllocated( preferredNodeId, true );
    return preferredNodeId;
  }
  return INVALID_NODE_ID;
}

void ScheduleNodeMemoryManager::freeNode( uint32_t nodeId )
{
  if ( nodeId < m_maxNodes )
  {
    setAllocated( nodeId, false );
    std::fill( m_memory.begin() + nodeId * m_nodeSize, m_memory.begin() + ( nodeId + 1 ) * m_nodeSize, 0 );
  }
}

uint32_t ScheduleNodeMemoryManager::getMaxNodes() const
{
  return m_maxNodes;
}

void ScheduleNodeMemoryManager::write( uint32_t nodeId, const std::span<const uint8_t> data )
{
  if ( nodeId < m_maxNodes && m_allocated[nodeId] && data.size() <= m_nodeSize )
  {
    std::copy( data.begin(), data.end(), m_memory.begin() + nodeId * m_nodeSize );
    // null the rest
    std::fill(
        m_memory.begin() + nodeId * m_nodeSize + data.size(), m_memory.begin() + ( nodeId + 1 ) * m_nodeSize, 0 );
  }
}

void ScheduleNodeMemoryManager::read( uint32_t nodeId, const std::span<uint8_t> data ) const
{
  if ( nodeId < m_maxNodes && m_allocated[nodeId] && data.size() <= m_nodeSize )
  {
    std::copy( m_memory.begin() + nodeId * m_nodeSize, m_memory.begin() + ( nodeId + 1 ) * m_nodeSize, data.begin() );
  }
}

const std::span<const uint8_t> ScheduleNodeMemoryManager::getNodeMemory( uint32_t nodeId ) const
{
  if ( nodeId < m_maxNodes && m_allocated[nodeId] )
  {
    return std::span<const uint8_t>( m_memory.data() + nodeId * m_nodeSize, m_nodeSize );
  }
  return std::span<const uint8_t>();
}

void ScheduleNodeMemoryManager::setAllocated( uint32_t index, bool value )
{
  if ( index < m_maxNodes )
  {
    m_allocated[index] = value;
  }
}