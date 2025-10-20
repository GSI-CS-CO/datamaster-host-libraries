#include "gtest/gtest.h"
#include <Lexer.h>

namespace {
void ExpectTokenStream( const std::string& input, const std::vector<datalang::Token>& expectedTokens )
{
  datalang::Lexer lexer( input );
  for ( const auto& expectedToken : expectedTokens )
  {
    auto token = lexer.getNextToken();
    EXPECT_EQ( token, expectedToken );
  }
}
}

TEST( LexerTest, SimpleConstant )
{
  std::string     input = "123";
  ExpectTokenStream( input, { datalang::ConstantToken{ 123 }, datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, ConstantAddition )
{
  std::string     input = "42 + 23";
  ExpectTokenStream( input, {
    datalang::ConstantToken{ 42 },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::ConstantToken{ 23 }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, VariableMultiplication )
{
  std::string     input = "myVar * anotherVar";
  ExpectTokenStream( input, {
    datalang::VariableToken{ "myVar" },
    datalang::OperatorToken{ datalang::OperatorType::Multiply },
    datalang::VariableToken{ "anotherVar" }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, ParenthesesAndSubtraction )
{
  std::string     input = "( x - 10 )";
  ExpectTokenStream( input, {
    datalang::ScopeToken{ true },
    datalang::VariableToken{ "x" },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::ConstantToken{ 10 },
    datalang::ScopeToken{ false }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, ComplexExpression )
{
  std::string     input = " ( var1 + 100 ) / var2 - 42 * ( var3 - 7 ) ";
  ExpectTokenStream( input, {
    datalang::ScopeToken{ true },
    datalang::VariableToken{ "var1" },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::ConstantToken{ 100 },
    datalang::ScopeToken{ false },
    datalang::OperatorToken{ datalang::OperatorType::Divide },
    datalang::VariableToken{ "var2" },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::ConstantToken{ 42 },
    datalang::OperatorToken{ datalang::OperatorType::Multiply },
    datalang::ScopeToken{ true },
    datalang::VariableToken{ "var3" },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::ConstantToken{ 7 },
    datalang::ScopeToken{ false }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, InvalidCharacters )
{
  std::string     input = "42 + @invalid - $var";
  ExpectTokenStream( input, {
    datalang::ConstantToken{ 42 },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::InvalidToken{ "@invalid" },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::InvalidToken{ "$var" }
  } );
}

TEST( LexerTest, EmptyInput )
{
  std::string     input = "   ";
  ExpectTokenStream( input, { datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, OnlyInvalidCharacters )
{
  std::string     input = "@#$%";
  ExpectTokenStream( input, {
    datalang::InvalidToken{ "@#$%" }
  } );
}

TEST( LexerTest, MixedValidAndInvalid )
{
  std::string     input = "x + 10 @ y - $z";
  ExpectTokenStream( input, {
    datalang::VariableToken{ "x" },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::ConstantToken{ 10 },
    datalang::InvalidToken{ "@"},
    datalang::VariableToken{ "y" },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::InvalidToken{ "$z" }
  } );
}

TEST( LexerTest, NoSpacesBetweenTokens )
{
  std::string     input = "a+42*(b-7)";
  ExpectTokenStream( input, {
    datalang::VariableToken{ "a" },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::ConstantToken{ 42 },
    datalang::OperatorToken{ datalang::OperatorType::Multiply },
    datalang::ScopeToken{ true },
    datalang::VariableToken{ "b" },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::ConstantToken{ 7 },
    datalang::ScopeToken{ false }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, LargeNumbers )
{
  std::string     input = "1234567890 + 987654321";
  ExpectTokenStream( input, {
    datalang::ConstantToken{ 1234567890 },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::ConstantToken{ 987654321 }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, SingleInvalidCharacter )
{
  std::string     input = "@";
  ExpectTokenStream( input, {
    datalang::InvalidToken{ "@" }
  } );
}

TEST( LexerTest, TrailingWhitespace )
{
  std::string     input = "var + 100   ";
  ExpectTokenStream( input, {
    datalang::VariableToken{ "var" },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::ConstantToken{ 100 }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, LeadingWhitespace )
{
  std::string     input = "   var - 50";
  ExpectTokenStream( input, {
    datalang::VariableToken{ "var" },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::ConstantToken{ 50 }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, ConsecutiveOperators )
{
  std::string     input = "x++y--z**2";
  ExpectTokenStream( input, {
    datalang::VariableToken{ "x" },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::VariableToken{ "y" },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::VariableToken{ "z" },
    datalang::OperatorToken{ datalang::OperatorType::Multiply },
    datalang::OperatorToken{ datalang::OperatorType::Multiply },
    datalang::ConstantToken{ 2 }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, EmptyParentheses )
{
  std::string     input = "()";
  ExpectTokenStream( input, {
    datalang::ScopeToken{ true },
    datalang::ScopeToken{ false }, datalang::EndOfStreamToken{} 
  } );
}

TEST( LexerTest, NestedParentheses )
{
  std::string     input = "( ( x + 1 ) * ( y - 2 ) )";
  ExpectTokenStream( input, {
    datalang::ScopeToken{ true },
    datalang::ScopeToken{ true },
    datalang::VariableToken{ "x" },
    datalang::OperatorToken{ datalang::OperatorType::Add },
    datalang::ConstantToken{ 1 },
    datalang::ScopeToken{ false },
    datalang::OperatorToken{ datalang::OperatorType::Multiply },
    datalang::ScopeToken{ true },
    datalang::VariableToken{ "y" },
    datalang::OperatorToken{ datalang::OperatorType::Subtract },
    datalang::ConstantToken{ 2 },
    datalang::ScopeToken{ false },
    datalang::ScopeToken{ false }, datalang::EndOfStreamToken{} 
  } );
}

