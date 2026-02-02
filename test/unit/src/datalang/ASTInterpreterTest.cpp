#include "ASTInterpreter.h"
#include <gtest/gtest.h>

#include "StubEnvironment.h"

using namespace datalang;

TEST( ASTInterpreterTest, SimpleConstantInterpretation )
{
  StubEnvironment env( {} );
  ASTInterpreter  interpreter( env );

  Program program;
  // Program with a single constant expression: 42
  ConstantPtr constantExpr = std::make_unique<Constant>();
  constantExpr->value      = 42;

  program.statements.push_back( std::make_unique<Expression>( std::move( constantExpr ) ) );

  ConstantPtr result = interpreter.interpret( program );

  ASSERT_NE( result, nullptr );
  EXPECT_EQ( result->value, 42 );
}

TEST( ASTInterpreterTest, SimpleVariableInterpretation )
{
  StubEnvironment env( { { "x", 42 } } );
  ASTInterpreter  interpreter( env );

  Program program;

  // Program with a single variable expression: x
  IdentifierPtr varExpr = std::make_unique<Identifier>();
  varExpr->name         = "x";

  program.statements.push_back( std::make_unique<Expression>( std::move( varExpr ) ) );

  ConstantPtr result = interpreter.interpret( program );

  ASSERT_NE( result, nullptr );
  EXPECT_EQ( result->value, 42 );
}

TEST( ASTInterpreterTest, VariableNotFound )
{
  StubEnvironment env( {} );
  ASTInterpreter  interpreter( env );

  Program program;

  // Program with a single variable expression: y
  IdentifierPtr varExpr = std::make_unique<Identifier>();
  varExpr->name         = "y";

  program.statements.push_back( std::make_unique<Expression>( std::move( varExpr ) ) );

  EXPECT_THROW( interpreter.interpret( program ), std::runtime_error );
}

TEST( ASTInterpreterTest, EmptyProgram )
{
  StubEnvironment env( {} );
  ASTInterpreter  interpreter( env );

  Program program; // Empty program

  ConstantPtr result = interpreter.interpret( program );

  EXPECT_EQ( result, nullptr );
}

TEST( ASTInterpreterTest, MultipleStatements )
{
  StubEnvironment env( { { "a", 10 }, { "b", 20 } } );
  ASTInterpreter  interpreter( env );

  Program program;

  // First statement: a
  {
    IdentifierPtr varExpr = std::make_unique<Identifier>();
    varExpr->name         = "a";
    program.statements.push_back( std::make_unique<Expression>( std::move( varExpr ) ) );
  }

  // Second statement: b
  {
    IdentifierPtr varExpr = std::make_unique<Identifier>();
    varExpr->name         = "b";
    program.statements.push_back( std::make_unique<Expression>( std::move( varExpr ) ) );
  }

  ConstantPtr result = interpreter.interpret( program );

  ASSERT_NE( result, nullptr );
  EXPECT_EQ( result->value, 20 ); // The result should be from the last statement
}

TEST( ASTInterpreterTest, MinMaxExpressions )
{
  StubEnvironment env( { { "a", 10 }, { "b", 5 }, { "c", 20 }, { "d", 3 } } );
  ASTInterpreter  interpreter( env );

  Program program;

  // min( a + b, c - d )
  {
    auto minExpr = std::make_unique<BinaryExpression>(
        std::make_unique<BinaryExpression>(
            std::make_unique<Identifier>( "a" ), OperatorType::Add, std::make_unique<Identifier>( "b" ) ),
        OperatorType::Min,
        std::make_unique<BinaryExpression>(
            std::make_unique<Identifier>( "c" ), OperatorType::Subtract, std::make_unique<Identifier>( "d" ) ) );
    program.statements.push_back( std::make_unique<Expression>( std::move( minExpr ) ) );
  }

  ConstantPtr result = interpreter.interpret( program );

  ASSERT_NE( result, nullptr );
  EXPECT_EQ( result->value, 15 ); // The result should be from the last statement
}

TEST( ASTInterpreterTest, ComplexIfTest )
{
  // if ( (a * 2) != (b + 5) ) { 1; } else { 0; }

  StubEnvironment env( { { "a", 10 }, { "b", 5 } } );
  StubEnvironment env2( { { "a", 15 }, { "b", 25 } } );

  ASTInterpreter interpreter( env );
  ASTInterpreter interpreter_false( env2 );

  Program program;

  {
    auto conditionExpr = std::make_unique<BinaryExpression>(
        std::make_unique<BinaryExpression>(
            std::make_unique<Identifier>( "a" ), OperatorType::Multiply, std::make_unique<Constant>( 2 ) ),
        OperatorType::NotEqual,
        std::make_unique<BinaryExpression>(
            std::make_unique<Identifier>( "b" ), OperatorType::Add, std::make_unique<Constant>( 5 ) ) );

    auto thenExpr   = std::make_unique<Constant>();
    thenExpr->value = 1;

    auto elseExpr   = std::make_unique<Constant>();
    elseExpr->value = 0;

    auto ifStmt        = std::make_unique<IfStatement>();
    ifStmt->condition  = std::move( conditionExpr );
    ifStmt->thenBranch = std::make_unique<Expression>( std::move( thenExpr ) );
    ifStmt->elseBranch = std::make_unique<Expression>( std::move( elseExpr ) );

    program.statements.push_back( std::move( ifStmt ) );
  }
  ConstantPtr result = interpreter.interpret( program );

  ASSERT_NE( result, nullptr );
  EXPECT_EQ( result->value, 1 );

  ConstantPtr result_false = interpreter_false.interpret( program );
  ASSERT_NE( result_false, nullptr );
  EXPECT_EQ( result_false->value, 0 );
}