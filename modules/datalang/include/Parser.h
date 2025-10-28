#pragma once

#include "AST.h"
#include "Lexer.h"
#include <datalang_export.h>

namespace datalang
{

class DATALANG_EXPORT Parser
{
public:
  Parser()  = default;
  ~Parser() = default;

public:
  Program parse( const TokenStream& tokens );

private:
};

}; // namespace datalang