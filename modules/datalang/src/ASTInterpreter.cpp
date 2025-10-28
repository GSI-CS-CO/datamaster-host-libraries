#include "ASTInterpreter.h"

using namespace datalang;

namespace
{
ConstantPtr interpretExpression( const Expression& expr, IASTInterpreterEnvironment& env )
{
  return std::visit(
      [&]( auto&& e )
      {
        using T = std::decay_t<decltype( e )>;
        if constexpr ( std::is_same_v<T, ConstantPtr> )
        {
          return std::make_unique<ConstantPtr::element_type>( e->value );
        }
        else if constexpr ( std::is_same_v<T, IdentifierPtr> )
        {
          int32_t value = env.getVariable( e->name );
          return std::make_unique<ConstantPtr::element_type>( value );
        }
        else if constexpr ( std::is_same_v<T, BinaryExprPtr> )
        {
          ConstantPtr leftConst  = interpretExpression( e->left, env );
          ConstantPtr rightConst = interpretExpression( e->right, env );
          if ( !leftConst || !rightConst )
          {
            return ConstantPtr{ nullptr };
          }

          int32_t resultValue = 0;
          switch ( e->op )
          {
          case OperatorType::Add:
            resultValue = leftConst->value + rightConst->value;
            break;
          case OperatorType::Subtract:
            resultValue = leftConst->value - rightConst->value;
            break;
          case OperatorType::Multiply:
            resultValue = leftConst->value * rightConst->value;
            break;
          case OperatorType::Divide:
            if ( rightConst->value == 0 )
            {
              return ConstantPtr{ nullptr }; // Handle division by zero
            }
            resultValue = leftConst->value / rightConst->value;
            break;
          case OperatorType::Min:
            resultValue = std::min( leftConst->value, rightConst->value );
            break;
          case OperatorType::Max:
            resultValue = std::max( leftConst->value, rightConst->value );
            break;
          case OperatorType::Equal:
            resultValue = ( leftConst->value == rightConst->value ) ? 1 : 0;
            break;
          case OperatorType::NotEqual:
            resultValue = ( leftConst->value != rightConst->value ) ? 1 : 0;
            break;
          case OperatorType::LessThan:
            resultValue = ( leftConst->value < rightConst->value ) ? 1 : 0;
            break;
          case OperatorType::LessThanOrEqual:
            resultValue = ( leftConst->value <= rightConst->value ) ? 1 : 0;
            break;
          case OperatorType::GreaterThan:
            resultValue = ( leftConst->value > rightConst->value ) ? 1 : 0;
            break;
          case OperatorType::GreaterThanOrEqual:
            resultValue = ( leftConst->value >= rightConst->value ) ? 1 : 0;
            break;
          case OperatorType::BitAnd:
            resultValue = leftConst->value & rightConst->value;
            break;
          case OperatorType::BitOr:
            resultValue = leftConst->value | rightConst->value;
            break;
          case OperatorType::BitXor:
            resultValue = leftConst->value ^ rightConst->value;
            break;
          case OperatorType::LSH:
            resultValue = leftConst->value << rightConst->value;
            break;
          case OperatorType::RSH:
            resultValue = leftConst->value >> rightConst->value;
            break;
          case OperatorType::Modulus:
            if ( rightConst->value == 0 )
            {
              return ConstantPtr{ nullptr }; // Handle modulus by zero
            }
            resultValue = leftConst->value % rightConst->value;
            break;
          case OperatorType::BitNot:
            // BitNot is a unary operator; should not be here
            return ConstantPtr{ nullptr };
          case OperatorType::And:
            resultValue = ( leftConst->value && rightConst->value ) ? 1 : 0;
            break;
          case OperatorType::Or:
            resultValue = ( leftConst->value || rightConst->value ) ? 1 : 0;
            break;
          }
          return std::make_unique<ConstantPtr::element_type>( resultValue );
        }
        else if constexpr ( std::is_same_v<T, UnaryExprPtr> )
        {
          ConstantPtr operandConst = interpretExpression( e->operand, env );
          if ( !operandConst )
          {
            return ConstantPtr{ nullptr };
          }

          int32_t resultValue = 0;
          switch ( e->op )
          {
          case OperatorType::Subtract:
            resultValue = -operandConst->value;
            break;
          case OperatorType::BitNot:
            resultValue = ~operandConst->value;
            break;
          default:
            return ConstantPtr{ nullptr }; // Unsupported unary operator
          }
          return std::make_unique<ConstantPtr::element_type>( resultValue );
        }

        // Handle other expression types as needed
        return ConstantPtr{ nullptr };
      },
      expr );
}

ConstantPtr interpretStatement( const Statement& stmt, IASTInterpreterEnvironment& env )
{
  return std::visit(
      [&]( auto&& s )
      {
        using T = std::decay_t<decltype( s )>;
        if constexpr ( std::is_same_v<T, ExprPtr> )
        {
          if ( s != nullptr )
          {
            return interpretExpression( *s, env );
          }
        }
        else if constexpr ( std::is_same_v<T, IfStatementPtr> )
        {
          ConstantPtr conditionConst = interpretExpression( s->condition, env );
          if ( !conditionConst )
          {
            return ConstantPtr{ nullptr };
          }

          if ( conditionConst->value != 0 )
          {
            return interpretStatement( s->thenBranch, env );
          }
          else if ( s->elseBranch.has_value() )
          {
            return interpretStatement( s->elseBranch.value(), env );
          }
          else
          {
            return ConstantPtr{ nullptr }; // No else branch
          }
        }
        // Handle other statement types as needed
        return ConstantPtr{ nullptr };
      },

      stmt );
};

} // namespace

ConstantPtr ASTInterpreter::interpret( const Program& root )
{
  const auto numStatements = root.statements.size();
  if ( numStatements == 0 )
  {
    return nullptr;
  }

  const auto& lastStatement = root.statements[numStatements - 1];

  // Interpretation logic goes here
  return interpretStatement( lastStatement, m_env );
}