#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
#include <system_error>
#include <iostream>
#include <fstream>

#include <unordered_map>

#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"

using namespace clang;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class InitialPassRecursiveASTVisitor : public RecursiveASTVisitor<InitialPassRecursiveASTVisitor>
{
public:
  
  std::string MAIN_NAME;
  std::string MAIN_CLASS_NAME;

  InitialPassRecursiveASTVisitor(std::string main_name, std::string main_class, const ASTContext& a_astContext, clang::SourceManager& a_sm) : 
                                 MAIN_NAME(main_name), MAIN_CLASS_NAME(main_class), m_astContext(a_astContext), m_sourceManager(a_sm)  { }
  
  bool VisitCXXMethodDecl(CXXMethodDecl* f);

private:

  const ASTContext&     m_astContext;
  clang::SourceManager& m_sourceManager;
};
  
class InitialPassASTConsumer : public ASTConsumer
{
 public:

  InitialPassASTConsumer(std::string main_name, std::string main_class, const ASTContext& a_astContext, clang::SourceManager& a_sm) : rv(main_name, main_class, a_astContext, a_sm) { }
  bool HandleTopLevelDecl(DeclGroupRef d) override;
  InitialPassRecursiveASTVisitor rv;
};


bool InitialPassASTConsumer::HandleTopLevelDecl(DeclGroupRef d)
{
  typedef DeclGroupRef::iterator iter;
  for (iter b = d.begin(), e = d.end(); b != e; ++b)
    rv.TraverseDecl(*b);
  return true; // keep going
}


bool InitialPassRecursiveASTVisitor::VisitCXXMethodDecl(CXXMethodDecl* f) 
{
  return true; // returning false aborts the traversal
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char **argv)
{
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  struct stat sb;
  
  std::string fileName = "data/input.cpp";
  llvm::ArrayRef<const char*> args(argv, argv+argc);

  // Make sure it exists
  if (stat(fileName.c_str(), &sb) == -1)
  {
    perror(fileName.c_str());
    exit(EXIT_FAILURE);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  CompilerInstance compiler;
  DiagnosticOptions diagnosticOptions;
  compiler.createDiagnostics();  //compiler.createDiagnostics(argc, argv);

  // Create an invocation that passes any flags to preprocessor
  std::shared_ptr<CompilerInvocation> Invocation = std::make_shared<CompilerInvocation>();
  CompilerInvocation::CreateFromArgs(*Invocation, args, compiler.getDiagnostics());
  compiler.setInvocation(Invocation);

  // Set default target triple
  std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
  pto->Triple     = llvm::sys::getDefaultTargetTriple();
  TargetInfo *pti = TargetInfo::CreateTargetInfo(compiler.getDiagnostics(), pto);
  compiler.setTarget(pti);
  
  {
    compiler.getLangOpts().GNUMode = 1; 
    compiler.getLangOpts().RTTI        = 1; 
    compiler.getLangOpts().Bool        = 1; 
    compiler.getLangOpts().CPlusPlus   = 1; 
    compiler.getLangOpts().CPlusPlus11 = 1;
    compiler.getLangOpts().CPlusPlus14 = 1;
    compiler.getLangOpts().CPlusPlus17 = 1;
  }
  compiler.createFileManager();
  compiler.createSourceManager(compiler.getFileManager());
  
  compiler.createPreprocessor(clang::TU_Complete);
  compiler.getPreprocessorOpts().UsePredefines = false;
  compiler.createASTContext();

  const FileEntry *pFile = compiler.getFileManager().getFile(fileName).get();
  compiler.getSourceManager().setMainFileID( compiler.getSourceManager().createFileID( pFile, clang::SourceLocation(), clang::SrcMgr::C_User));
  compiler.getDiagnosticClient().BeginSourceFile(compiler.getLangOpts(), &compiler.getPreprocessor());

  std::cout << "Parse and Traverse AST with AST consumer ... " << std::endl; 
  
  InitialPassASTConsumer astConsumer("MainFunc", "TestClass", compiler.getASTContext(), compiler.getSourceManager());  
  ParseAST(compiler.getPreprocessor(), &astConsumer, compiler.getASTContext());
  compiler.getDiagnosticClient().EndSourceFile();

  return 0;
}