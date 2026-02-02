#pragma once
#include "Lexer.h"

#include <memory>

/**
Program        ::= StatementList
StatementList  ::= { Statement }

Statement      ::= IfStatement
                 | Block
                 | ExprStatement

IfStatement    ::= IF LPAREN Expression RPAREN Statement ElsePart
ElsePart       ::= ELSE Statement | ε

Block          ::= LBRACE StatementList RBRACE

ExprStatement  ::= Expression SEMICOLON

Expression     ::= BitwiseOr
                 | ε

BitwiseOr      ::= BitwiseXor BitwiseOrTail
BitwiseOrTail  ::= PIPE BitwiseXor BitwiseOrTail
                 | ε

BitwiseXor     ::= BitwiseAnd BitwiseXorTail
BitwiseXorTail ::= CARET BitwiseAnd BitwiseXorTail
                 | ε

BitwiseAnd     ::= Equality BitwiseAndTail
BitwiseAndTail ::= AMP Equality BitwiseAndTail
                 | ε

Equality       ::= Relational EqualityTail
EqualityTail   ::= EQ Relational EqualityTail
               | NEQ Relational EqualityTail
               | ε

Relational     ::= ShiftExpr RelationalTail
RelationalTail ::= LT ShiftExpr RelationalTail
               | LE ShiftExpr RelationalTail
               | GT ShiftExpr RelationalTail
               | GE ShiftExpr RelationalTail
               | ε

ShiftExpr      ::= AddExpr ShiftExprTail
ShiftExprTail  ::= LSH AddExpr ShiftExprTail
               | RSH AddExpr ShiftExprTail
               | ε

AddExpr        ::= MulExpr AddExprTail
AddExprTail    ::= PLUS MulExpr AddExprTail
               | MINUS MulExpr AddExprTail
               | ε

MulExpr        ::= UnaryExpr MulExprTail
MulExprTail    ::= STAR UnaryExpr MulExprTail
               | SLASH UnaryExpr MulExprTail
               | PERCENT UnaryExpr MulExprTail
               | ε

UnaryExpr      ::= UnaryOp UnaryExpr
               | Primary

UnaryOp        ::= MINUS | TILDE

Primary        ::= IDENT
               | CONST
               | LPAREN Expression RPAREN
               | MinMaxCall

MinMaxCall     ::= MIN_KW LPAREN Expression COMMA Expression RPAREN
               | MAX_KW LPAREN Expression COMMA Expression RPAREN


INDENT          ::= '\t'
CONST           ::= '[0-9]+'
PLUS            ::= '+'
MINUS           ::= '-'
STAR            ::= '*'
SLASH           ::= '/'
PERCENT         ::= '%'
MIN_KW          ::= 'min'
MAX_KW          ::= 'max'
LSH             ::= '<<'
RSH             ::= '>>'
AMP             ::= '&'
PIPE            ::= '|'
CARET           ::= '^'
TILDE           ::= '~'
EQ              ::= '=='
NEQ             ::= '!='
LT              ::= '<'
LE              ::= '<='
GT              ::= '>'
GE              ::= '>='
IF              ::= 'if'
ELSE            ::= 'else'
LPAREN          ::= '('
RPAREN          ::= ')'
LBRACE          ::= '{'
RBRACE          ::= '}'
SEMICOLON       ::= ';'
# COMMA           ::= ','

*/

namespace datalang
{

struct BinaryExpression;
struct UnaryExpr;
using BinaryExprPtr = std::unique_ptr<BinaryExpression>;
using UnaryExprPtr  = std::unique_ptr<UnaryExpr>;

/**
 * Identifier expression representing a variable.
 */
struct Identifier
{
  std::string name;
};
using IdentifierPtr = std::unique_ptr<Identifier>;

/**
 * Constant expression representing an integer constant.
 */
struct Constant
{
  int32_t value;
};

using ConstantPtr = std::unique_ptr<Constant>;

using Expression = std::variant<UnaryExprPtr, BinaryExprPtr, IdentifierPtr, ConstantPtr>;

/**
 * Binary expression representing operations with two operands.
 */
struct BinaryExpression
{
  Expression   left;
  OperatorType op;
  Expression   right;
};

/**
 * Unary expression representing operations with a single operand.
 */
struct UnaryExpr
{
  OperatorType op;
  Expression   operand;
};

struct IfStatement;
struct Block;

using IfStatementPtr = std::unique_ptr<IfStatement>;
using ExprPtr        = std::unique_ptr<Expression>;

using Statement = std::variant<IfStatementPtr, ExprPtr>;

/**
 * If statement representing conditional execution.
 */
struct IfStatement
{
  ~IfStatement() = default;
  Expression               condition;
  Statement                thenBranch;
  std::optional<Statement> elseBranch;
};

/**
 * Program representing the root of the AST.
 */
struct Program
{
  std::vector<Statement> statements;
};

inline bool operator==(Program const& lhs, Program const& rhs)
{
  return lhs.statements == rhs.statements;
}

} // namespace datalang
