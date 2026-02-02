#pragma once
#include <iostream>

#include "AST.h"

namespace datalang
{
/**
 * AST Printer interface for printing the AST structure.
 */
class ASTPrinter
{
public:
  ASTPrinter( std::ostream& output = std::cout )
      : m_output( output )
  {
  }
  ~ASTPrinter() = default;

  void print( const datalang::Program& program );

private:
  std::ostream& m_output;
};

} // namespace datalang