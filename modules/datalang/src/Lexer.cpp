#include "Lexer.h"

using datalang::Lexer;
using datalang::Token;

namespace
{
bool isWhitespace( char c )
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool skipWhitespace( std::span<const char> input, size_t& position )
{
  while ( position < input.size() && isWhitespace( input[position] ) )
  {
    ++position;
  }
  return position < input.size();
}

bool isNumber( char c )
{
  return c >= '0' && c <= '9';
}

bool isAlpha( char c )
{
  return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || c == '_';
}

bool isAlphaNumeric( char c )
{
  return isAlpha( c ) || isNumber( c );
}

bool isOperatorChar( char c )
{
  return c == '+' || c == '-' || c == '*' || c == '/';
}

} // namespace

namespace datalang
{
bool operator==( const Token& lhs, const Token& rhs )
{
  return std::visit(
      []( auto&& l, auto&& r )
      {
        using lT = std::decay_t<decltype( l )>;
        using rT = std::decay_t<decltype( r )>;

        if constexpr ( std::is_same_v<lT, ConstantToken> && std::is_same_v<rT, ConstantToken> )
          return l.value == r.value;
        else if constexpr ( std::is_same_v<lT, VariableToken> && std::is_same_v<rT, VariableToken> )
          return l.name == r.name;
        else if constexpr ( std::is_same_v<lT, OperatorToken> && std::is_same_v<rT, OperatorToken> )
          return l.op == r.op;
        else if constexpr ( std::is_same_v<lT, ScopeToken> && std::is_same_v<rT, ScopeToken> )
          return l.opens == r.opens;
        else if constexpr ( std::is_same_v<lT, EndOfStreamToken> && std::is_same_v<rT, EndOfStreamToken> )
          return true;
        else if constexpr ( std::is_same_v<lT, InvalidToken> && std::is_same_v<rT, InvalidToken> )
          return l.value == r.value;
        else
          return false;
      },
      lhs,
      rhs );
}
} // namespace datalang

Lexer::Lexer( std::span<const char> input )
    : m_input( input )
    , m_currentPosition( 0 )
{
}

Token Lexer::getNextToken()
{
  if ( !skipWhitespace( m_input, m_currentPosition ) )
  {
    return EndOfStreamToken{};
  }

  if ( isNumber( m_input[m_currentPosition] ) )
  {
    int32_t value = 0;
    while ( m_currentPosition < m_input.size() && isNumber( m_input[m_currentPosition] ) )
    {
      value = value * 10 + ( m_input[m_currentPosition] - '0' );
      ++m_currentPosition;
    }
    return ConstantToken{ value };
  }
  else if ( isAlpha( m_input[m_currentPosition] ) )
  {
    std::string name;
    while ( m_currentPosition < m_input.size() && isAlphaNumeric( m_input[m_currentPosition] ) )
    {
      name += m_input[m_currentPosition];
      ++m_currentPosition;
    }
    return VariableToken{ name };
  }
  else if ( isOperatorChar( m_input[m_currentPosition] ) )
  {
    char op = m_input[m_currentPosition];
    ++m_currentPosition;
    switch ( op )
    {
    case '+':
      return OperatorToken{ OperatorType::Add };
    case '-':
      return OperatorToken{ OperatorType::Subtract };
    case '*':
      return OperatorToken{ OperatorType::Multiply };
    case '/':
      return OperatorToken{ OperatorType::Divide };
    default:
      // This should never happen if isOperatorChar is correct, but handle it gracefully
      return InvalidToken{ std::string( 1, op ) };
    }
  }
  else if ( m_input[m_currentPosition] == '(' )
  {
    ++m_currentPosition;
    return ScopeToken{ true };
  }
  else if ( m_input[m_currentPosition] == ')' )
  {
    ++m_currentPosition;
    return ScopeToken{ false };
  }
  else
  {
    std::string invalidChar;
    while ( !isWhitespace( m_input[m_currentPosition] ) && m_currentPosition < m_input.size() )
    {
      invalidChar += m_input[m_currentPosition];
      ++m_currentPosition;
    }
    return InvalidToken{ invalidChar };
  }

  return EndOfStreamToken{};
}
