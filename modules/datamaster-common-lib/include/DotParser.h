#pragma once

#include "ScheduleGraph.h"

#include <optional>
#include <unordered_map>
#include <variant>

namespace carpeDM
{

struct DotGraphVertex
{
  std::string                                  id;
  std::unordered_map<std::string, std::string> attributes;
};

struct DotGraphEdge
{
  std::string                                  source;
  std::string                                  target;
  std::unordered_map<std::string, std::string> attributes;
};

/**
 * @brief Properties of a DOT graph.
 *
 * This struct holds the properties of a DOT graph, including its name,
 * whether it is strict, and any custom attributes associated with the graph.
 *
 */
struct GraphProperties
{
  // Name of the graph
  std::string name;
  // Whether the graph is strict
  bool isStrict = false;
  // Custom attributes of the graph
  std::unordered_map<std::string, std::string> customAttributes;
};

/**
 * @brief Representation of a DOT graph.
 */
struct DotGraph
{
  // Properties of the graph
  GraphProperties properties;
  // Vertices in the graph
  std::unordered_map<std::string, DotGraphVertex> vertices;
  // Edges in the graph
  std::vector<DotGraphEdge> edges;
};

/**
 * Parser for DOT graphs.
 */
class DotGraphParser
{
public:
  /**
   * Parses a DOT graph from a string view.
   * @param fileBuffer The string view containing the DOT graph.
   * @return A variant containing either a DotGraph or a ParsingError.
   *         If parsing is successful, it returns a DotGraph.
   *         If parsing fails, it returns a ParsingError with an error message.
   */
  [[nodiscard]] std::variant<DotGraph, ParsingError> parseGraph( std::string_view fileBuffer );

private:
  /**
   * @brief Parse a list of statements from the string view.
   * @param g The DotGraph to store the parsed statements.
   * @param currentPosition The current position in the string.
   * @param endPosition The end position of the string.
   * @return The position of the next token after the statement list, or a ParsingError if parsing fails.
   *
   * This method parses a list of statements from the string view. It expects the statement list to be enclosed in
   * curly braces `{}`. Each statement can be separated by semicolons, which are optional and are ignored. If the
   * statement list is not well-formed, it returns a ParsingError with an appropriate message.
   */
  [[nodiscard]] std::optional<ParsingError>
  ParseStatementList( DotGraph& g, const char* currentPosition, const char* const endPosition );

  /**
   * @brief Parse a statement from the string view.
   * @param g The DotGraph to store the parsed statement.
   * @param currentPosition The current position in the string.
   * @param endPosition The end position of the string.
   * @return The position of the next token after the statement, or a ParsingError if parsing fails.
   *
   * This method parses a single statement from the string view. It can handle node statements,
   * edge statements, attribute statements, and subgraph statements. If the statement is not well-formed,
   * it returns a ParsingError with an appropriate message.
   *
   */
  [[nodiscard]] std::variant<const char*, ParsingError>
  ParseStatement( DotGraph& g, const char* currentPosition, const char* const endPosition );

  [[nodiscard]] std::variant<const char*, ParsingError>
  ParseEdge( DotGraph& g, const std::string& lhId, const char* currentPosition, const char* const endPosition );

  /**
   * @brief Parse a port from the string view.
   * @param port The string to store the parsed port.
   * @param compassPt The string to store the parsed compass point (if any).
   * @param currentPosition The current position in the string.
   * @param endPosition The end position of the string.
   * @return The position of the next token after the port, or a ParsingError if parsing fails.
   *
   * This method parses a port in the format `:port` or `:port:compass_pt` or just `:compass_pt`.
   * It expects the port to start with a `:` character. If a compass point is specified,
   * it is expected to be in the format `:compass_pt`, where compass_pt can be one of the predefined compass points
   * (n, ne, e, se, s, sw, w, nw, c, _).
   * If the port is not well-formed,
   * it returns a ParsingError with an appropriate message.
   */
  [[nodiscard]] std::variant<const char*, ParsingError>
  ParsePort( std::string& port, std::string& compassPt, const char* currentPosition, const char* const endPosition );

  /**
   * @brief Parse the right-hand side of an edge statement.
   * @param targetId The string to store the target ID.
   * @param currentPosition The current position in the string.
   * @param endPosition The end position of the string.
   * @return The position of the next token after the edgeRhs, or a ParsingError if parsing fails.
   *
   * This method parses the right-hand side of an edge statement in the format `-> target_id` or `-- target_id`.
   * It expects the edgeRhs to start with either `->` or `--`, followed by a target ID.
   * If the edgeRhs is not well-formed, it returns a ParsingError with an appropriate message.
   */
  [[nodiscard]] std::variant<const char*, ParsingError>
  ParseEdgeRhs( std::string& targetId, const char* currentPosition, const char* const endPosition );

private:
  // The dot graph being parsed
  DotGraph m_dotGraph;

  // State of default node and edge attributes
  std::unordered_map<std::string, std::string> m_nodeAttributes;
  std::unordered_map<std::string, std::string> m_edgeAttributes;
};

} // namespace carpeDM