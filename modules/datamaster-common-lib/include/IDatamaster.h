#pragma once

namespace carpeDM
{
class ScheduleGraph;

class IDatamaster
{
public:
  virtual ~IDatamaster() = default;

  virtual bool           clearAllGraphs()                               = 0;
  virtual ScheduleGraph* downloadGraph() const                          = 0;
  virtual bool           addGraph( const ScheduleGraph& graph )         = 0;
  virtual bool           removeGraph( const ScheduleGraph& graph )      = 0;
  virtual bool           removeExcept( const ScheduleGraph& graph )     = 0;
  virtual bool           checkRemoveGraph( const ScheduleGraph& graph ) = 0;
};
} // namespace carpeDM