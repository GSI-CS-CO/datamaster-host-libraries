#pragma once

#include "AST.h"

namespace datalang
{

/**
 * Interface for the environment that provides variable values during AST interpretation.
 */
class IASTInterpreterEnvironment
{
public:
  virtual ~IASTInterpreterEnvironment() = default;

  virtual int32_t getVariable( const std::string& name ) const = 0;
};

/**
 * AST Interpreter that evaluates the AST and produces a constant result.
 */
class ASTInterpreter
{
public:
  /**
   * Constructs an ASTInterpreter with the given environment.
   * @param env The environment to use for variable lookups.
   */
  ASTInterpreter( IASTInterpreterEnvironment& env )
      : m_env( env )
  {
  }
  ~ASTInterpreter() = default;

  /**
   * Interprets the given program AST and returns the resulting constant.
   *
   * @param root The root of the AST to interpret.
   * @return A pointer to the resulting constant, or nullptr if interpretation fails.
   */
  ConstantPtr interpret( const Program& root );

private:
  // The environment used for variable lookups during interpretation.
  IASTInterpreterEnvironment& m_env;
};
} // namespace datalang