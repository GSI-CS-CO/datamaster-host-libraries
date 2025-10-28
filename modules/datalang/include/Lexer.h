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

enum class DATALANG_EXPORT StatementType
{
  If,
  Else
};

struct ConstantToken
{
  int32_t value;
};

struct VariableToken
{
  std::string name;
};

struct OperatorToken
{
  OperatorType op; // one of +, -, *, /
};

struct ScopeToken
{
  bool opens; // true if '{', false if '}'
};

struct GroupingToken
{
  bool opens; // true if '(', false if ')'
};

struct EndOfStreamToken
{
};

struct InvalidToken
{
  std::string value;
};

struct StatementToken
{
  StatementType type;
};

struct EndOfStatementToken
{
};

struct MinMaxToken
{
  bool isMin; // true if 'min', false if 'max'
};

struct StatementSeparatorToken
{
};

using Token       = std::variant<ConstantToken,
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
using TokenStream = std::vector<Token>;

bool operator==( const Token& lhs, const Token& rhs );

class DATALANG_EXPORT Lexer
{
public:
  Lexer( std::span<const char> input );
  Token getNextToken();

private:
  std::span<const char> m_input;
  size_t                m_currentPosition;
};
} // namespace datalang