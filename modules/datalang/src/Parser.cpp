#include "Parser.h"
using datalang::Parser;
using datalang::Program;
using datalang::Statement;
using datalang::TokenStream;
using namespace datalang;

namespace
{

std::optional<Expression> parseExpression( const TokenStream& tokens, size_t& currentIndex );
std::optional<Statement>  parseStatement( const TokenStream& tokens, size_t& currentIndex );

bool expectAndComsume( const TokenStream& tokens, size_t& currentIndex, const datalang::Token& expectedToken )
{
  if ( currentIndex < tokens.size() && tokens[currentIndex] == expectedToken )
  {
    ++currentIndex;
    return true;
  }
  return false;
}

void consumeEndOfStatement( const TokenStream& tokens, size_t& currentIndex )
{
  expectAndComsume( tokens, currentIndex, datalang::EndOfStatementToken{} );
}

std::optional<Expression>
resolveExpression( std::optional<Expression> left, std::optional<Expression> right, OperatorType op )
{
  if ( left.has_value() && right.has_value() )
  {
    BinaryExprPtr binExpr = std::make_unique<BinaryExpression>();
    binExpr->left         = std::move( *left );
    binExpr->op           = op;
    binExpr->right        = std::move( *right );
    return std::move( binExpr );
  }
  else if ( left.has_value() )
  {
    return std::move( *left );
  }
  return std::nullopt;
}

std::optional<Expression> parsePrimary( const TokenStream& tokens, size_t& currentIndex );

std::optional<Statement> parseIfStatement( const TokenStream& tokens, size_t& currentIndex )
{
  auto isIfStatement = expectAndComsume( tokens, currentIndex, StatementToken{ StatementType::If } );
  if ( !isIfStatement )
  {
    return std::nullopt;
  }

  // Further parsing logic for IfStatement goes here
  if ( !expectAndComsume( tokens, currentIndex, GroupingToken{ true } ) )
  {
    // TODO: Error handling for missing opening parenthesis
    return std::nullopt;
  }

  auto condition                = parseExpression( tokens, currentIndex );
  auto correctlyParsedCondition = condition.has_value();
  auto scopeClosedCorrectly     = expectAndComsume( tokens, currentIndex, GroupingToken{ false } );

  if ( !correctlyParsedCondition || !scopeClosedCorrectly )
  {
    // TODO: Error handling for invalid condition or missing closing parenthesis
    return std::nullopt;
  }

  auto                     thenBranch = parseStatement( tokens, currentIndex );
  std::optional<Statement> elseBranch = std::nullopt;

  if ( expectAndComsume( tokens, currentIndex, StatementToken{ StatementType::Else } ) )
  {
    elseBranch = parseStatement( tokens, currentIndex );
  }

  auto ifStmt        = std::make_unique<IfStatement>();
  ifStmt->condition  = std::move( *condition );
  ifStmt->thenBranch = std::move( *thenBranch );
  ifStmt->elseBranch = std::move( *elseBranch );

  return ifStmt;
}

std::optional<Statement> parseBlock( const TokenStream& tokens, size_t& currentIndex )
{
  if ( !expectAndComsume( tokens, currentIndex, ScopeToken{ true } ) )
  {
    // TODO: Error handling for missing opening brace
    return std::nullopt;
  }

  auto stmt = parseStatement( tokens, currentIndex );

  // Further parsing logic for Block goes here
  if ( !expectAndComsume( tokens, currentIndex, ScopeToken{ false } ) )
  {
    // TODO: Error handling for missing closing brace
    return std::nullopt;
  }

  return stmt;
}

std::optional<Expression> parseMinMaxCall( const TokenStream& tokens, size_t& currentIndex )
{
  if ( !std::holds_alternative<MinMaxToken>( tokens[currentIndex] ) )
  {
    return std::nullopt;
  }

  auto minMaxToken = std::get<MinMaxToken>( tokens[currentIndex] );
  ++currentIndex; // consume 'min' or 'max'
  if ( !( std::holds_alternative<GroupingToken>( tokens[currentIndex] ) &&
          std::get<GroupingToken>( tokens[currentIndex] ).opens ) )
  {
    return std::nullopt; // missing opening parenthesis
  }

  ++currentIndex; // consume '('
  auto firstArg = parseExpression( tokens, currentIndex );
  if ( !firstArg.has_value() )
  {
    return std::nullopt; // failed to parse first argument
  }

  if ( !( std::holds_alternative<StatementSeparatorToken>( tokens[currentIndex] ) ) )
  {
    return std::nullopt; // missing comma
  }

  ++currentIndex; // consume ','
  auto secondArg = parseExpression( tokens, currentIndex );
  if ( !secondArg.has_value() )
  {
    return std::nullopt; // failed to parse second argument
  }

  if ( !( std::holds_alternative<GroupingToken>( tokens[currentIndex] ) &&
          !std::get<GroupingToken>( tokens[currentIndex] ).opens ) )
  {
    return std::nullopt; // missing closing parenthesis
  }

  ++currentIndex; // consume ')'

  BinaryExprPtr minMaxExpr = std::make_unique<BinaryExpression>();
  minMaxExpr->left         = std::move( firstArg.value() );
  minMaxExpr->right        = std::move( secondArg.value() );
  minMaxExpr->op           = ( minMaxToken.isMin ) ? OperatorType::Min : OperatorType::Max;
  return std::move( minMaxExpr );
}

std::optional<Expression> parsePrimary( const TokenStream& tokens, size_t& currentIndex )
{
  if ( std::holds_alternative<ConstantToken>( tokens[currentIndex] ) )
  {
    auto constToken = std::get<ConstantToken>( tokens[currentIndex] );
    ++currentIndex;

    auto constExpr   = std::make_unique<Constant>();
    constExpr->value = constToken.value;
    return std::move( constExpr );
  }

  if ( std::holds_alternative<VariableToken>( tokens[currentIndex] ) )
  {
    auto varToken = std::get<VariableToken>( tokens[currentIndex] );
    ++currentIndex;

    auto varExpr  = std::make_unique<Identifier>();
    varExpr->name = varToken.name;
    return std::move( varExpr );
  }

  if ( std::holds_alternative<GroupingToken>( tokens[currentIndex] ) &&
       std::get<GroupingToken>( tokens[currentIndex] ).opens )
  {
    ++currentIndex; // consume '('
    auto expr = parseExpression( tokens, currentIndex );
    if ( !expr.has_value() )
    {
      return std::nullopt;
    }

    if ( std::holds_alternative<GroupingToken>( tokens[currentIndex] ) &&
         !std::get<GroupingToken>( tokens[currentIndex] ).opens )
    {
      ++currentIndex; // consume ')'
      return expr;
    }
    else
    {
      return std::nullopt; // missing closing parenthesis
    }
  }

  if ( std::holds_alternative<MinMaxToken>( tokens[currentIndex] ) )
  {
    return parseMinMaxCall( tokens, currentIndex );
  }

  return std::nullopt;
}

std::optional<OperatorType> parseUnaryOp( const TokenStream& tokens, size_t& currentIndex )
{
  if ( !std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    return std::nullopt;
  }

  auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
  if ( !( opToken == OperatorType::Subtract || opToken == OperatorType::BitNot ) )
  {
    return std::nullopt;
  }

  ++currentIndex;
  return opToken;
}

std::optional<Expression> parseUnaryExpr( const TokenStream& tokens, size_t& currentIndex )
{
  auto potentialOp = parseUnaryOp( tokens, currentIndex );
  if ( potentialOp.has_value() )
  {
    auto unaryExpr = std::make_unique<UnaryExpr>();
    unaryExpr->op  = *potentialOp;
    auto operand   = parseUnaryExpr( tokens, currentIndex );
    if ( !operand.has_value() )
    {
      return std::nullopt;
    }
    unaryExpr->operand = std::move( operand.value() );
    return std::move( unaryExpr );
  }

  return parsePrimary( tokens, currentIndex );
}

std::optional<Expression> parseMulExprTail( const TokenStream& tokens, size_t& currentIndex, OperatorType& op )
{
  if ( !std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    return std::nullopt;
  }

  auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
  if ( !( opToken == OperatorType::Multiply || opToken == OperatorType::Divide || opToken == OperatorType::Modulus ) )
  {
    return std::nullopt;
  }

  op = opToken;
  ++currentIndex;
  auto left  = parseUnaryExpr( tokens, currentIndex );
  auto right = parseMulExprTail( tokens, currentIndex, op );
  return resolveExpression( std::move( left ), std::move( right ), op );
}

std::optional<Expression> parseMulExpr( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseUnaryExpr( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }

  OperatorType op    = OperatorType::Multiply; // Placeholder, will be set in parseMulExprTail
  auto         right = parseMulExprTail( tokens, currentIndex, op );
  return resolveExpression( std::move( left ), std::move( right ), op );
}

std::optional<Expression> parseAddExprTail( const TokenStream& tokens, size_t& currentIndex, OperatorType& op )
{
  if ( !std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    return std::nullopt;
  }

  auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
  if ( !( opToken == OperatorType::Add || opToken == OperatorType::Subtract ) )
  {
    return std::nullopt;
  }

  op = opToken;
  ++currentIndex;
  auto left  = parseMulExpr( tokens, currentIndex );
  auto right = parseAddExprTail( tokens, currentIndex, op );
  return resolveExpression( std::move( left ), std::move( right ), op );
}

std::optional<Expression> parseAddExpr( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseMulExpr( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }

  OperatorType op    = OperatorType::Add; // Placeholder, will be set in parseAddExprTail
  auto         right = parseAddExprTail( tokens, currentIndex, op );
  return resolveExpression( std::move( left ), std::move( right ), op );
}

std::optional<Expression> parseShiftExprTail( const TokenStream& tokens, size_t& currentIndex, OperatorType& op )
{
  if ( !std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    return std::nullopt;
  }

  auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
  if ( !( opToken == OperatorType::LSH || opToken == OperatorType::RSH ) )
  {
    return std::nullopt;
  }

  op = opToken;
  ++currentIndex;
  auto left  = parseAddExpr( tokens, currentIndex );
  auto right = parseShiftExprTail( tokens, currentIndex, op );
  return resolveExpression( std::move( left ), std::move( right ), op );
}

std::optional<Expression> parseShiftExpr( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseAddExpr( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }

  OperatorType op = OperatorType::LSH; // Placeholder, will be set in parseShiftExprTail

  auto right = parseShiftExprTail( tokens, currentIndex, op );
  return resolveExpression( std::move( left ), std::move( right ), op );
}

std::optional<Expression> parseRelationalTail( const TokenStream& tokens, size_t& currentIndex, OperatorType& op )
{
  if ( !std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    return std::nullopt;
  }

  auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
  if ( !( opToken == OperatorType::LessThan || opToken == OperatorType::LessThanOrEqual ||
          opToken == OperatorType::GreaterThan || opToken == OperatorType::GreaterThanOrEqual ) )
  {
    return std::nullopt;
  }

  op = opToken;
  ++currentIndex;
  auto left  = parseShiftExpr( tokens, currentIndex );
  auto right = parseRelationalTail( tokens, currentIndex, op );
  return resolveExpression( std::move( left ), std::move( right ), op );
}

std::optional<Expression> parseRelational( const TokenStream& tokens, size_t& currentIndex )
{
  auto         left  = parseShiftExpr( tokens, currentIndex );
  OperatorType op    = OperatorType::LessThan; // Placeholder, will be set in parseRelationalTail
  auto         right = parseRelationalTail( tokens, currentIndex, op );
  if ( left.has_value() && right.has_value() )
  {
    BinaryExprPtr binExpr = std::make_unique<BinaryExpression>();
    binExpr->left         = std::move( *left );
    binExpr->op           = op;
    binExpr->right        = std::move( *right );
    return std::move( binExpr );
  }
  else if ( left.has_value() )
  {
    return std::move( left );
  }
  return std::nullopt;
}

std::optional<Expression> parseEqualityTail( const TokenStream& tokens, size_t& currentIndex, OperatorType& op )
{
  if ( !std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    return std::nullopt;
  }

  auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
  if ( !( opToken == OperatorType::Equal || opToken == OperatorType::NotEqual ) )
  {
    return std::nullopt;
  }

  op = opToken;
  ++currentIndex;
  auto left  = parseRelational( tokens, currentIndex );
  auto right = parseEqualityTail( tokens, currentIndex, op );
  return resolveExpression( std::move( left ), std::move( right ), op );
}

std::optional<Expression> parseEquality( const TokenStream& tokens, size_t& currentIndex )
{
  auto         left  = parseRelational( tokens, currentIndex );
  OperatorType op    = OperatorType::Equal; // Placeholder, will be set in parseEqualityTail
  auto         right = parseEqualityTail( tokens, currentIndex, op );

  return resolveExpression( std::move( left ), std::move( right ), op );
}

std::optional<Expression> parseBitwiseAndTail( const TokenStream& tokens, size_t& currentIndex )
{
  if ( !( std::holds_alternative<OperatorToken>( tokens[currentIndex] ) &&
          std::get<OperatorToken>( tokens[currentIndex] ).op == OperatorType::BitAnd ) )
  {
    return std::nullopt;
  }

  ++currentIndex;
  auto left  = parseEquality( tokens, currentIndex );
  auto right = parseBitwiseAndTail( tokens, currentIndex );
  return resolveExpression( std::move( left ), std::move( right ), OperatorType::BitAnd );
}

std::optional<Expression> parseBitwiseAnd( const TokenStream& tokens, size_t& currentIndex )
{
  auto left  = parseEquality( tokens, currentIndex );
  auto right = parseBitwiseAndTail( tokens, currentIndex );
  return resolveExpression( std::move( left ), std::move( right ), OperatorType::BitAnd );
}

std::optional<Expression> parseBitwiseXorTail( const TokenStream& tokens, size_t& currentIndex )
{
  if ( !( std::holds_alternative<OperatorToken>( tokens[currentIndex] ) &&
          std::get<OperatorToken>( tokens[currentIndex] ).op == OperatorType::BitXor ) )
  {
    return std::nullopt;
  }
  ++currentIndex;
  auto left  = parseBitwiseAnd( tokens, currentIndex );
  auto right = parseBitwiseXorTail( tokens, currentIndex );
  return resolveExpression( std::move( left ), std::move( right ), OperatorType::BitXor );
}

std::optional<Expression> parseBitwiseXor( const TokenStream& tokens, size_t& currentIndex )
{
  auto left  = parseBitwiseAnd( tokens, currentIndex );
  auto right = parseBitwiseXorTail( tokens, currentIndex );
  return resolveExpression( std::move( left ), std::move( right ), OperatorType::BitXor );
}

std::optional<Expression> parseBitwiseOrTail( const TokenStream& tokens, size_t& currentIndex )
{
  if ( !( std::holds_alternative<OperatorToken>( tokens[currentIndex] ) &&
          std::get<OperatorToken>( tokens[currentIndex] ).op == OperatorType::BitOr ) )
  {
    return std::nullopt;
  }

  ++currentIndex;
  auto left  = parseBitwiseXor( tokens, currentIndex );
  auto right = parseBitwiseOrTail( tokens, currentIndex );
  return resolveExpression( std::move( left ), std::move( right ), OperatorType::BitOr );
}

std::optional<Expression> parseBitwiseOr( const TokenStream& tokens, size_t& currentIndex )
{
  auto left  = parseBitwiseXor( tokens, currentIndex );
  auto right = parseBitwiseOrTail( tokens, currentIndex );
  return resolveExpression( std::move( left ), std::move( right ), OperatorType::BitOr );
}

std::optional<Expression> parseExpression( const TokenStream& tokens, size_t& currentIndex )
{
  auto expr = parseBitwiseOr( tokens, currentIndex );
  if ( expr.has_value() )
  {
    return expr;
  }
  return std::nullopt;
}

std::optional<Statement> parseExprStatement( const TokenStream& tokens, size_t& currentIndex )
{
  auto statement = parseExpression( tokens, currentIndex );
  if ( statement.has_value() )
  {
    return std::make_unique<Expression>( std::move( statement.value() ) );
  }
  return std::nullopt;
}

std::optional<Statement> parseStatement( const TokenStream& tokens, size_t& currentIndex )
{
  std::optional<Statement> result = parseIfStatement( tokens, currentIndex );
  if ( !result.has_value() )
  {
    result = parseBlock( tokens, currentIndex );
  }

  if ( !result.has_value() )
  {
    result = parseExprStatement( tokens, currentIndex );
  }

  if ( result.has_value() )
  {
    consumeEndOfStatement( tokens, currentIndex );
  }

  return result;
}

Program parseProgram( const TokenStream& tokens )
{
  Program                  program;
  size_t                   index     = 0;
  std::optional<Statement> statement = std::nullopt;
  while ( true )
  {
    auto statement = parseStatement( tokens, index );
    if ( !statement.has_value() )
    {
      break;
    }

    program.statements.push_back( std::move( statement.value() ) );
  }

  return program;
}
} // namespace

Program Parser::parse( const TokenStream& tokens )
{
  return parseProgram( tokens );
}