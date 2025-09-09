#pragma once

// #include "magic_enum/magic_enum.hpp"
#include "magic_enum/magic_enum.hpp"
#include "ftm_common.h"

namespace carpeDM
{
enum class VertexType
{
  Tmsg = NODE_TYPE_TMSG,
  Noop = NODE_TYPE_CNOOP,
  Flow = NODE_TYPE_CFLOW,
  Flush = NODE_TYPE_CFLUSH,
  Wait = NODE_TYPE_CWAIT,
  Block = NODE_TYPE_BLOCK_FIXED,
  BlockAlign = NODE_TYPE_BLOCK_ALIGN,
  Origin = NODE_TYPE_ORIGIN,
};
}; // namespace carpeDM
