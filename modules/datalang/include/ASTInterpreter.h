#pragma once

#include "AST.h"

namespace datalang
{

class IASTInterpreterEnvironment
{
public:
  virtual ~IASTInterpreterEnvironment() = default;

  virtual int32_t getVariable( const std::string& name ) const = 0;
};

class ASTInterpreter
{
public:
  ASTInterpreter( IASTInterpreterEnvironment& env )
      : m_env( env )
  {
  }
  ~ASTInterpreter() = default;

  // Interpret the given AST Program and return the resulting constant value
  ConstantPtr interpret( const Program& root );

private:
  IASTInterpreterEnvironment& m_env;
};
} // namespace datalang