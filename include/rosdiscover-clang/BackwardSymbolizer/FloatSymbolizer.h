#pragma once

#include <string>
#include <unordered_map>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/APValue.h>

#include "../Builder/ValueBuilder.h"
#include "../Value/Float.h"
#include "../Helper/FindDefVisitor.h"

namespace rosdiscover {

class FloatSymbolizer {
public:
  FloatSymbolizer(
  )
  : valueBuilder() {}

  std::unique_ptr<SymbolicFloat> symbolize(clang::Expr *expr) {

    if (expr == nullptr) {
      llvm::outs() << "symbolizing (float): NULLPTR";
      return valueBuilder.unknown();
    }

    expr = expr->IgnoreParenCasts();

    llvm::outs() << "symbolizing (float): ";
    expr->dump();
    llvm::outs() << "\n";

    if (clang::FloatingLiteral *literal = clang::dyn_cast<clang::FloatingLiteral>(expr)) {
      return symbolize(literal);
    } 

    llvm::outs() << "unable to symbolize expression: treating as unknown\n";
    return valueBuilder.unknown();
  }
  
  std::unique_ptr<SymbolicFloat> symbolize(clang::APValue literal) {
    if (literal.isFloat()) {
      return valueBuilder.floatingLiteral(literal.getFloat().convertToDouble());
    } else if (literal.isInt()) {
      return valueBuilder.floatingLiteral(literal.getInt().getSExtValue());
    } else {
      llvm::outs() << "unable to symbolize value: treating as unknown\n";
      return valueBuilder.unknown();
    }
  }
private:
  ValueBuilder valueBuilder;

  std::unique_ptr<SymbolicFloat> symbolize(clang::FloatingLiteral *literal) {
    return valueBuilder.floatingLiteral(literal->getValue().convertToDouble());
  }

  std::unique_ptr<SymbolicFloat> symbolize(clang::IntegerLiteral *literal) {
    return valueBuilder.floatingLiteral(literal->getValue().getSExtValue());
  }


};
} // rosdiscover
