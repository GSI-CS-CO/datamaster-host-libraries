#pragma once

#include "IDatamasterEtherboneConnection.h"
#include "IMemoryRegion.h"
#include <span>

namespace carpeDM
{
class LM32MemoryRegion : public IMemoryRegion
{
public:
  LM32MemoryRegion( uint64_t baseAddress, uint64_t size, IDatamasterEtherboneConnection& connection );
  virtual ~LM32MemoryRegion() = default;

public:
  void read( uint64_t address, std::span<uint8_t> memory ) const override;
  void write( uint64_t address, const std::span<uint8_t>& memory ) override;

private:
  uint64_t                        m_baseAddress;
  uint64_t                        m_size;
  IDatamasterEtherboneConnection& m_connection;
};
} // namespace carpeDM