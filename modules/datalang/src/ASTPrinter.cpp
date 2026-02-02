#include "ASTPrinter.h"
#include "magic_enum/magic_enum.hpp"
#include <iostream>

using namespace datalang;

namespace
{

void printExpression( const Expression& expr, int indentLevel, std::ostream& output )
{
  std::string indent( indentLevel * 2, ' ' );
  std::visit(
      [&]( auto&& e )
      {
        using T = std::decay_t<decltype( e )>;
        if constexpr ( std::is_same_v<T, ConstantPtr> )
        {
          output << indent << "Constant: " << e->value << std::endl;
        }
        else if constexpr ( std::is_same_v<std::decay_t<decltype( e )>, IdentifierPtr> )
        {
          output << indent << "Identifier: " << e->name << std::endl;
        }
        else if constexpr ( std::is_same_v<std::decay_t<decltype( e )>, BinaryExprPtr> )
        {
          output << indent << "Binary Expression:" << std::endl;
          output << indent << " Left:" << std::endl;
          printExpression( e->left, indentLevel + 1, output );
          output << indent << " Operator: " << magic_enum::enum_name( e->op ) << std::endl;
          output << indent << " Right:" << std::endl;
          printExpression( e->right, indentLevel + 1, output );
        }
        else
        {
          output << indent << "Expression: " << e << std::endl;
        }
      },
      expr );
}

void printStatement( const Statement& stmt, int indentLevel, std::ostream& output )
{
  std::string indent( indentLevel * 2, ' ' );
  std::visit(
      [&]( auto&& s )
      {
        using T = std::decay_t<decltype( s )>;
        if constexpr ( std::is_same_v<T, ExprPtr> )
        {
          output << indent << "Expression Statement:" << std::endl;
          printExpression( *s, indentLevel + 1, output );
        }
        else if constexpr ( std::is_same_v<T, IfStatementPtr> )
        {
          output << indent << "If Statement:" << std::endl;
          output << indent << " Condition:" << std::endl;
          printExpression( s->condition, indentLevel + 1, output );
          output << indent << " Then Branch:" << std::endl;
          printStatement( s->thenBranch, indentLevel + 1, output );
          if ( s->elseBranch.has_value() )
          {
            output << indent << " Else Branch:" << std::endl;
            printStatement( s->elseBranch.value(), indentLevel + 1, output );
          }
        }
      },
      stmt );
}
} // namespace

void ASTPrinter::print( const datalang::Program& program )
{
  auto statementCount = program.statements.size();
  m_output << "Program with " << statementCount << " statements." << std::endl;
  for ( size_t i = 0; i < statementCount; ++i )
  {
    m_output << " Statement " << i + 1 << ":" << std::endl;
    // For simplicity, we just indicate the type of statement here.

    const auto& stmt = program.statements[i];
    printStatement( stmt, 1, m_output );
  }

  m_output << "End of program." << std::endl;
}