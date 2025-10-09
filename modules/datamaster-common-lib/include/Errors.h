#pragma once

#include <string>

namespace carpeDM
{
struct ConversionError
{
  std::string message;
};

struct ParsingError
{
  std::string message;
};

struct FilterGraphError
{
  std::string message;
};

struct ConnectionError
{
  std::string message;
};

} // namespace carpeDM