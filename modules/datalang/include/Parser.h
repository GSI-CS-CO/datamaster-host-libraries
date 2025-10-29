#pragma once

#include "AST.h"
#include "Lexer.h"
#include <datalang_export.h>

namespace datalang
{

/**
 * Parser class for parsing a stream of tokens into an AST.
 */
class DATALANG_EXPORT Parser
{
public:
  Parser()  = default;
  ~Parser() = default;

public:
  /**
   * Parses the given token stream into a Program AST.
   */
  Program parse( const TokenStream& tokens );

private:
};

}; // namespace datalang