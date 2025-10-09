#include "ScheduleGraphSerializer.h"

using carpeDM::DefaultScheduleGraphSerializer;
using carpeDM::IScheduleGraphSerializer;

DefaultScheduleGraphSerializer::DefaultScheduleGraphSerializer()
{
}

DefaultScheduleGraphSerializer::~DefaultScheduleGraphSerializer()
{
}

bool DefaultScheduleGraphSerializer::serialize( const ScheduleGraph& graph, std::span<uint8_t>& output ) const
{
}

bool DefaultScheduleGraphSerializer::deserialize( std::span<const uint8_t> input, ScheduleGraph& graph ) const
{
}
