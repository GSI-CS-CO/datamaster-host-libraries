#pragma once

#include <numeric>
#include <optional>
#include <string>
#include <vector>

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

template <typename T>
inline std::optional<T> ConcatenateErrors( const std::vector<T>& errors )
{
  if ( errors.empty() )
  {
    return std::nullopt; // No errors to concatenate
  }

  const auto concatenatedMessage = std::accumulate( errors.begin(),
                                                    errors.end(),
                                                    std::string{},
                                                    []( const std::string& acc, const T& error )
                                                    {
                                                      return acc + ( acc.empty() ? "" : "\n" ) + error.message;
                                                    } );

  return T{ concatenatedMessage };
}

} // namespace carpeDM