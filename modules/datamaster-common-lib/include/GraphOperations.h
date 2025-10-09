#pragma once

#include "Errors.h"
#include "ScheduleGraph.h"

namespace carpeDM
{
std::variant<FilterGraphError, ScheduleGraph> FilterGraphBySchedule(
    const ScheduleGraph& graph,
    const std::string&   filterPattern);
}