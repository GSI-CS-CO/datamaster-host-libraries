#include "ASTInterpreter.h"
#include "Parser.h"
#include "gtest/gtest.h"

#include "StubEnvironment.h"

using namespace datalang;

namespace
{

std::unordered_map<std::string, int32_t> DEFAULT_VARIABLES = { { "x", 10 },  { "y", 20 }, { "a", 15 }, { "b", 25 },
                                                               { "c", 40 },  { "m", 5 },  { "n", 30 }, { "p", 50 },
                                                               { "q", 100 }, { "d", 2 } };

std::vector<std::pair<std::string, int32_t>> TestList = { { "10 + 20", 30 },
                                                          { "50 - 15", 35 },
                                                          { "100 / 4", 25 },
                                                          { "x / y", 0 },
                                                          { "min(a, b)", 15 },
                                                          { "max(a, b)", 25 },
                                                          { "10 / 0", 0 },
                                                          { "(x + 10) * (y - 2)", 360 },
                                                          { "a + b; b - a; a * b;", 375 },
                                                          { "5 & 3", 1 },
                                                          { "5 | 3", 7 },
                                                          { "5 ^ 3", 6 },
                                                          { "if ( x < y ) { 100; } else { 200; }", 100 },
                                                          { "if ( x > y ) { 100; } else { 200; }", 200 },
                                                          { "if ( a == 15 ) { 1; } else { 0; }", 1 },
                                                          { "if ( b != 25 ) { 1; } else { 0; }", 0 },
                                                          { "min( max( x, y ), max( a, b ) )", 20 },
                                                          { "max( min( x, y ), min( a, b ) )", 15 },
                                                          { "((a + b) * (c - d)) / x", ( ( 40 ) * ( 38 ) ) / 10 },
                                                          { "if ( ( x + y ) > ( a + b ) ) { p; } else { q; }", 100 } };

int32_t RunProgram( std::string program, StubEnvironment& env )
{
  Parser      parser;
  Lexer       lexer( std::span<const char>( program.c_str(), program.size() ) );
  TokenStream tokens;
  for ( auto nextToken = lexer.getNextToken(); !std::holds_alternative<EndOfStreamToken>( nextToken );
        nextToken      = lexer.getNextToken() )
  {
    tokens.push_back( nextToken );
  }

  tokens.push_back( EndOfStreamToken{} );
  Program        prog = parser.parse( tokens );
  ASTInterpreter interpreter( env );
  ConstantPtr    result = interpreter.interpret( prog );
  return result ? result->value : 0;
}
} // namespace

class DatalangInterpreterTestsSuite : public ::testing::TestWithParam<std::pair<std::string, int32_t>>
{
};

TEST_P( DatalangInterpreterTestsSuite, Interpretations )
{
  auto [program, expectedResult] = GetParam();
  StubEnvironment env( DEFAULT_VARIABLES );
  int32_t         result = RunProgram( program, env );
  EXPECT_EQ( result, expectedResult );
}

INSTANTIATE_TEST_SUITE_P( DatalangInterpreterTestsSuite,
                          DatalangInterpreterTestsSuite,
                          ::testing::ValuesIn( TestList ) );