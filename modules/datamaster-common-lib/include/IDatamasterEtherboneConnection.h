#pragma once

#include <cstdint>
#include <etherbone.h>
#include <string>

struct sdb_device;

namespace carpeDM
{
struct DatamasterMetaInformation
{
  uint32_t numberOfRamDevices;
  uint32_t numberOfDiagDevices;
  uint32_t numberOfClusterTimeModules;
};

class IDatamasterEtherboneConnection
{
public:
  virtual ~IDatamasterEtherboneConnection() = default;

  virtual DatamasterMetaInformation getMetaInformation() const = 0;

  virtual std::string getBuildInfo() const = 0;

  virtual bool isConnected() const = 0;

  virtual eb_device_t getEbDevice() const = 0;

  virtual const sdb_device* const getRAMDevice( uint32_t index ) const         = 0;
  virtual const sdb_device* const getDiagDevice( uint32_t index ) const        = 0;
  virtual const sdb_device* const getClusterTimeModule( uint32_t index ) const = 0;
};
} // namespace carpeDM