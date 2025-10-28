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