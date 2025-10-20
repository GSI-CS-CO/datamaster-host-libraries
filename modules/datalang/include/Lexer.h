#pragma once

#include "datalang_export.h"

#include <cstdint>
#include <string>
#include <variant>
#include <span>
#include <optional>

namespace datalang
{

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

struct EndOfStreamToken
{
};

struct InvalidToken
{
  std::string value;
};

using Token = std::variant<ConstantToken, VariableToken, OperatorToken, ScopeToken, EndOfStreamToken, InvalidToken>;
bool operator==( const Token& lhs, const Token& rhs );

class DATALANG_EXPORT Lexer
{
public:
  Lexer( std::span<const char> input );
  Token getNextToken();

private:
  std::span<const char> m_input;
  size_t      m_currentPosition;
};
} // namespace datalang