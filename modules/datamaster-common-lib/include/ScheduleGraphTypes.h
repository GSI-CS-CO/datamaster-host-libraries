#pragma once

// #include "magic_enum/magic_enum.hpp"
#include "magic_enum/magic_enum.hpp"

namespace carpeDM
{
enum class VertexType
{
  Tmsg,
  Noop,
  Flow,
  Switch,
  Origin,
  Flush,
  Wait,
  Start,
  Stop,
  Abort,
  StartThread,
  Lock,
  Unlock,
  AsyncClear,
  Block,
  BLockAlign,
  QInfo,
  ListDst,
  QBuf,
  Meta,
  Global
};
}; // namespace carpeDM
