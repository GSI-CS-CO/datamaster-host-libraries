#pragma once

#include <vector>

namespace carpeDM
{
class ScheduleGraph;

class IScheduleGraphLoader
{
public:
  virtual ~IScheduleGraphLoader() = default;

  virtual ScheduleGraph load( const DotGraph& dotGraph ) = 0;
};

class ISCheduleGraphStorer
{
  virtual DotGraph store( const ScheduleGraph& dotGraph ) = 0;
};

std::vector<uint8_t>
MergeGraphs( const ScheduleGraph& newGraph, const ScheduleGraph& oldGraph, const std::vector<uint8_t>& oldMemoryLayout )
{
}

ScheduleGraph loadGraphFromMemory( const std::vector<uint8_t>& graph ) = 0;

class IMemoryLayoutGenerator
{
public:
  virtual ~IMemoryLayoutGenerator()                                               = default;
  virtual std::vector<uint8_t> generateMemoryLayout( const ScheduleGraph& graph ) = 0;
}

class SpecificMemoryLayoutGenerator : public IMemoryLayoutGenerator
{
public:
  SpecificMemoryLayoutGenerator( uint32_t memSize );

  std::vector<uint8_t> generateMemoryLayout( const ScheduleGraph& graph ) override;

private:
  ebdevice m_device;
};

class IScheduleGraphUploader
{
public:
  virtual ~IScheduleGraphUploader()                                    = default;
  virtual void uploadGraph( const std::vector<uint8_t>& memoryLayout ) = 0;
};

class SpecificScheduleGraphLoader : public IScheduleGraphLoader
{
public:
  SpecificScheduleGraphLoader( ebdevice );

  ScheduleGraph load( const DotGraph& dotGraph ) override
};

class FileScheduleGraphLoader : public IScheduleGraphLoader
{
public:
  FileScheduleGraphLoader( const std::string& filePath );
  ScheduleGraph load( const DotGraph& dotGraph ) override;

private:
  std::string m_filePath;
};

} // namespace carpeDM