#include "gtest/gtest.h"
#include <Lexer.h>

namespace
{
void ExpectTokenStream( const std::string& input, const std::vector<datalang::Token>& expectedTokens )
{
  datalang::Lexer lexer( input );
  for ( const auto& expectedToken : expectedTokens )
  {
    auto token = lexer.getNextToken();
    EXPECT_EQ( token, expectedToken );
    if ( !( token == expectedToken ) )
    {
      break;
    }
  }
}
} // namespace

TEST( LexerTest, SimpleConstant )
{
  std::string input = "123";
  ExpectTokenStream( input, { datalang::ConstantToken{ 123 }, datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, ConstantAddition )
{
  std::string input = "42 + 23";
  ExpectTokenStream( input,
                     { datalang::ConstantToken{ 42 },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::ConstantToken{ 23 },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, VariableMultiplication )
{
  std::string input = "myVar * anotherVar";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "myVar" },
                       datalang::OperatorToken{ datalang::OperatorType::Multiply },
                       datalang::VariableToken{ "anotherVar" },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, ParenthesesAndSubtraction )
{
  std::string input = "( x - 10 )";
  ExpectTokenStream( input,
                     { datalang::GroupingToken{ true },
                       datalang::VariableToken{ "x" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::ConstantToken{ 10 },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, ComplexExpression )
{
  std::string input = " ( var1 + 100 ) / var2 - 42 * ( var3 - 7 ) ";
  ExpectTokenStream( input,
                     { datalang::GroupingToken{ true },
                       datalang::VariableToken{ "var1" },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::ConstantToken{ 100 },
                       datalang::GroupingToken{ false },
                       datalang::OperatorToken{ datalang::OperatorType::Divide },
                       datalang::VariableToken{ "var2" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::ConstantToken{ 42 },
                       datalang::OperatorToken{ datalang::OperatorType::Multiply },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "var3" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::ConstantToken{ 7 },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, InvalidCharacters )
{
  std::string input = "42 + @invalid - $var";
  ExpectTokenStream( input,
                     { datalang::ConstantToken{ 42 },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::InvalidToken{ "@invalid" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::InvalidToken{ "$var" } } );
}

TEST( LexerTest, EmptyInput )
{
  std::string input = "   ";
  ExpectTokenStream( input, { datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, OnlyInvalidCharacters )
{
  std::string input = "@#$%";
  ExpectTokenStream( input, { datalang::InvalidToken{ "@#$%" } } );
}

TEST( LexerTest, MixedValidAndInvalid )
{
  std::string input = "x + 10 @ y - $z";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "x" },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::ConstantToken{ 10 },
                       datalang::InvalidToken{ "@" },
                       datalang::VariableToken{ "y" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::InvalidToken{ "$z" } } );
}

TEST( LexerTest, NoSpacesBetweenTokens )
{
  std::string input = "a+42*(b-7)";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "a" },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::ConstantToken{ 42 },
                       datalang::OperatorToken{ datalang::OperatorType::Multiply },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "b" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::ConstantToken{ 7 },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, LargeNumbers )
{
  std::string input = "1234567890 + 987654321";
  ExpectTokenStream( input,
                     { datalang::ConstantToken{ 1234567890 },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::ConstantToken{ 987654321 },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, SingleInvalidCharacter )
{
  std::string input = "@";
  ExpectTokenStream( input, { datalang::InvalidToken{ "@" } } );
}

TEST( LexerTest, TrailingWhitespace )
{
  std::string input = "var + 100   ";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "var" },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::ConstantToken{ 100 },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, LeadingWhitespace )
{
  std::string input = "   var - 50";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "var" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::ConstantToken{ 50 },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, ConsecutiveOperators )
{
  std::string input = "x++y--z**2";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "x" },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::VariableToken{ "y" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::VariableToken{ "z" },
                       datalang::OperatorToken{ datalang::OperatorType::Multiply },
                       datalang::OperatorToken{ datalang::OperatorType::Multiply },
                       datalang::ConstantToken{ 2 },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, EmptyParentheses )
{
  std::string input = "()";
  ExpectTokenStream(
      input, { datalang::GroupingToken{ true }, datalang::GroupingToken{ false }, datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, NestedParentheses )
{
  std::string input = "( ( x + 1 ) * ( y - 2 ) )";
  ExpectTokenStream( input,
                     { datalang::GroupingToken{ true },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "x" },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::ConstantToken{ 1 },
                       datalang::GroupingToken{ false },
                       datalang::OperatorToken{ datalang::OperatorType::Multiply },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "y" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::ConstantToken{ 2 },
                       datalang::GroupingToken{ false },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, MinTest )
{
  std::string input = "min( x , 10 )";
  ExpectTokenStream( input,
                     { datalang::MinMaxToken{ true },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "x" },
                       datalang::StatementSeparatorToken{},
                       datalang::ConstantToken{ 10 },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, MiTest )
{
  std::string input = "mi( x , 10 )mi";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "mi" },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "x" },
                       datalang::StatementSeparatorToken{},
                       datalang::ConstantToken{ 10 },
                       datalang::GroupingToken{ false },
                       datalang::VariableToken{ "mi" },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, MaxTest )
{
  std::string input = "max( y , 20 )";
  ExpectTokenStream( input,
                     { datalang::MinMaxToken{ false },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "y" },
                       datalang::StatementSeparatorToken{},
                       datalang::ConstantToken{ 20 },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, MaTest )
{
  std::string input = "ma( y , 20 )ma";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "ma" },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "y" },
                       datalang::StatementSeparatorToken{},
                       datalang::ConstantToken{ 20 },
                       datalang::GroupingToken{ false },
                       datalang::VariableToken{ "ma" },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, IfTest )
{
  std::string input = "if ( x < 10 )";
  ExpectTokenStream( input,
                     { datalang::StatementToken{ datalang::StatementType::If },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "x" },
                       datalang::OperatorToken{ datalang::OperatorType::LessThan },
                       datalang::ConstantToken{ 10 },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, ElseTest )
{
  std::string input = "else { y == 20; }";
  ExpectTokenStream( input,
                     { datalang::StatementToken{ datalang::StatementType::Else },
                       datalang::ScopeToken{ true },
                       datalang::VariableToken{ "y" },
                       datalang::OperatorToken{ datalang::OperatorType::Equal },
                       datalang::ConstantToken{ 20 },
                       datalang::EndOfStatementToken{},
                       datalang::ScopeToken{ false },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, ShiftTokens )
{
  std::string input = "x << 2 >> y";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "x" },
                       datalang::OperatorToken{ datalang::OperatorType::LSH },
                       datalang::ConstantToken{ 2 },
                       datalang::OperatorToken{ datalang::OperatorType::RSH },
                       datalang::VariableToken{ "y" },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, AllTokens )
{
  std::string input = "if (a & b | c ^ ~d == 10 || d \% f && x < 7) { max(x, y); } else { min(m, n); }";
  ExpectTokenStream( input,
                     { datalang::StatementToken{ datalang::StatementType::If },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "a" },
                       datalang::OperatorToken{ datalang::OperatorType::BitAnd },
                       datalang::VariableToken{ "b" },
                       datalang::OperatorToken{ datalang::OperatorType::BitOr },
                       datalang::VariableToken{ "c" },
                       datalang::OperatorToken{ datalang::OperatorType::BitXor },
                       datalang::OperatorToken{ datalang::OperatorType::BitNot },
                       datalang::VariableToken{ "d" },
                       datalang::OperatorToken{ datalang::OperatorType::Equal },
                       datalang::ConstantToken{ 10 },
                       datalang::OperatorToken{ datalang::OperatorType::Or },
                       datalang::VariableToken{ "d" },
                       datalang::OperatorToken{ datalang::OperatorType::Modulus },
                       datalang::VariableToken{ "f" },
                       datalang::OperatorToken{ datalang::OperatorType::And },
                       datalang::VariableToken{ "x" },
                       datalang::OperatorToken{ datalang::OperatorType::LessThan },
                       datalang::ConstantToken{ 7 },
                       datalang::GroupingToken{ false },
                       datalang::ScopeToken{ true },
                       datalang::MinMaxToken{ false },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "x" },
                       datalang::StatementSeparatorToken{},
                       datalang::VariableToken{ "y" },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStatementToken{},
                       datalang::ScopeToken{ false },
                       datalang::StatementToken{ datalang::StatementType::Else },
                       datalang::ScopeToken{ true },
                       datalang::MinMaxToken{ true },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "m" },
                       datalang::StatementSeparatorToken{},
                       datalang::VariableToken{ "n" },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStatementToken{},
                       datalang::ScopeToken{ false },
                       datalang::EndOfStreamToken{} } );
}

TEST( LexerTest, MinMaxExpressions )
{
  std::string input = "min( a + b, c - d )";
  ExpectTokenStream( input,
                     { datalang::MinMaxToken{ true },
                       datalang::GroupingToken{ true },
                       datalang::VariableToken{ "a" },
                       datalang::OperatorToken{ datalang::OperatorType::Add },
                       datalang::VariableToken{ "b" },
                       datalang::StatementSeparatorToken{},
                       datalang::VariableToken{ "c" },
                       datalang::OperatorToken{ datalang::OperatorType::Subtract },
                       datalang::VariableToken{ "d" },
                       datalang::GroupingToken{ false },
                       datalang::EndOfStreamToken{} } );
}
TEST( LexerTest, Inequality )
{
  std::string input = "x != y";
  ExpectTokenStream( input,
                     { datalang::VariableToken{ "x" },
                       datalang::OperatorToken{ datalang::OperatorType::NotEqual },
                       datalang::VariableToken{ "y" },
                       datalang::EndOfStreamToken{} } );
}