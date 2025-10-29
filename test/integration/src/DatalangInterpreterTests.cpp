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

std::vector<std::pair<std::string, int32_t>> TestList = {
  { "10 + 20", 30 },
  { "0 + 100", 100 },
  { "100 + 0", 100 },
  { "50 - 15", 35 },
  { "100 / 4", 25 },
  { "x / y", 0 },
  { "min(a, b)", 15 },
  { "max(a, b)", 25 },
  { "10 / 0", 0 },
  { "x + 10", 20 },
  { "y - 2", 18 },
  { "a * b", 375 },
  { "c / d", 20 },
  { "min(x, y)", 10 },
  { "max(x, y)", 20 },
  { "(x + 10) * (y - 2)", 360 },
  { "(10 + x) * (-2 + y)", 360 },
  { "a + b; b - a; a * b;", 375 },
  { "5 & 3", 1 },
  { "5 | 3", 7 },
  { "5 ^ 3", 6 },
  { "x < y", 1 },
  { "x > y", 0 },
  { "a == 15", 1 },
  { "b != 25", 0 },
  { "a + b", 40 },
  { "c - d", 38 },
  { "p + q", 150 },
  { "~d < 0", 1 },
  { "if ( x < y ) { 100; } else { 200; }", 100 },
  { "if ( x > y ) { 100; } else { 200; }", 200 },
  { "if ( a == 15 ) { 1; } else { 0; }", 1 },
  { "if ( b != 25 ) { 1; } else { 0; }", 0 },
  { "min( max( x, y ), max( a, b ) )", 20 },
  { "max( min( x, y ), min( a, b ) )", 15 },
  { "((a + b) * (c - d)) / x", ( ( 40 ) * ( 38 ) ) / 10 },
  { "if ( ( x + y ) > ( a + b ) ) { p; } else { q; }", 100 },
  { "if ( ( x + y ) < ( a + b ) ) { p; } else { q; }", 50 },
  { "min( a + b, c - d )", 38 },
  { "max( a * 2, b + 5 )", 30 },
  { "if ( x <= 10 ) { 123; } else { 456; }", 123 },
  { "if ( y >= 20 ) { 789; } else { 101; }", 789 },
  { "~0", -1 },
  { "~15", -16 },
  { "if ( ~d < 0 ) { 1; } else { 0; }", 1 },
  { "((x + y) * (a - b)) / d", -150 },
  { "if ( (a + b) == (c - d) ) { 1; } else { 0; }", 0 },
  { "if ( (a * 2) != (b + 5) ) { 1; } else { 0; }", 0 },
  { "50 % 3", 2 },
  { "20 % 7", 6 },
  { "if ( (x % 3) == 1 ) { 100; } else { 200; }", 100 },
  { "if ( (y % 4) == 0 ) { 300; } else { 400; }", 300 },
  { "min( a % 10, b % 10 )", 5 },
  { "max( c % 15, d % 3 )", 10 },
  { "if ( x + y > a + b ) { p; } else { q; }", 100 },
  { "if ( x + y < a + b ) { p; } else { q; }", 50 },
  { "(a + b) * (c - d) / x", 152 },
  { "if ( x * 2 <= y + 10 ) { 111; } else { 222; }", 111 },
  { "if ( y / 2 >= x - 5 ) { 333; } else { 444; }", 333 },
  { "~(a + b)", -41 },
  { "~(c - d)", -39 },
  { "if ( ~(x - y) < 0 ) { 1; } else { 0; }", 0 },
  { "if ( ~(a + b) > -100 ) { 1; } else { 0; }", 1 },
  { "if ( ~(c - d) < 0 ) { 1; } else { 0; }", 1 },

  // Left-to-right associativity tests
  { "10 - 5 - 2", 3 },
  { "20 / 4 / 2", 2 },
  { "16 / 4 * 2", 8 },
  { "20 % 7 % 4", 2 },
  { "8 << 1 >> 1", 8 },
  { "3 > 2 > 0", 1 },
  { "3 > 2 > 1", 0 },
  { "6 & 4 & 1", 0 },
  { "1 | 2 | 4", 7 },
  { "6 ^ 3 ^ 1", 4 },
  { "10 - 3 - 2 - 1", 4 },
  { "2 * 3 / 2 * 4", 12 },
  { "20 - 10 - 5", 5 },
  { "8 >> 1 << 2", 16 },
  { "2 < 4 <= 1", 1 },
  { "5 > 2 >= 3", 0 },

  // Very very large expressions
  { "1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10", 55 },
  { "10 * 9 * 8 * 7 * 6 / 5 / 4 / 3 / 2 / 1", 252 },

  // Edge cases
  { "0 + 0", 0 },
  { "0 - 0", 0 },
  { "0 * 0", 0 },
  { "0 / 1", 0 },
  { "1 / 0", 0 },
  { "min(0, 0)", 0 },
  { "max(0, 0)", 0 },
};

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