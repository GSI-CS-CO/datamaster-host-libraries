#pragma once

#include "datalang_export.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

// Terminals
// +, - , *, /, %, min, max, <<, >>, &, |, ^, ~, ==, !=, <, <=, >, >=, if, else, (, ), {, }, identifiers, constants

namespace datalang
{

/**
 * Enumeration of operator types.
 */
enum class DATALANG_EXPORT OperatorType
{
  Add,
  Subtract,
  Multiply,
  Divide,
  Modulus,
  LSH,
  RSH,
  BitAnd,
  BitOr,
  BitXor,
  BitNot,
  Equal,
  NotEqual,
  Or,
  And,
  LessThan,
  LessThanOrEqual,
  GreaterThan,
  GreaterThanOrEqual,
  Min,
  Max
};

/**
 * Enumeration of statement types.
 */
enum class DATALANG_EXPORT StatementType
{
  If,
  Else
};

/**
 * Constant token representing an integer constant.
 */
struct ConstantToken
{
  int32_t value;
};

/**
 * Variable token representing an identifier.
 */
struct VariableToken
{
  std::string name;
};

/**
 * Operator token representing an operator.
 */
struct OperatorToken
{
  OperatorType op; // one of +, -, *, /
};

/**
 * Scope token representing opening or closing of a scope.
 */
struct ScopeToken
{
  bool opens; // true if '{', false if '}'
};

/**
 * Grouping token representing opening or closing of a grouping.
 */
struct GroupingToken
{
  bool opens; // true if '(', false if ')'
};

/**
 * End of stream token representing the end of the input.
 */
struct EndOfStreamToken
{
};

/**
 * Invalid token representing an unrecognized sequence.
 */
struct InvalidToken
{
  std::string value;
};

/**
 * Statement token representing a control statement.
 */
struct StatementToken
{
  StatementType type;
};

/**
 * End of statement token representing the end of a statement.
 */
struct EndOfStatementToken
{
};

/**
 * MinMax token representing 'min' or 'max' function.
 */
struct MinMaxToken
{
  bool isMin; // true if 'min', false if 'max'
};

/**
 * Statement separator token representing a separator between statements.
 */
struct StatementSeparatorToken
{
};

/**
 * Variant type representing any token.
 */
using Token = std::variant<ConstantToken,
                           VariableToken,
                           OperatorToken,
                           ScopeToken,
                           GroupingToken,
                           EndOfStreamToken,
                           InvalidToken,
                           StatementToken,
                           EndOfStatementToken,
                           MinMaxToken,
                           StatementSeparatorToken>;

/**
 * Type alias for a stream of tokens.
 */
using TokenStream = std::vector<Token>;

/**
 * Equality operator for tokens.
 */
bool operator==( const Token& lhs, const Token& rhs );

/**
 * Lexer class for tokenizing input strings.
 */
class DATALANG_EXPORT Lexer
{
public:
  /**
   * Constructor initializing the lexer with input.
   *
   * @param input The input string to be tokenized.
   */
  Lexer( std::span<const char> input );

  /**
   * Retrieves the next token from the input.
   *
   * @return The next token.
   */
  Token getNextToken();

private:
  // Input string as a span of characters
  std::span<const char> m_input;

  // Current position in the input
  size_t m_currentPosition;
};
} // namespace datalang