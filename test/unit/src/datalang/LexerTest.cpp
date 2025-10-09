#include "gtest/gtest.h"

#include <Lexer.h>
TEST( LexerTest, ConstantAddition )
{
  std::string     input = "42 + 23";
  datalang::Lexer lexer( input );

  auto token = lexer.getNextToken();
  EXPECT_EQ( token.type, datalang::TokenType::Constant );
  EXPECT_EQ( token.value, "42" );

  token = lexer.getNextToken();
  EXPECT_EQ( token.type, datalang::TokenType::Plus );
  EXPECT_EQ( token.value, "+" );

  token = lexer.getNextToken();
  EXPECT_EQ( token.type, datalang::TokenType::Constant );
  EXPECT_EQ( token.value, "23" );

  token = lexer.getNextToken();
  EXPECT_EQ( token.type, datalang::TokenType::End );
}