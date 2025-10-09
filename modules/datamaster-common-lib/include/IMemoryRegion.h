#pragma once

#include <cstdint>
#include <span>

namespace carpeDM
{
class IMemoryRegion
{
public:
  virtual ~IMemoryRegion() = default;

  virtual void read( uint64_t address, std::span<uint8_t> memory ) const   = 0;
  virtual void write( uint64_t address, const std::span<uint8_t>& memory ) = 0;
};
} // namespace carpeDM