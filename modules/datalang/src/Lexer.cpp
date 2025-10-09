#include "Lexer.h"

using datalang::Lexer;
using datalang::Token;

Lexer::Lexer( const std::span<const char>& input )
    : input( input )
    , currentPosition( 0 )
{
}

Token Lexer::getNextToken()
{
}