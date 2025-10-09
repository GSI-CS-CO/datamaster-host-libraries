#pragma once
#include <string>

namespace carpeDM
{
class ScheduleGraph;
class IScheduleGraphSerializer
{
public:
  IScheduleGraphSerializer()          = default;
  virtual ~IScheduleGraphSerializer() = default;

  virtual bool serialize( const ScheduleGraph& graph, std::span<uint8_t>& output ) const = 0;
  virtual bool deserialize( std::span<const uint8_t> input, ScheduleGraph& graph ) const = 0;
};

class DefaultScheduleGraphSerializer : public IScheduleGraphSerializer
{
public:
  DefaultScheduleGraphSerializer()          = default;
  virtual ~DefaultScheduleGraphSerializer() = default;

  virtual bool serialize( const ScheduleGraph& graph, std::span<uint8_t>& output ) const override;
  virtual bool deserialize( std::span<const uint8_t> input, ScheduleGraph& graph ) const override;
};

} // namespace carpeDM