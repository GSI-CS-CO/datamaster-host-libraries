#include <Parser.h>
#include <gtest/gtest.h>

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