#pragma once
#include "ASTInterpreter.h"

#include <unordered_map>

class StubEnvironment : public datalang::IASTInterpreterEnvironment
{
public:
  StubEnvironment( std::unordered_map<std::string, int32_t> vars )
      : m_variables( std::move( vars ) )
  {
  }

  // IASTInterpreterEnvironment interface
  int32_t getVariable( const std::string& name ) const override
  {
    auto it = m_variables.find( name );
    if ( it == m_variables.end() )
    {
      throw std::runtime_error( "Variable not found: " + name );
    }
    return it->second;
  }

private:
  std::unordered_map<std::string, int32_t> m_variables;
};