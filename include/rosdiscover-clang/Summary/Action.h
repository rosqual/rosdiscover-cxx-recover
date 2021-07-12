#pragma once

#include <unordered_set>

#include <clang/Analysis/CallGraph.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include <rosdiscover-clang/ApiCall/RosApiCall.h>
#include <rosdiscover-clang/ApiCall/Finder.h>
#include <rosdiscover-clang/Name/Symbolizer.h>

namespace rosdiscover {
namespace summary {


//class FunctionSymbolizer {
//
//  // find all of the ROS API calls within this function
//
//};


/**
 * LIMITATION: this does not CURRENTLY provide CTU results
 *
 * NOTES:
 * * we may want to limit our attention to specific files
 */

class SummaryBuilderASTConsumer : public clang::ASTConsumer {
public:

  void HandleTranslationUnit(clang::ASTContext &context) override {
    // build a summary for each (unseen) function definition
    /*
    llvm::outs() << "building summaries\n";
    auto *tu_decl = context.getTranslationUnitDecl();
    for (auto *decl : tu_decl->decls()) {
      if (auto *func_decl = dyn_cast<clang::FunctionDecl>(decl)) {
        llvm::outs() << "checking function: " << func_decl->getNameInfo().getAsString() << "\n";
      }
    }
    */

    // for now, build a call graph for this single translation unit
    // - later on we can expand this to support CTU analysis
    clang::CallGraph callGraph;
    callGraph.addToCallGraph(context.getTranslationUnitDecl());

    // step one:
    // - find all FunctionDecls that transitively perform a ROS API call
    //    - these are the architecturally relevant function calls
    //    - we need a summary for each of these function calls
    //
    //
    //

    // find all ROS API calls within this translation unit [RawRosApiCall]
    auto calls = api_call::RosApiCallFinder::find(context);

    // determine the set of functions that contain a ROS API call within this translation unit
    std::unordered_set<clang::FunctionDecl const *> containingFunctions;
    for (auto *call : calls) {
      auto *functionDecl = getParentFunctionDecl(context, call->getCallExpr());
      if (functionDecl == nullptr) {
        llvm::errs() << "failed to determine parent function for ROS API call\n";
      }
      containingFunctions.insert(functionDecl);
    }

    for (auto *functionDecl : containingFunctions) {
      llvm::outs() << "ROS API calls found in function: " << functionDecl->getNameInfo().getAsString() << "\n";
    }

    // determine the set of functions that transitively call a function with a ROS API call
    std::unordered_set<clang::FunctionDecl const *> relevantFunctions;
    for (auto *functionDecl : containingFunctions) {
      auto &callGraphNode = *callGraph.getNode(functionDecl);
      for (auto calleeRecord : callGraphNode) {
        auto *callee = dyn_cast<clang::FunctionDecl>(calleeRecord.Callee->getDecl());
        llvm::outs() << "callee: " << callee->getNameInfo().getAsString() << "\n";
      }
      llvm::outs() << "\n";
    }

    // UnsymbolizedFunction: API calls + relevant function calls
    // SymbolizedFunction: API calls [SymbolicRosApiCall] + relevant function calls
    // FunctionSummary: conditional API calls [SymbolicConditionalRosApiCall] + conditional function calls

    // build a symbolizer
    // - turn this into a FunctionSymbolizer
    // - first step: find the ROS API calls within this function
    // - second step: symbolize those ROS API calls
    // - third step: compute the path condition for each ROS API call
    // auto symbolizer = name::NameSymbolizer(context);


    // BasicFunctionSummary
    // --------------------
    // - ApiCall -> ConditionalApiCall
    // --------------------
    // - API calls [path condition]
    // - non-API calls


    // - parameter read [w/ default]



    // - group ROS API calls by their parent function decl
    // - we need to symbolize the entire group at once

    // find each ROS API call
    // - TODO: find parent function decl
    // - possibly use ASTMatcher
    // - but then use a silly visitor to get a mutable version of the same API call?
    // auto calls = api_call::RosApiCallFinder::find(context);
    // for (auto *call : calls) {
    //   call->print(llvm::outs());

    //   clang::Expr *name_expr = const_cast<clang::Expr*>(call->getNameExpr());
    //   name_expr->dumpColor();

    //   auto symbolicName = symbolizer.symbolize(name_expr);


    //   llvm::outs() << "\n\n";
    // }

    // - write a path condition builder
    //    - accepts a Clang stmt
    //    - produces a symbolic path condition in terms of formals

  }

private:
  /** Computes the set of functions that (transitively) make ROS API calls */
  std::unordered_set<clang::FunctionDecl const *> computeRelevantFunctions(
    std::unordered_set<clang::FunctionDecl const *> &containingFunctions,
    clang::CallGraph &callGraph
  ) {
    std::unordered_set<clang::FunctionDecl const *> relevantFunctions;
    for (auto *function : containingFunctions) {
      relevantFunctions.insert(function);
    }

    for (auto const &callGraphEntry : callGraph) {
      clang::FunctionDecl const *caller = dyn_cast<clang::FunctionDecl>(callGraphEntry.first);
      if (caller == nullptr) {
        continue;
      }

      clang::CallGraphNode const &callerNode = *callGraphEntry.second.get();
      for (clang::CallGraphNode::CallRecord const &callRecord : callerNode) {
        auto const *callee = dyn_cast<clang::FunctionDecl>(callRecord.Callee->getDecl());
        llvm::outs() << "cool\n";
      }
    }

    return relevantFunctions;
  };

};


class SummaryBuilderAction : public clang::ASTFrontendAction {
public:
  // TODO use pass-by-reference to store summaries
  static std::unique_ptr<clang::tooling::FrontendActionFactory> factory() {
    class Factory : public clang::tooling::FrontendActionFactory {
    public:
      std::unique_ptr<clang::FrontendAction> create() override {
        llvm::outs() << "building action...\n";
        return std::unique_ptr<clang::FrontendAction>(new SummaryBuilderAction());
      }
    };

    return std::unique_ptr<clang::tooling::FrontendActionFactory>(new Factory());
  }

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer (
    clang::CompilerInstance &compiler,
    llvm::StringRef filename
  ) override {
    llvm::outs() << "building consumer...\n";
    return std::unique_ptr<clang::ASTConsumer>(
      new SummaryBuilderASTConsumer()
    );
  }
};


} // rosdiscover::sumary
} // rosdiscover
