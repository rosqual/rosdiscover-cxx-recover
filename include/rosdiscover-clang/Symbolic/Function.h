#pragma once

#include <nlohmann/json.hpp>

#include "../Value/Value.h"
#include "../Variable/LocalVariable.h"
#include "../Variable/Parameter.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {


class SymbolicFunction {
public:
  void print(llvm::raw_ostream &os) const {
    os << "function " << qualifiedName << " [";
    for (auto const &paramEntry : parameters) {
      paramEntry.second.print(os);
      os << "; ";
    }
    os << "] ";
    body->print(os);
  }

  nlohmann::json toJson() const {
    auto jsonParams = nlohmann::json::array();
    for (auto const &entry : parameters) {
      jsonParams.push_back(entry.second.toJson());
    }

    return {
      {"name", qualifiedName},
      {"parameters", jsonParams},
      {"source-location", location},
      {"body", body.get()->toJson()}
    };
  }

  void define(std::unique_ptr<SymbolicCompound> body) {
    this->body = std::move(body);
  }

  LocalVariable& createLocal(std::string const &name, SymbolicValueType const &type) {
    locals.emplace_back(name, type);
    return locals.back();
  }

  std::string getName() const {
    return qualifiedName;
  }

  static SymbolicFunction* create(
      clang::ASTContext const &context,
      clang::FunctionDecl const *function
  ) {
    auto qualifiedName = function->getQualifiedNameAsString();
    auto location = function->getLocation().printToString(context.getSourceManager());
    auto symbolic = new SymbolicFunction(qualifiedName, location);

    // TODO check whether this is the "main" function
    auto numParams = function->getNumParams();
    for (size_t paramIndex = 0; paramIndex < numParams; ++paramIndex) {
      symbolic->addParam(paramIndex, function->getParamDecl(paramIndex));
    }

    return symbolic;
  }

private:
  std::string qualifiedName;
  std::string location;
  std::unique_ptr<SymbolicCompound> body;
  std::unordered_map<size_t, Parameter> parameters;
  std::vector<LocalVariable> locals;

  SymbolicFunction(
    std::string const &qualifiedName,
    std::string const &location
  ) : qualifiedName(qualifiedName),
      location(location),
      body(std::make_unique<SymbolicCompound>()),
      parameters(),
      locals()
  {}

  void addParam(size_t index, clang::ParmVarDecl const *param) {
    // TODO does this have a default?
    auto name = param->getNameAsString();
    auto type = SymbolicValue::getSymbolicType(param->getOriginalType());

    if (type == SymbolicValueType::Unsupported)
      return;

    addParam(Parameter(index, name, type));
  }

  void addParam(Parameter const &param) {
    parameters.emplace(param.getIndex(), param);
  }
};

// TODO record symbolic function call arguments
class SymbolicFunctionCall : public virtual SymbolicStmt {
public:
  SymbolicFunctionCall(SymbolicFunction *callee) : callee(callee) {}
  ~SymbolicFunctionCall(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(call " << callee->getName() << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "call"},
      {"callee", callee->getName()}
    };
  }

private:
  SymbolicFunction *callee;
};

} // rosdiscover::symbolic
} // rosdiscover
