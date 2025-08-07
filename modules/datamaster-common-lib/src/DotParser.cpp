#include "DotParser.h"

#include <cstring>
#include <optional>
#include <variant>

#include "ScheduleGraph.h"

using carpeDM::DotGraph;
using carpeDM::DotGraphParser;
using carpeDM::GraphProperties;
using carpeDM::ParsingError;
using carpeDM::ScheduleGraph;

/**
 * graph 	: 	[ strict ] (graph | digraph) [ ID ] '{' stmt_list '}'
 * stmt_list 	: 	[ stmt [ ';' ] stmt_list ]
 * stmt 	: 	node_stmt
 * 	| 	edge_stmtParseGraph
 * 	| 	attr_stmt
 * 	| 	ID '=' ID
 * 	| 	subgraph
 * attr_stmt 	: 	(graph | node | edge) attr_list
 * attr_list 	: 	'[' [ a_list ] ']' [ attr_list ]
 * a_list 	: 	ID '=' ID [ (';' | ',') ] [ a_list ]
 * edge_stmt 	: 	(node_id | subgraph) edgeRHS [ attr_list ]
 * edgeRHS 	: 	edgeop (node_id | subgraph) [ edgeRHS ]
 * node_stmt 	: 	node_id [ attr_list ]
 * node_id 	: 	ID [ port ]
 * port 	: 	':' ID [ ':' compass_pt ]
 * 	| 	':' compass_pt
 * subgraph 	: 	[ subgraph [ ID ] ] '{' stmt_list '}'
 * compass_pt 	: 	n | ne | e | se | s | sw | w | nw | c | _
 *
 */

namespace
{

/**
 * @brief Enum class for terminal symbols in the DOT grammar.
 * This enum class defines the terminal symbols used in the DOT grammar.
 */
enum class Terminal
{
  strict,
  graph,
  digraph,
  node,
  edge,
  subgraph,
};

/**
 * @brief Skip whitespace characters and semicolons or commas.
 * @param currentPosition The current position in the string.
 * @param endPosition The end position of the string.
 * @return The position after skipping whitespace and semicolons or commas.
 *
 * This function iterates through the string, skipping any whitespace characters,
 * semicolons, or commas until it reaches a non-whitespace character or the end of
 * the string. We skip semicolons and commas as they are optional in DOT syntax
 * and can be used to separate statements, but they do not affect the parsing of IDs or attributes.
 */
[[nodiscard]] const char* SkipWhiteSpace( const char* currentPosition, const char* const endPosition )
{
  while ( currentPosition < endPosition &&
          ( std::isspace( *currentPosition ) || *currentPosition == ';' || *currentPosition == ',' ) )
  {
    ++currentPosition;
  }
  return currentPosition;
}

/**
 * @brief Find a specific character in the string view.
 * @param currentPosition The current position in the string.
 * @param endPosition The end position of the string.
 * @param character The character to find.
 * @return The position of the character if found, or the end position if not found.
 *
 * This function searches for a specific character in the string view and returns
 * the position of that character. If the character is not found, it returns the
 * end position of the string view.
 */
[[nodiscard]] const char*
FindCharacterAhead( const char* currentPosition, const char* const endPosition, char character )
{
  while ( currentPosition < endPosition && *currentPosition != character )
  {
    ++currentPosition;
  }
  return currentPosition;
}

/**
 * @brief Parse an ID from the string view.
 * @param id The string to store the parsed ID.
 * @param currentPosition The current position in the string.
 * @param endPosition The end position of the string.
 * @return The position of the next token after the ID, or a ParsingError if parsing fails.
 *
 * This method attempts to parse an ID from the string view.
 * It supports both quoted and unquoted IDs. If the ID is quoted, it expects
 * it to be enclosed in double quotes. If the ID is unquoted, it can consist
 * of alphanumeric characters, underscores, and dots. If the ID is not found,
 * it returns a ParsingError with an appropriate message.
 *
 */
[[nodiscard]] std::variant<const char*, ParsingError>
ParseID( std::string& id, const char* currentPosition, const char* const endPosition )
{
  if ( currentPosition >= endPosition )
  {
    return ParsingError( "Unexpected end of input while parsing ID" );
  }

  if ( *currentPosition == '"' )
  {
    auto idEnd = FindCharacterAhead( currentPosition + 1, endPosition, '"' );
    if ( idEnd == endPosition )
    {
      return ParsingError( "Unterminated string literal for ID" );
    }
    id.assign( currentPosition + 1, idEnd - currentPosition - 1 );
    return SkipWhiteSpace( idEnd + 1, endPosition ); // Return the position after the ID
  }

  // Read ID
  const char* idStart = currentPosition;
  while ( currentPosition < endPosition &&
          ( std::isalnum( *currentPosition ) || *currentPosition == '_' || *currentPosition == '.' ) )
  {
    ++currentPosition;
  }
  if ( idStart == currentPosition )
  {
    return ParsingError( "Expected ID" );
  }
  id.assign( idStart, currentPosition - idStart );
  return SkipWhiteSpace( currentPosition, endPosition ); // Return the position after the ID
}

/**
 * @brief Parse an attribute list from the string view.
 * @param currentPosition The current position in the string.
 * @param endPosition The end position of the string.
 * @param attributes The map to store the parsed attributes.
 * @return The position of the next token after the attribute list, or a ParsingError if parsing fails.
 *
 * This method parses an attribute list in the format `[ key = value ; key = value ; ... ]`.
 * It expects the attribute list to start with a `[` character and end with a `]` character.
 * Each attribute is expected to be in the format `key = value`, where both key and value are IDs.
 * If the attribute list is not well-formed, it returns a ParsingError with an appropriate message.
 *
 * We ignore semicolons and commas as they are optional in DOT syntax and can be used to separate attributes, but they
 * are not required.
 */
[[nodiscard]] std::variant<const char*, ParsingError>
ParseAttributeList( const char*                                   currentPosition,
                    const char* const                             endPosition,
                    std::unordered_map<std::string, std::string>& attributes )
{
  if ( currentPosition >= endPosition )
  {
    return ParsingError( "Unexpected end of input while parsing attribute list" );
  }

  if ( *currentPosition != '[' )
  {
    return ParsingError( "Expected '[' at the start of attribute list" );
  }
  ++currentPosition; // Skip '['
  currentPosition = SkipWhiteSpace( currentPosition, endPosition );

  while ( currentPosition < endPosition && *currentPosition != ']' )
  {
    std::string key, value;
    auto        parseIDResult = ParseID( key, currentPosition, endPosition );

    // we always store the key in lowercase to ensure case-insensitivity later
    std::transform( key.begin(), key.end(), key.begin(), ::tolower );

    if ( std::holds_alternative<ParsingError>( parseIDResult ) )
    {
      return std::get<ParsingError>( parseIDResult );
    }
    currentPosition = std::get<const char*>( parseIDResult );

    if ( *currentPosition != '=' )
    {
      return ParsingError( "Expected '=' after attribute key" );
    }
    ++currentPosition; // Skip '='
    currentPosition = SkipWhiteSpace( currentPosition, endPosition );

    parseIDResult = ParseID( value, currentPosition, endPosition );
    if ( std::holds_alternative<ParsingError>( parseIDResult ) )
    {
      return std::get<ParsingError>( parseIDResult );
    }
    currentPosition = std::get<const char*>( parseIDResult );

    attributes[key] = value; // Store the attribute
  }

  if ( *currentPosition != ']' )
  {
    return ParsingError( "Expected ']' at the end of attribute list" );
  }
  ++currentPosition; // Skip ']'

  return SkipWhiteSpace( currentPosition, endPosition ); // Return the position after the attribute list
}

} // namespace

[[nodiscard]] std::variant<const char*, ParsingError>
DotGraphParser::ParseEdgeRhs( std::string& targetId, const char* currentPosition, const char* const endPosition )
{
  if ( *currentPosition != '-' )
  {
    return ParsingError( "Expected '-' at the start of edgeRhs statement" );
  }

  ++currentPosition; // Skip '-'
  if ( currentPosition >= endPosition )
  {
    return ParsingError( "Unexpected end of input after '-'" );
  }

  if ( *currentPosition != '>' && *currentPosition != '-' )
  {
    return ParsingError( "Expected '--' or '->' in edgeRhs statement" );
  }

  bool isDirected = ( *currentPosition == '>' ); // we do not care for now
  ++currentPosition;                             // Skip '>' or '-'

  currentPosition          = SkipWhiteSpace( currentPosition, endPosition );
  auto parseTargetIDResult = ParseID( targetId, currentPosition, endPosition );
  if ( std::holds_alternative<ParsingError>( parseTargetIDResult ) )
  {
    return std::get<ParsingError>( parseTargetIDResult );
  }
  return std::get<const char*>( parseTargetIDResult );
}

[[nodiscard]] std::variant<const char*, ParsingError> DotGraphParser::ParsePort( std::string&      port,
                                                                                 std::string&      compassPt,
                                                                                 const char*       currentPosition,
                                                                                 const char* const endPosition )
{
  if ( currentPosition >= endPosition || *currentPosition != ':' )
  {
    return ParsingError( "Expected ':' at the start of port" );
  }
  ++currentPosition; // Skip ':'

  auto parseIDResult = ParseID( port, currentPosition, endPosition );
  if ( std::holds_alternative<ParsingError>( parseIDResult ) )
  {
    return std::get<ParsingError>( parseIDResult );
  }
  currentPosition = std::get<const char*>( parseIDResult );

  if ( currentPosition < endPosition && *currentPosition == ':' )
  {
    ++currentPosition; // Skip ':'
    parseIDResult = ParseID( compassPt, currentPosition, endPosition );
    if ( std::holds_alternative<ParsingError>( parseIDResult ) )
    {
      return std::get<ParsingError>( parseIDResult );
    }
    currentPosition = std::get<const char*>( parseIDResult );
  }
  else
  {
    compassPt.clear(); // No compass point specified
  }

  return SkipWhiteSpace( currentPosition, endPosition ); // Return the token after the port
}

[[nodiscard]] std::variant<const char*, ParsingError> DotGraphParser::ParseEdge( DotGraph&          g,
                                                                                 const std::string& lhId,
                                                                                 const char*        currentPosition,
                                                                                 const char* const  endPosition )
{
  std::string targetId;
  auto        parseEdgeRhsResult = ParseEdgeRhs( targetId, currentPosition, endPosition );
  if ( std::holds_alternative<ParsingError>( parseEdgeRhsResult ) )
  {
    return std::get<ParsingError>( parseEdgeRhsResult );
  }
  currentPosition = std::get<const char*>( parseEdgeRhsResult );

  std::unordered_map<std::string, std::string> edgeAttributes = m_edgeAttributes; // Copy default edge attributes
  if ( *currentPosition == '[' )
  {
    std::unordered_map<std::string, std::string> additionalAttributes;
    auto parseAttrListResult = ParseAttributeList( currentPosition, endPosition, additionalAttributes );
    for ( auto&& attrib : additionalAttributes )
    {
      edgeAttributes[attrib.first] = attrib.second; // Merge additional attributes into edge attributes
    }

    if ( std::holds_alternative<ParsingError>( parseAttrListResult ) )
    {
      return std::get<ParsingError>( parseAttrListResult );
    }
    currentPosition = std::get<const char*>( parseAttrListResult );
  }

  g.edges.push_back( { lhId, targetId, edgeAttributes } );
  if ( g.vertices.find( lhId ) == g.vertices.end() )
  {
    // Add the source node to the graph if it does not exist
    g.vertices.insert( { lhId, { lhId, m_nodeAttributes } } );
  }
  if ( g.vertices.find( targetId ) == g.vertices.end() )
  {
    // Add the target node to the graph if it does not exist
    g.vertices.insert( { targetId, { targetId, m_nodeAttributes } } );
  }

  return currentPosition;
}

[[nodiscard]] std::variant<const char*, ParsingError>
DotGraphParser::ParseStatement( DotGraph& g, const char* currentPosition, const char* const endPosition )
{
  std::string                                  id;
  std::string                                  port;
  std::string                                  compassPt;
  std::unordered_map<std::string, std::string> attributes;

  auto parseIDResult = ParseID( id, currentPosition, endPosition );
  if ( std::holds_alternative<ParsingError>( parseIDResult ) )
  {
    return std::get<ParsingError>( parseIDResult );
  }

  currentPosition = std::get<const char*>( parseIDResult );

  // check if id is either node or edge - then we expect this to be a attribute statement
  auto maybeTerminal = magic_enum::enum_cast<Terminal>( id, magic_enum::case_insensitive );

  if ( maybeTerminal.has_value() &&
       ( maybeTerminal.value() == Terminal::node || maybeTerminal.value() == Terminal::edge ||
         maybeTerminal.value() == Terminal::graph ) )
  {
    auto parseAttrListResult = ParseAttributeList( currentPosition, endPosition, attributes );
    if ( std::holds_alternative<ParsingError>( parseAttrListResult ) )
    {
      return std::get<ParsingError>( parseAttrListResult );
    }
    currentPosition = std::get<const char*>( parseAttrListResult );
    if ( maybeTerminal.value() == Terminal::node )
    {
      m_nodeAttributes = attributes;
    }
    else if ( maybeTerminal.value() == Terminal::edge )
    {
      m_edgeAttributes = attributes;
    }
    else if ( maybeTerminal.value() == Terminal::graph )
    {
      g.properties.customAttributes = attributes; // Set graph custom attributes
    }
    else if ( maybeTerminal.value() == Terminal::subgraph )
    {
      return ParsingError( "Subgraph statements are not supported in this context" );
    }

    return currentPosition;
  }

  // If a port is specified, it will be in the form of ":port" or ":port:compass_pt" or just ":compass_pt"
  if ( *currentPosition == ':' )
  {
    auto parsePortResult = ParsePort( port, compassPt, currentPosition, endPosition );
    if ( std::holds_alternative<ParsingError>( parsePortResult ) )
    {
      return std::get<ParsingError>( parsePortResult );
    }
    currentPosition = std::get<const char*>( parsePortResult );
  }

  if ( *currentPosition == '-' )
  {
    auto parseEdgeResult = ParseEdge( g, id, currentPosition, endPosition );
    if ( std::holds_alternative<ParsingError>( parseEdgeResult ) )
    {
      return std::get<ParsingError>( parseEdgeResult );
    }
    return std::get<const char*>( parseEdgeResult );
  }

  if ( *currentPosition == '=' )
  {
    // This is an assignment statement, e.g., "ID = ID"
    // What does it do?
    // We will ignore it for now, but we could handle it if needed.
    ++currentPosition; // Skip '='
    currentPosition = SkipWhiteSpace( currentPosition, endPosition );
    std::string targetId;
    auto        parseTargetIDResult = ParseID( targetId, currentPosition, endPosition );
    if ( std::holds_alternative<ParsingError>( parseTargetIDResult ) )
    {
      return std::get<ParsingError>( parseTargetIDResult );
    }
    return std::get<const char*>( parseTargetIDResult );
  }

  std::unordered_map<std::string, std::string> additionalAttributes;
  if ( *currentPosition == '[' )
  {
    auto parseAttrListResult = ParseAttributeList( currentPosition, endPosition, additionalAttributes );

    if ( std::holds_alternative<ParsingError>( parseAttrListResult ) )
    {
      return std::get<ParsingError>( parseAttrListResult );
    }
    currentPosition = std::get<const char*>( parseAttrListResult );
  }

  if ( g.vertices.count( id ) != 0 )
  {
    // Extend or update existing node attributes
    for ( auto&& attrib : additionalAttributes )
    {
      g.vertices[id].attributes[attrib.first] = attrib.second; // Update existing node attributes
    }
  }
  else
  {
    auto nodeAttributes = m_nodeAttributes; // Copy default node attributes
    for ( auto&& attrib : additionalAttributes )
    {
      nodeAttributes[attrib.first] = attrib.second; // Merge additional attributes into node attributes
    }
    g.vertices.insert( { id, { id, std::move( nodeAttributes ) } } ); // Add the node to the graph
  }

  return currentPosition;
}

[[nodiscard]] std::optional<ParsingError>
DotGraphParser::ParseStatementList( DotGraph& g, const char* currentPosition, const char* const endPosition )
{
  while ( currentPosition < endPosition )
  {
    if ( *currentPosition == '}' )
    {
      ++currentPosition; // Skip '}'
      break;
    }

    currentPosition           = SkipWhiteSpace( currentPosition, endPosition );
    auto parseStatementResult = ParseStatement( g, currentPosition, endPosition );
    if ( std::holds_alternative<ParsingError>( parseStatementResult ) )
    {
      return std::get<ParsingError>( parseStatementResult );
    }
    auto nextPosition = std::get<const char*>( parseStatementResult );
    if ( nextPosition == currentPosition )
    {
      return ParsingError( "Unable to parse statement" );
    }
    currentPosition = nextPosition;
  }
  return std::nullopt;
}

[[nodiscard]] std::variant<DotGraph, ParsingError> DotGraphParser::parseGraph( std::string_view fileBuffer )
{
  DotGraph    g;
  std::string graphName = "";
  bool        isStrict  = false;

  if ( fileBuffer.empty() )
  {
    return ParsingError( "File buffer is empty" );
  }

  const char* currentPosition = fileBuffer.data();
  const char* endPosition     = fileBuffer.data() + fileBuffer.size();

  std::string token;
  currentPosition       = SkipWhiteSpace( currentPosition, endPosition );
  auto tokenParseResult = ParseID( token, currentPosition, endPosition );
  if ( std::holds_alternative<ParsingError>( tokenParseResult ) )
  {
    return std::get<ParsingError>( tokenParseResult );
  }
  currentPosition = std::get<const char*>( tokenParseResult );
  currentPosition = SkipWhiteSpace( currentPosition, endPosition );

  auto potentialTerminal = magic_enum::enum_cast<Terminal>( token, magic_enum::case_insensitive );
  if ( potentialTerminal.has_value() && potentialTerminal.value() == Terminal::strict )
  {
    g.properties.isStrict = true;
    tokenParseResult      = ParseID( token, currentPosition, endPosition );
    if ( std::holds_alternative<ParsingError>( tokenParseResult ) )
    {
      return std::get<ParsingError>( tokenParseResult );
    }
    currentPosition   = std::get<const char*>( tokenParseResult );
    currentPosition   = SkipWhiteSpace( currentPosition, endPosition );
    potentialTerminal = magic_enum::enum_cast<Terminal>( token, magic_enum::case_insensitive );
  }
  else
  {
    g.properties.isStrict = false;
  }

  if ( potentialTerminal.has_value() &&
       ( potentialTerminal.value() == Terminal::graph || potentialTerminal.value() == Terminal::digraph ) )
  {
    // Ignore for now
  }
  else
  {
    return ParsingError( "Expected 'graph' or 'digraph' after graph name" );
  }

  if ( *currentPosition != '{' )
  {
    std::string graphName;
    auto        graphNameParseResult = ParseID( graphName, currentPosition, endPosition );
    if ( std::holds_alternative<ParsingError>( graphNameParseResult ) )
    {
      return std::get<ParsingError>( graphNameParseResult );
    }
    g.properties.name = std::move( graphName );
    currentPosition   = std::get<const char*>( graphNameParseResult );
    currentPosition   = SkipWhiteSpace( currentPosition, endPosition );
  }

  if ( *currentPosition != '{' )
  {
    return ParsingError( "Expected '{' to start statement list" );
  }

  currentPosition++;

  currentPosition               = SkipWhiteSpace( currentPosition, endPosition );
  auto parseStatementListResult = ParseStatementList( g, currentPosition, endPosition );
  if ( parseStatementListResult.has_value() )
  {
    return *parseStatementListResult; // Return the parsing error if any
  }

  return g;
}
