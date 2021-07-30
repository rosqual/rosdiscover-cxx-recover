#include <iostream>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include <rosdiscover-clang/Symbolic/ProgramSymbolizer.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace rosdiscover::symbolic;

static llvm::cl::OptionCategory MyToolCategory("rosdiscover options");
static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

static llvm::cl::opt<std::string> outputFilename(
  "output-filename",
  llvm::cl::desc("the name of the file to which the node summary should be written."),
  llvm::cl::value_desc("filename"),
  llvm::cl::init("node-summary.json")
);

int main(int argc, const char **argv) {
  CommonOptionsParser optionsParser(argc, argv, MyToolCategory);

  auto program = ProgramSymbolizer::symbolize(optionsParser.getCompilations(), optionsParser.getSourcePathList());
  auto json = program->toJson();
  std::cout << std::setw(2) << json;

  // save the program to disk
  program->save(outputFilename);

  return 0;
}
