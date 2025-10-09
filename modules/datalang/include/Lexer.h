#pragma once

#include "datalang_export.h"

#include <cstdint>
#include <span>

namespace datalang
{
enum class DATALANG_EXPORT TokenType
{
  Constant, // a constant number, example: 42
  Variable, // a variable name, example: myVar
  Plus,     // +
  Minus,    // -
  Asterisk, // *
  Slash,    // /
  LParen,   // (
  RParen,   // )
  End       // end of input
};

enum class DATALANG_EXPORT OperatorType
{
  Add,
  Subtract,
  Multiply,
  Divide
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
  bool opens; // true if '(', false if ')'
};

using Token = std::variant<ConstantToken, VariableToken, OperatorToken, ScopeToken>;

class DATALANG_EXPORT Lexer
{
public:
  Lexer( const std::span<const char>& input );
  Token getNextToken();

private:
  std::span<const char> input;
  size_t                currentPosition;
};
} // namespace datalang