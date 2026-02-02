#include "Parser.h"
using datalang::Parser;
using datalang::Program;
using datalang::Statement;
using datalang::TokenStream;
using namespace datalang;

namespace
{

std::optional<OperatorType>
tryConsumeOperator( const TokenStream& tokens, size_t& currentIndex, OperatorType expectedOp )
{
  if ( std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
    if ( opToken == expectedOp )
    {
      ++currentIndex;
      return expectedOp;
    }
  }
  return std::nullopt;
}

std::optional<OperatorType>
tryConsumeAnyOperator( const TokenStream& tokens, size_t& currentIndex, const std::vector<OperatorType>& expectedOps )
{
  if ( std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
    for ( const auto& expectedOp : expectedOps )
    {
      if ( opToken == expectedOp )
      {
        ++currentIndex;
        return expectedOp;
      }
    }
  }
  return std::nullopt;
}

// Forward declarations
std::optional<Expression> parseExpression( const TokenStream& tokens, size_t& currentIndex );
std::optional<Statement>  parseStatement( const TokenStream& tokens, size_t& currentIndex );

/**
 * Tries to match and consume the expected token from the token stream.
 *
 * @param tokens The stream of tokens.
 * @param currentIndex The current index in the token stream.
 * @param expectedToken The token to expect and consume.
 * @return True if the expected token was found and consumed, false otherwise.
 */
bool expectAndConsume( const TokenStream& tokens, size_t& currentIndex, const datalang::Token& expectedToken )
{
  if ( currentIndex < tokens.size() && tokens[currentIndex] == expectedToken )
  {
    ++currentIndex;
    return true;
  }
  return false;
}

/**
 * Consumes an EndOfStatementToken from the token stream.
 *
 * @param tokens The stream of tokens.
 * @param currentIndex The current index in the token stream.
 */
void consumeEndOfStatement( const TokenStream& tokens, size_t& currentIndex )
{
  expectAndConsume( tokens, currentIndex, datalang::EndOfStatementToken{} );
}

/**
 * Resolves a binary expression from the given left and right expressions and operator.
 * @param left The left expression.
 * @param right The right expression.
 * @param op The operator.
 * @return The resolved expression, or std::nullopt if resolution failed.
 */
std::optional<Expression>
resolveBinaryExpression( std::optional<Expression> left, std::optional<Expression> right, OperatorType op )
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
  else
    return std::move( *right );
}

/**
 * Parses a primary expression from the token stream.
 * @param tokens The stream of tokens.
 * @param currentIndex The current index in the token stream.
 * @return The parsed primary expression, or std::nullopt if parsing failed.
 */
std::optional<Expression> parsePrimary( const TokenStream& tokens, size_t& currentIndex );

/**
 * Parses an IfStatement from the token stream.
 * @param tokens The stream of tokens.
 * @param currentIndex The current index in the token stream.
 * @return The parsed IfStatement, or std::nullopt if parsing failed.
 */
std::optional<Statement> parseIfStatement( const TokenStream& tokens, size_t& currentIndex )
{
  auto isIfStatement = expectAndConsume( tokens, currentIndex, StatementToken{ StatementType::If } );
  if ( !isIfStatement )
  {
    return std::nullopt;
  }

  // Further parsing logic for IfStatement goes here
  if ( !expectAndConsume( tokens, currentIndex, GroupingToken{ true } ) )
  {
    // TODO: Error handling for missing opening parenthesis
    return std::nullopt;
  }

  auto condition                = parseExpression( tokens, currentIndex );
  auto correctlyParsedCondition = condition.has_value();
  auto scopeClosedCorrectly     = expectAndConsume( tokens, currentIndex, GroupingToken{ false } );

  if ( !correctlyParsedCondition || !scopeClosedCorrectly )
  {
    // TODO: Error handling for invalid condition or missing closing parenthesis
    return std::nullopt;
  }

  auto                     thenBranch = parseStatement( tokens, currentIndex );
  std::optional<Statement> elseBranch = std::nullopt;

  if ( expectAndConsume( tokens, currentIndex, StatementToken{ StatementType::Else } ) )
  {
    elseBranch = parseStatement( tokens, currentIndex );
  }

  auto ifStmt        = std::make_unique<IfStatement>();
  ifStmt->condition  = std::move( *condition );
  ifStmt->thenBranch = std::move( *thenBranch );
  ifStmt->elseBranch = std::move( *elseBranch );

  return ifStmt;
}

/**
 * Parses a Block from the token stream.
 * @param tokens The stream of tokens.
 * @param currentIndex The current index in the token stream.
 * @return The parsed Block, or std::nullopt if parsing failed.
 */
std::optional<Statement> parseBlock( const TokenStream& tokens, size_t& currentIndex )
{
  if ( !expectAndConsume( tokens, currentIndex, ScopeToken{ true } ) )
  {
    // TODO: Error handling for missing opening brace
    return std::nullopt;
  }

  auto stmt = parseStatement( tokens, currentIndex );

  // Further parsing logic for Block goes here
  if ( !expectAndConsume( tokens, currentIndex, ScopeToken{ false } ) )
  {
    // TODO: Error handling for missing closing brace
    return std::nullopt;
  }

  return stmt;
}

/**
 * Parses a min/max function call from the token stream.
 * @param tokens The stream of tokens.
 * @param currentIndex The current index in the token stream.
 * @return The parsed min/max expression, or std::nullopt if parsing failed.
 */
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

  return resolveBinaryExpression(
      std::move( firstArg ), std::move( secondArg ), ( minMaxToken.isMin ) ? OperatorType::Min : OperatorType::Max );
}

/**
 * Parses a primary expression from the token stream.
 * @param tokens The stream of tokens.
 * @param currentIndex The current index in the token stream.
 * @return The parsed primary expression, or std::nullopt if parsing failed.
 */
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

/**
 * Parses a unary operator from the token stream.
 * @param tokens The stream of tokens.
 * @param currentIndex The current index in the token stream.
 * @return The parsed unary operator, or std::nullopt if parsing failed.
 */
std::optional<OperatorType> parseUnaryOp( const TokenStream& tokens, size_t& currentIndex )
{
  return tryConsumeAnyOperator( tokens, currentIndex, { OperatorType::Subtract, OperatorType::BitNot } );
}

/**
 * Parses a unary expression from the token stream.
 * @param tokens The stream of tokens.
 * @param currentIndex The current index in the token stream.
 * @return The parsed unary expression, or std::nullopt if parsing failed.
 */
std::optional<Expression> parseUnaryExpr( const TokenStream& tokens, size_t& currentIndex )
{
  auto potentialOp = parseUnaryOp( tokens, currentIndex );
  if ( !potentialOp.has_value() )
  {
    return parsePrimary( tokens, currentIndex );
  }

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

std::optional<Expression>
parseMulExprTail( Expression mulStartExpression, const TokenStream& tokens, size_t& currentIndex )
{
  auto operatorConsumed = tryConsumeAnyOperator(
      tokens, currentIndex, { OperatorType::Multiply, OperatorType::Divide, OperatorType::Modulus } );

  if ( !operatorConsumed.has_value() )
  {
    return mulStartExpression;
  }

  auto left = parseUnaryExpr( tokens, currentIndex );
  left      = resolveBinaryExpression( std::move( mulStartExpression ), std::move( left ), *operatorConsumed );
  return parseMulExprTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseMulExpr( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseUnaryExpr( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }
  return parseMulExprTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseAddExprTail( Expression addExprLeft, const TokenStream& tokens, size_t& currentIndex )
{
  auto operatorConsumed = tryConsumeAnyOperator( tokens, currentIndex, { OperatorType::Add, OperatorType::Subtract } );
  if ( !operatorConsumed.has_value() )
  {
    return addExprLeft;
  }

  auto left = parseMulExpr( tokens, currentIndex );
  left      = resolveBinaryExpression( std::move( addExprLeft ), std::move( left ), *operatorConsumed );
  return parseAddExprTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseAddExpr( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseMulExpr( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }

  return parseAddExprTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression>
parseShiftExprTail( Expression shiftExpressionLeft, const TokenStream& tokens, size_t& currentIndex )
{
  if ( !std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    return shiftExpressionLeft;
  }

  auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
  if ( !( opToken == OperatorType::LSH || opToken == OperatorType::RSH ) )
  {
    return shiftExpressionLeft;
  }

  ++currentIndex;

  OperatorType childOperatorType = OperatorType::LSH; // Placeholder for child operator
  auto         left              = parseAddExpr( tokens, currentIndex );
  left = resolveBinaryExpression( std::move( shiftExpressionLeft ), std::move( left ), opToken );
  return parseShiftExprTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseShiftExpr( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseAddExpr( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }

  return parseShiftExprTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression>
parseRelationalTail( Expression relationalLeft, const TokenStream& tokens, size_t& currentIndex )
{
  if ( !std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    return relationalLeft;
  }

  auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
  if ( !( opToken == OperatorType::LessThan || opToken == OperatorType::LessThanOrEqual ||
          opToken == OperatorType::GreaterThan || opToken == OperatorType::GreaterThanOrEqual ) )
  {
    return relationalLeft;
  }

  ++currentIndex;
  OperatorType childOperatorType = OperatorType::LessThan; // Placeholder for child operator
  auto         left              = parseShiftExpr( tokens, currentIndex );
  left                           = resolveBinaryExpression( std::move( relationalLeft ), std::move( left ), opToken );
  return parseRelationalTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseRelational( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseShiftExpr( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }
  return parseRelationalTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseEqualityTail( Expression equalityLeft, const TokenStream& tokens, size_t& currentIndex )
{
  if ( !std::holds_alternative<OperatorToken>( tokens[currentIndex] ) )
  {
    return equalityLeft;
  }

  auto opToken = std::get<OperatorToken>( tokens[currentIndex] ).op;
  if ( !( opToken == OperatorType::Equal || opToken == OperatorType::NotEqual ) )
  {
    return equalityLeft;
  }

  ++currentIndex;
  OperatorType childOperatorType = OperatorType::Equal; // Placeholder for child operator
  auto         left              = parseRelational( tokens, currentIndex );
  left                           = resolveBinaryExpression( std::move( equalityLeft ), std::move( left ), opToken );
  return parseEqualityTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseEquality( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseRelational( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }
  return parseEqualityTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression>
parseBitwiseAndTail( Expression bitwiseAndLeft, const TokenStream& tokens, size_t& currentIndex )
{
  if ( !( std::holds_alternative<OperatorToken>( tokens[currentIndex] ) &&
          std::get<OperatorToken>( tokens[currentIndex] ).op == OperatorType::BitAnd ) )
  {
    return bitwiseAndLeft;
  }

  ++currentIndex;
  auto left = parseEquality( tokens, currentIndex );
  left      = resolveBinaryExpression( std::move( bitwiseAndLeft ), std::move( left ), OperatorType::BitAnd );
  return parseBitwiseAndTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseBitwiseAnd( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseEquality( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }
  return parseBitwiseAndTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression>
parseBitwiseXorTail( Expression bitwiseXorLeft, const TokenStream& tokens, size_t& currentIndex )
{
  if ( !( std::holds_alternative<OperatorToken>( tokens[currentIndex] ) &&
          std::get<OperatorToken>( tokens[currentIndex] ).op == OperatorType::BitXor ) )
  {
    return bitwiseXorLeft;
  }
  ++currentIndex;
  auto left = parseBitwiseAnd( tokens, currentIndex );
  left      = resolveBinaryExpression( std::move( bitwiseXorLeft ), std::move( left ), OperatorType::BitXor );
  return parseBitwiseXorTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseBitwiseXor( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseBitwiseAnd( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }
  return parseBitwiseXorTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseBitwiseOrTail( Expression bitwiseLeft, const TokenStream& tokens, size_t& currentIndex )
{
  if ( !( std::holds_alternative<OperatorToken>( tokens[currentIndex] ) &&
          std::get<OperatorToken>( tokens[currentIndex] ).op == OperatorType::BitOr ) )
  {
    return bitwiseLeft;
  }

  ++currentIndex;
  auto left = parseBitwiseXor( tokens, currentIndex );
  left      = resolveBinaryExpression( std::move( bitwiseLeft ), std::move( left ), OperatorType::BitOr );
  return parseBitwiseOrTail( std::move( *left ), tokens, currentIndex );
}

std::optional<Expression> parseBitwiseOr( const TokenStream& tokens, size_t& currentIndex )
{
  auto left = parseBitwiseXor( tokens, currentIndex );
  if ( !left.has_value() )
  {
    return std::nullopt;
  }
  return parseBitwiseOrTail( std::move( *left ), tokens, currentIndex );
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