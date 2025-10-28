#include "Lexer.h"

using datalang::Lexer;
using datalang::Token;

namespace
{

constexpr char OPERATOR_START_TOKENS[] = { '+', '-', '*', '/', '%', '&', '|', '^', '~', '<', '>', '=', '~', ';' };

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
  return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
}

bool isAlphaNumeric( char c )
{
  return isAlpha( c ) || isNumber( c );
}

bool isOperatorStartChar( char c )
{
  for ( char op : OPERATOR_START_TOKENS )
  {
    if ( c == op )
      return true;
  }
  return false;
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
        else if constexpr ( std::is_same_v<lT, StatementToken> && std::is_same_v<rT, StatementToken> )
          return l.type == r.type;
        else if constexpr ( std::is_same_v<lT, EndOfStatementToken> && std::is_same_v<rT, EndOfStatementToken> )
          return true;
        else if constexpr ( std::is_same_v<lT, MinMaxToken> && std::is_same_v<rT, MinMaxToken> )
          return l.isMin == r.isMin;
        else if constexpr ( std::is_same_v<lT, StatementSeparatorToken> && std::is_same_v<rT, StatementSeparatorToken> )
          return true;
        else if constexpr ( std::is_same_v<lT, GroupingToken> && std::is_same_v<rT, GroupingToken> )
          return l.opens == r.opens;
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

  if ( isAlpha( m_input[m_currentPosition] ) )
  {

    if ( m_currentPosition + 3 <= m_input.size() && m_input[m_currentPosition] == 'm' )
    {
      auto subsection = m_input.subspan( m_currentPosition, 3 );

      if ( subsection[1] == 'i' && subsection[2] == 'n' )
      {
        m_currentPosition += 3;
        return MinMaxToken{ true };
      }

      if ( subsection[1] == 'a' && subsection[2] == 'x' )
      {
        m_currentPosition += 3;
        return MinMaxToken{ false };
      }
    }

    if ( m_currentPosition + 2 <= m_input.size() && m_input[m_currentPosition] == 'i' &&
         m_input[m_currentPosition + 1] == 'f' )
    {
      m_currentPosition += 2;
      return StatementToken{ StatementType::If };
    }

    if ( m_currentPosition + 4 <= m_input.size() && m_input[m_currentPosition] == 'e' &&
         m_input[m_currentPosition + 1] == 'l' && m_input[m_currentPosition + 2] == 's' &&
         m_input[m_currentPosition + 3] == 'e' )
    {
      m_currentPosition += 4;
      return StatementToken{ StatementType::Else };
    }

    std::string name;
    while ( m_currentPosition < m_input.size() && isAlphaNumeric( m_input[m_currentPosition] ) )
    {
      name += m_input[m_currentPosition];
      ++m_currentPosition;
    }
    return VariableToken{ name };
  }

  if ( isOperatorStartChar( m_input[m_currentPosition] ) )
  {
    char op = m_input[m_currentPosition];
    switch ( op )
    {
    case '+':
      ++m_currentPosition;
      return OperatorToken{ OperatorType::Add };
    case '-':
      ++m_currentPosition;
      return OperatorToken{ OperatorType::Subtract };
    case '*':
      ++m_currentPosition;
      return OperatorToken{ OperatorType::Multiply };
    case '/':
      ++m_currentPosition;
      return OperatorToken{ OperatorType::Divide };
    case '%':
      ++m_currentPosition;
      return OperatorToken{ OperatorType::Modulus };
    case '<':
      if ( m_currentPosition + 1 < m_input.size() && m_input[m_currentPosition + 1] == '<' )
      {
        m_currentPosition += 2;
        return OperatorToken{ OperatorType::LSH };
      }

      if ( m_currentPosition + 1 < m_input.size() && m_input[m_currentPosition + 1] == '=' )
      {
        m_currentPosition += 2;
        return OperatorToken{ OperatorType::LessThanOrEqual };
      }

      ++m_currentPosition;
      return OperatorToken{ OperatorType::LessThan };
    case '>':
      if ( m_currentPosition + 1 < m_input.size() && m_input[m_currentPosition + 1] == '>' )
      {
        m_currentPosition += 2;
        return OperatorToken{ OperatorType::RSH };
      }

      if ( m_currentPosition + 1 < m_input.size() && m_input[m_currentPosition + 1] == '=' )
      {
        m_currentPosition += 2;
        return OperatorToken{ OperatorType::GreaterThanOrEqual };
      }

      ++m_currentPosition;
      return OperatorToken{ OperatorType::GreaterThan };
    case '&':
      if ( m_currentPosition + 1 < m_input.size() && m_input[m_currentPosition + 1] == '&' )
      {
        m_currentPosition += 2;
        return OperatorToken{ OperatorType::And };
      }
      ++m_currentPosition;
      return OperatorToken{ OperatorType::BitAnd };
    case '|':
      if ( m_currentPosition + 1 < m_input.size() && m_input[m_currentPosition + 1] == '|' )
      {
        m_currentPosition += 2;
        return OperatorToken{ OperatorType::Or };
      }
      ++m_currentPosition;
      return OperatorToken{ OperatorType::BitOr };
    case '^':
      ++m_currentPosition;
      return OperatorToken{ OperatorType::BitXor };
    case '~':
      ++m_currentPosition;
      return OperatorToken{ OperatorType::BitNot };
    case '=':
      if ( m_currentPosition + 1 < m_input.size() && m_input[m_currentPosition + 1] == '=' )
      {
        m_currentPosition += 2;
        return OperatorToken{ OperatorType::Equal };
      }
      break;
    case '!':
      if ( m_currentPosition + 1 < m_input.size() && m_input[m_currentPosition + 1] == '=' )
      {
        m_currentPosition += 2;
        return OperatorToken{ OperatorType::NotEqual };
      }
      break;
    case ';':
      ++m_currentPosition;
      return EndOfStatementToken{};
    default:
      // This should never happen if isOperatorChar is correct, but handle it gracefully
      return InvalidToken{ std::string( 1, op ) };
    }
  }

  if ( m_input[m_currentPosition] == '(' )
  {
    ++m_currentPosition;
    return GroupingToken{ true };
  }

  if ( m_input[m_currentPosition] == ')' )
  {
    ++m_currentPosition;
    return GroupingToken{ false };
  }

  if ( m_input[m_currentPosition] == ',' )
  {
    ++m_currentPosition;
    return StatementSeparatorToken{};
  }

  if ( m_input[m_currentPosition] == '{' )
  {
    ++m_currentPosition;
    return ScopeToken{ true };
  }

  if ( m_input[m_currentPosition] == '}' )
  {
    ++m_currentPosition;
    return ScopeToken{ false };
  }

  std::string invalidChar;
  while ( !isWhitespace( m_input[m_currentPosition] ) && m_currentPosition < m_input.size() )
  {
    invalidChar += m_input[m_currentPosition];
    ++m_currentPosition;
  }
  return InvalidToken{ invalidChar };
}
