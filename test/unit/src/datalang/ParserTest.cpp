#include <gtest/gtest.h>

#include "ASTPrinter.h"
#include <Parser.h>

TEST( ParserTest, EmptyProgram )
{
  datalang::Parser      parser;
  datalang::TokenStream tokens{ datalang::EndOfStreamToken{} };

  datalang::Program program = parser.parse( tokens );

  EXPECT_TRUE( program.statements.empty() );
}

TEST( ParserTest, SimpleAdditionStatementTest )
{
  datalang::Parser      parser;
  datalang::TokenStream tokens{ datalang::ConstantToken{ 42 },
                                datalang::OperatorToken{ datalang::OperatorType::Add },
                                datalang::ConstantToken{ 23 },
                                datalang::EndOfStatementToken{},
                                datalang::EndOfStreamToken{} };

  datalang::Program program = parser.parse( tokens );

  EXPECT_EQ( program.statements.size(), 1 );
}

TEST( ParserTest, SingleEmptyStatement )
{
  datalang::Parser      parser;
  datalang::TokenStream tokens{ datalang::EndOfStatementToken{}, datalang::EndOfStreamToken{} };

  datalang::Program program = parser.parse( tokens );

  EXPECT_EQ( program.statements.size(), 0 );
}

TEST( ParserTest, IfElseTest )
{
  datalang::Parser parser;

  // if ( x < 10 ) { y == 20; } else { y == 30; }
  datalang::TokenStream tokens{ datalang::StatementToken{ datalang::StatementType::If },
                                datalang::GroupingToken{ true },
                                datalang::VariableToken{ "x" },
                                datalang::OperatorToken{ datalang::OperatorType::LessThan },
                                datalang::ConstantToken{ 10 },
                                datalang::GroupingToken{ false },
                                datalang::ScopeToken{ true },
                                datalang::VariableToken{ "y" },
                                datalang::OperatorToken{ datalang::OperatorType::Equal },
                                datalang::ConstantToken{ 20 },
                                datalang::EndOfStatementToken{},
                                datalang::ScopeToken{ false },
                                datalang::StatementToken{ datalang::StatementType::Else },
                                datalang::ScopeToken{ true },
                                datalang::VariableToken{ "y" },
                                datalang::OperatorToken{ datalang::OperatorType::Equal },
                                datalang::ConstantToken{ 30 },
                                datalang::EndOfStatementToken{},
                                datalang::ScopeToken{ false },
                                datalang::EndOfStreamToken{} };

  datalang::Program program = parser.parse( tokens );

  auto& rootStatement     = program.statements[0];
  auto  rootIsIfStatement = std::holds_alternative<std::unique_ptr<datalang::IfStatement>>( rootStatement );
  EXPECT_TRUE( rootIsIfStatement );
  if ( !rootIsIfStatement )
    return;

  auto& ifStmt = std::get<std::unique_ptr<datalang::IfStatement>>( rootStatement );
  EXPECT_TRUE( ifStmt->elseBranch.has_value() );

  auto thenBranchIsValid = std::holds_alternative<datalang::ExprPtr>( ifStmt->thenBranch );
  EXPECT_TRUE( thenBranchIsValid );

  auto& thenBranch = std::get<datalang::ExprPtr>( ifStmt->thenBranch );
  EXPECT_TRUE( thenBranch != nullptr );
  auto isBinaryExpression = std::holds_alternative<datalang::BinaryExprPtr>( *thenBranch );
  EXPECT_TRUE( isBinaryExpression );
  if ( !isBinaryExpression )
    return;

  auto& binaryExpr = std::get<datalang::BinaryExprPtr>( *thenBranch );
  EXPECT_EQ( binaryExpr->op, datalang::OperatorType::Equal );

  auto leftIsIdentifier = std::holds_alternative<datalang::IdentifierPtr>( binaryExpr->left );
  EXPECT_TRUE( leftIsIdentifier );
  if ( !leftIsIdentifier )
    return;

  auto& identifier = std::get<datalang::IdentifierPtr>( binaryExpr->left );
  EXPECT_EQ( identifier->name, "y" );

  auto rightIsConstant = std::holds_alternative<datalang::ConstantPtr>( binaryExpr->right );
  EXPECT_TRUE( rightIsConstant );
  if ( !rightIsConstant )
    return;

  auto& constant = std::get<datalang::ConstantPtr>( binaryExpr->right );
  EXPECT_EQ( constant->value, 20 );
}

TEST( ParserTest, IfWithoutElseTest )
{
  datalang::Parser parser;

  // if ( x < 10 ) { y == 20; }
  datalang::TokenStream tokens{ datalang::StatementToken{ datalang::StatementType::If },
                                datalang::GroupingToken{ true },
                                datalang::VariableToken{ "x" },
                                datalang::OperatorToken{ datalang::OperatorType::LessThan },
                                datalang::ConstantToken{ 10 },
                                datalang::GroupingToken{ false },
                                datalang::ScopeToken{ true },
                                datalang::VariableToken{ "y" },
                                datalang::OperatorToken{ datalang::OperatorType::Equal },
                                datalang::ConstantToken{ 20 },
                                datalang::EndOfStatementToken{},
                                datalang::ScopeToken{ false },
                                datalang::EndOfStreamToken{} };

  datalang::Program program = parser.parse( tokens );

  EXPECT_EQ( program.statements.size(), 1 );
}

TEST( ParserTest, OperatorPrecedenceTest )
{
  datalang::Parser parser;

  // a + b * c
  datalang::TokenStream tokens{
    datalang::VariableToken{ "a" }, datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::VariableToken{ "b" }, datalang::OperatorToken{ datalang::OperatorType::Multiply },
    datalang::VariableToken{ "c" }, datalang::EndOfStatementToken{},
    datalang::EndOfStreamToken{}
  };

  datalang::Program program = parser.parse( tokens );

  EXPECT_EQ( program.statements.size(), 1 );

  auto& rootStatement       = program.statements[0];
  auto  rootIsExprStatement = std::holds_alternative<datalang::ExprPtr>( rootStatement );
  EXPECT_TRUE( rootIsExprStatement );
  if ( !rootIsExprStatement )
    return;

  auto& exprStmt = std::get<datalang::ExprPtr>( rootStatement );
  EXPECT_TRUE( exprStmt != nullptr );
  auto isBinaryExpression = std::holds_alternative<datalang::BinaryExprPtr>( *exprStmt );
  EXPECT_TRUE( isBinaryExpression );
  if ( !isBinaryExpression )
    return;

  auto& binaryExpr = std::get<datalang::BinaryExprPtr>( *exprStmt );
  EXPECT_EQ( binaryExpr->op, datalang::OperatorType::Add );

  auto rightIsBinaryExpression = std::holds_alternative<datalang::BinaryExprPtr>( binaryExpr->right );
  EXPECT_TRUE( rightIsBinaryExpression );
  if ( !rightIsBinaryExpression )
    return;

  auto& rightBinaryExpr = std::get<datalang::BinaryExprPtr>( binaryExpr->right );
  EXPECT_EQ( rightBinaryExpr->op, datalang::OperatorType::Multiply );
}

TEST( ParserTest, SimpleAdditionTest )
{
  datalang::Parser parser;

  // 10 + 20
  datalang::TokenStream tokens{ datalang::ConstantToken{ 10 },
                                datalang::OperatorToken{ datalang::OperatorType::Add },
                                datalang::ConstantToken{ 20 },
                                datalang::EndOfStatementToken{},
                                datalang::EndOfStreamToken{} };

  datalang::Program program = parser.parse( tokens );

  EXPECT_EQ( program.statements.size(), 1 );

  auto& rootStatement       = program.statements[0];
  auto  rootIsExprStatement = std::holds_alternative<datalang::ExprPtr>( rootStatement );
  EXPECT_TRUE( rootIsExprStatement );
  if ( !rootIsExprStatement )
    return;

  auto& exprStmt = std::get<datalang::ExprPtr>( rootStatement );
  EXPECT_TRUE( exprStmt != nullptr );
  auto isBinaryExpression = std::holds_alternative<datalang::BinaryExprPtr>( *exprStmt );
  EXPECT_TRUE( isBinaryExpression );
  if ( !isBinaryExpression )
    return;

  auto& binaryExpr = std::get<datalang::BinaryExprPtr>( *exprStmt );
  EXPECT_EQ( binaryExpr->op, datalang::OperatorType::Add );

  auto leftIsConstant = std::holds_alternative<datalang::ConstantPtr>( binaryExpr->left );
  EXPECT_TRUE( leftIsConstant );
  if ( !leftIsConstant )
    return;

  auto& leftConst = std::get<datalang::ConstantPtr>( binaryExpr->left );
  EXPECT_EQ( leftConst->value, 10 );

  auto rightIsConstant = std::holds_alternative<datalang::ConstantPtr>( binaryExpr->right );
  EXPECT_TRUE( rightIsConstant );
  if ( !rightIsConstant )
    return;

  auto& rightConst = std::get<datalang::ConstantPtr>( binaryExpr->right );
  EXPECT_EQ( rightConst->value, 20 );
}

TEST( ParserTest, MinMaxExpressions )
{
  datalang::Parser parser;

  // min( a + b, c - d )
  datalang::TokenStream tokens{
    datalang::MinMaxToken{ true },  datalang::GroupingToken{ true },
    datalang::VariableToken{ "a" }, datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::VariableToken{ "b" }, datalang::StatementSeparatorToken{},
    datalang::VariableToken{ "c" }, datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::VariableToken{ "d" }, datalang::GroupingToken{ false },
    datalang::EndOfStreamToken{}
  };

  datalang::Program program = parser.parse( tokens );

  EXPECT_EQ( program.statements.size(), 1 );

  auto& rootStatement       = program.statements[0];
  auto  rootIsExprStatement = std::holds_alternative<datalang::ExprPtr>( rootStatement );
  EXPECT_TRUE( rootIsExprStatement );

  if ( !rootIsExprStatement )
    return;

  auto& exprStmt = std::get<datalang::ExprPtr>( rootStatement );
  EXPECT_TRUE( exprStmt != nullptr );
  auto isBinaryExpression = std::holds_alternative<datalang::BinaryExprPtr>( *exprStmt );
  EXPECT_TRUE( isBinaryExpression );

  if ( !isBinaryExpression )
    return;

  auto& binaryExpr = std::get<datalang::BinaryExprPtr>( *exprStmt );
  EXPECT_EQ( binaryExpr->op, datalang::OperatorType::Min );

  auto& leftExpr = std::get<datalang::BinaryExprPtr>( binaryExpr->left );
  EXPECT_EQ( leftExpr->op, datalang::OperatorType::Add );

  auto& leftLeftExpr = std::get<datalang::IdentifierPtr>( leftExpr->left );
  EXPECT_EQ( leftLeftExpr->name, "a" );

  auto& leftRightExpr = std::get<datalang::IdentifierPtr>( leftExpr->right );
  EXPECT_EQ( leftRightExpr->name, "b" );

  auto& rightExpr = std::get<datalang::BinaryExprPtr>( binaryExpr->right );
  EXPECT_EQ( rightExpr->op, datalang::OperatorType::Subtract );

  auto& rightLeftExpr = std::get<datalang::IdentifierPtr>( rightExpr->left );
  EXPECT_EQ( rightLeftExpr->name, "c" );

  auto& rightRightExpr = std::get<datalang::IdentifierPtr>( rightExpr->right );
  EXPECT_EQ( rightRightExpr->name, "d" );
}

TEST( ParserTest, ComplexIfTest )
{
  // if ( (a * 2) != (b + 5) ) { 1; } else { 0; }

  datalang::Parser      parser;
  datalang::TokenStream tokens{ datalang::StatementToken{ datalang::StatementType::If },
                                datalang::GroupingToken{ true },
                                datalang::GroupingToken{ true },
                                datalang::VariableToken{ "a" },
                                datalang::OperatorToken{ datalang::OperatorType::Multiply },
                                datalang::ConstantToken{ 2 },
                                datalang::GroupingToken{ false },
                                datalang::OperatorToken{ datalang::OperatorType::NotEqual },
                                datalang::GroupingToken{ true },
                                datalang::VariableToken{ "b" },
                                datalang::OperatorToken{ datalang::OperatorType::Add },
                                datalang::ConstantToken{ 5 },
                                datalang::GroupingToken{ false },
                                datalang::GroupingToken{ false },
                                datalang::ScopeToken{ true },
                                datalang::ConstantToken{ 1 },
                                datalang::EndOfStatementToken{},
                                datalang::ScopeToken{ false },
                                datalang::StatementToken{ datalang::StatementType::Else },
                                datalang::ScopeToken{ true },
                                datalang::ConstantToken{ 0 },
                                datalang::EndOfStatementToken{},
                                datalang::ScopeToken{ false },
                                datalang::EndOfStreamToken{} };

  datalang::Program program = parser.parse( tokens );

  EXPECT_EQ( program.statements.size(), 1 );

  auto& rootStatement     = program.statements[0];
  auto  rootIsIfStatement = std::holds_alternative<std::unique_ptr<datalang::IfStatement>>( rootStatement );
  EXPECT_TRUE( rootIsIfStatement );
  if ( !rootIsIfStatement )
    return;

  auto& ifStmt = std::get<std::unique_ptr<datalang::IfStatement>>( rootStatement );

  auto& conditionExpr = ifStmt->condition;
  EXPECT_TRUE( std::holds_alternative<datalang::BinaryExprPtr>( conditionExpr ) );
  auto& conditionBinExpr = std::get<datalang::BinaryExprPtr>( conditionExpr );
  EXPECT_EQ( conditionBinExpr->op, datalang::OperatorType::NotEqual );

  auto& leftConditionExpr = std::get<datalang::BinaryExprPtr>( conditionBinExpr->left );
  EXPECT_EQ( leftConditionExpr->op, datalang::OperatorType::Multiply );

  auto& leftLeftCondIdent = std::get<datalang::IdentifierPtr>( leftConditionExpr->left );
  EXPECT_EQ( leftLeftCondIdent->name, "a" );

  auto& leftRightCondConst = std::get<datalang::ConstantPtr>( leftConditionExpr->right );
  EXPECT_EQ( leftRightCondConst->value, 2 );

  auto& rightConditionExpr = std::get<datalang::BinaryExprPtr>( conditionBinExpr->right );
  EXPECT_EQ( rightConditionExpr->op, datalang::OperatorType::Add );

  auto& rightLeftCondIdent = std::get<datalang::IdentifierPtr>( rightConditionExpr->left );
  EXPECT_EQ( rightLeftCondIdent->name, "b" );

  auto& rightRightCondConst = std::get<datalang::ConstantPtr>( rightConditionExpr->right );
  EXPECT_EQ( rightRightCondConst->value, 5 );

  auto& thenExpression = std::get<datalang::ExprPtr>( ifStmt->thenBranch );
  EXPECT_TRUE( thenExpression != nullptr );
  auto isConstant = std::holds_alternative<datalang::ConstantPtr>( *thenExpression );
  EXPECT_TRUE( isConstant );
  if ( !isConstant )
    return;
  EXPECT_EQ( std::get<datalang::ConstantPtr>( *thenExpression )->value, 1 );

  EXPECT_TRUE( ifStmt->elseBranch.has_value() );
  if ( !ifStmt->elseBranch.has_value() )
    return;

  auto& elseStmt       = *ifStmt->elseBranch;
  auto& elseExpression = std::get<datalang::ExprPtr>( elseStmt );
  EXPECT_TRUE( elseExpression != nullptr );
  isConstant = std::holds_alternative<datalang::ConstantPtr>( *elseExpression );
  EXPECT_TRUE( isConstant );
  if ( !isConstant )
    return;
  EXPECT_EQ( std::get<datalang::ConstantPtr>( *elseExpression )->value, 0 );
}

TEST( ParserTest, ComplexExpression_1 )
{
  // (a + b) * (c - d) / x

  datalang::Parser      parser;
  datalang::TokenStream tokens{ datalang::GroupingToken{ true },
                                datalang::VariableToken{ "a" },
                                datalang::OperatorToken{ datalang::OperatorType::Add },
                                datalang::VariableToken{ "b" },
                                datalang::GroupingToken{ false },
                                datalang::OperatorToken{ datalang::OperatorType::Multiply },
                                datalang::GroupingToken{ true },
                                datalang::VariableToken{ "c" },
                                datalang::OperatorToken{ datalang::OperatorType::Subtract },
                                datalang::VariableToken{ "d" },
                                datalang::GroupingToken{ false },
                                datalang::OperatorToken{ datalang::OperatorType::Divide },
                                datalang::VariableToken{ "x" },
                                datalang::EndOfStreamToken{} };

  datalang::Program program = parser.parse( tokens );
  std::stringstream output;
  datalang::ASTPrinter( output ).print( program );
  EXPECT_EQ( program.statements.size(), 1 );

  auto& rootStatement       = program.statements[0];
  auto  rootIsExprStatement = std::holds_alternative<datalang::ExprPtr>( rootStatement );
  EXPECT_TRUE( rootIsExprStatement );
  if ( !rootIsExprStatement )
    return;

  auto& rootExpr = std::get<datalang::ExprPtr>( rootStatement );
  EXPECT_TRUE( rootExpr != nullptr );

  auto isBinaryExpression = std::holds_alternative<datalang::BinaryExprPtr>( *rootExpr );
  EXPECT_TRUE( isBinaryExpression );
  if ( !isBinaryExpression )
    return;

  auto& multiplyExpression = std::get<datalang::BinaryExprPtr>( *rootExpr );
  EXPECT_EQ( multiplyExpression->op, datalang::OperatorType::Divide );

  auto& leftMultiply = std::get<datalang::BinaryExprPtr>( multiplyExpression->left );
  EXPECT_EQ( leftMultiply->op, datalang::OperatorType::Multiply );
}