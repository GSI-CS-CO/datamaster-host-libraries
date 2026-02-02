#pragma once

#include "DotParser.h"
#include "Errors.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace carpeDM
{

/**
 * @brief Parse a value of type T from a string.
 * @tparam T The type to parse the value into.
 * @param value The string containing the value to parse.
 * @return The parsed value of type T.
 */
template <typename T>
constexpr inline T ParseValue( const std::string& value )
{
  if constexpr ( std::is_same_v<T, uint64_t> )
  {
    return static_cast<uint64_t>( std::stoull( value ) );
  }
  else if constexpr ( std::is_same_v<T, uint32_t> )
  {
    return static_cast<uint32_t>( std::stoul( value ) );
  }
  else if constexpr ( std::is_same_v<T, uint8_t> )
  {
    return static_cast<uint8_t>( std::stoi( value ) );
  }
  else if constexpr ( std::is_same_v<T, bool> )
  {
    if ( value == "true" || value == "1" )
    {
      return true;
    }
    else if ( value == "false" || value == "0" )
    {
      return false;
    }
    return false;
  }
  else if constexpr ( std::is_same_v<T, std::string> )
  {
    return value;
  }
  else
  {
    static_assert( std::is_same_v<T, uint64_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, uint8_t> ||
                       std::is_same_v<T, bool>,
                   "Unsupported type for ParseValue" );
  }
}

/**
 * @brief Parse an optional value of type T from a map of string key-value pairs.
 *        If the key is not found in the map, returns the provided default value.
 * @tparam T The type to parse the value into.
 * @param map The map of string key-value pairs.
 * @param key The key to look for in the map.
 * @param defaultValue The default value to return if the key is not found.
 * @return The parsed value of type T, or the default value if the key is not found.
 */
template <typename TargetType>
inline TargetType ParseOptionalValue( const std::unordered_map<std::string, std::string>& map,
                                      const std::string&                                  key,
                                      TargetType                                          defaultValue )
{
  auto found = map.count( key ) != 0;
  if ( !found )
  {
    return defaultValue;
  }
  return ParseValue<TargetType>( map.at( key ) );
}

/**
 * @brief Expect that a specific attribute is present in a vertex.
 * @param vertex The vertex to check.
 * @param attributeName The name of the attribute to look for.
 * @return An optional ConversionError if the attribute is missing, or std::nullopt if it is present.
 * @todo: Replace std::optional with std::expected when available.
 */
inline std::optional<ConversionError>
ExpectAttributeToBePresent( const decltype( DotGraph::vertices )::value_type::second_type& vertex,
                            const std::string&                                             attributeName )
{
  if ( vertex.attributes.find( attributeName ) == vertex.attributes.end() )
  {
    return ConversionError{ fmt::format( "Missing required attribute: {} in Vertex: {}", attributeName, vertex.id ) };
  }
  return std::nullopt; // Attribute is present
}

/**
 * @brief Expect that a specific attribute is present in an edge.
 * @param edge The edge to check.
 * @param attributeName The name of the attribute to look for.
 * @return An optional ConversionError if the attribute is missing, or std::nullopt if it is present.
 * @todo: Replace std::optional with std::expected when available.
 */
inline std::optional<ConversionError> ExpectAttributeToBePresent( const decltype( DotGraph::edges )::value_type& edge,
                                                                  const std::string& attributeName )
{
  if ( edge.attributes.find( attributeName ) == edge.attributes.end() )
  {
    return ConversionError{ fmt::format(
        "Missing required attribute: {} in Edge: {}", attributeName, edge.source + " -> " + edge.target ) };
  }
  return std::nullopt; // Attribute is present
}

/**
 * @brief Expect that a list of attributes are present in a vertex.
 * @param vertex The vertex to check.
 * @param attributes The list of attribute names to look for.
 * @return A vector of ConversionErrors for any missing attributes.
 */
template <class T>
inline std::vector<ConversionError> ExpectAttributesToBePresent( const T&                        vertex,
                                                                 const std::vector<std::string>& attributes )
{
  std::vector<ConversionError> errors;
  for ( const auto& attr : attributes )
  {
    auto error = ExpectAttributeToBePresent( vertex, attr );
    if ( error.has_value() )
    {
      errors.push_back( *error );
    }
  }
  return errors;
}
} // namespace carpeDM