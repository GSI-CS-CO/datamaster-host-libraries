#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace carpeDM
{

struct DataLoadError
{
  std::string message;
};

class ScheduleNodeMemoryManager
{
public:
  static constexpr uint32_t INVALID_NODE_ID = UINT32_MAX;

public:
  ScheduleNodeMemoryManager( uint32_t maxNodes, uint32_t nodeSize, uint8_t alignment = 4 );

  uint32_t allocateNode();
  // tries to allocate the preferred node, otherwise returns invalid id
  uint32_t allocateNode( uint32_t preferredNodeId );
  void     freeNode( uint32_t nodeId );
  uint32_t getMaxNodes() const;

  std::optional<DataLoadError> loadData( const std::span<const uint8_t> data );

  void write( uint32_t nodeId, const std::span<const uint8_t> data );
  void read( uint32_t nodeId, const std::span<uint8_t> data ) const;

  const std::span<const uint8_t> getNodeMemory( uint32_t nodeId ) const;

private:
  void setAllocated( uint32_t index, bool value );

private:
  uint32_t             m_maxNodes;
  uint32_t             m_nodeSize;
  uint8_t              m_alignment;
  std::vector<uint8_t> m_memory;
  std::vector<bool>    m_allocated;
};
} // namespace carpeDM