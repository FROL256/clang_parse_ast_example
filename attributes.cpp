//===- Attribute.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Example clang plugin which adds an an annotation to file-scope declarations
// with the 'example' attribute.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/IR/Attributes.h"
#include "clang/Frontend/CompilerInstance.h"
using namespace clang;


namespace {

// size

struct SizeAttrInfo : public ParsedAttrInfo {
  SizeAttrInfo() {
    // Can take up to 3 optional arguments
    OptArgs = 3;
    // GNU-style __attribute__(("example")) and C++-style [[example]] and
    // [[plugin::example]] supported.
    static constexpr Spelling S[] = {{ParsedAttr::AS_CXX11, "kslicer::size"}};
    Spellings = S;
  }

  bool diagAppertainsToDecl(Sema &S, const ParsedAttr &Attr,
                            const Decl *D) const override {
    // This attribute appertains to function parameter only
    if (!isa<ParmVarDecl>(D)) {
      S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type_str) << Attr << "function parameters";
      return false;
    }
    return true;
  }

  AttrHandling handleDeclAttribute(Sema &S, Decl *D, const ParsedAttr &Attr) const override 
  {
    // Check if the decl is at file scope.
    if (!D->getDeclContext()->isFileContext()) {
      unsigned ID = S.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error, "'kslicer::size' attribute only allowed at file scope");
      S.Diag(Attr.getLoc(), ID);
      return AttributeNotApplied;
    }
    
    auto argInfo = Attr.getInfo();
    auto argNum  = Attr.getNumArgs();

    // We make some rules here:
    // 1. Only accept at most 3 arguments here.
    // 2. The first argument must be a string literal if it exists.
    if (argNum > 3) {
      unsigned ID = S.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error, "'kslicer::size' attribute only accepts at most three arguments");
      S.Diag(Attr.getLoc(), ID);
      return AttributeNotApplied;
    }

    // If there are arguments, the first argument should be a string literal.
    if (argNum > 0) {
      auto *Arg0 = Attr.getArgAsExpr(0);
      StringLiteral *Literal =
          dyn_cast<StringLiteral>(Arg0->IgnoreParenCasts());
      if (!Literal) {
        unsigned ID = S.getDiagnostics().getCustomDiagID(
            DiagnosticsEngine::Error, "first argument to the 'size' "
                                      "attribute must be a string literal");
        S.Diag(Attr.getLoc(), ID);
        return AttributeNotApplied;
      }
      SmallVector<Expr *, 16> ArgsBuf;
      for (unsigned i = 0; i < Attr.getNumArgs(); i++) {
        ArgsBuf.push_back(Attr.getArgAsExpr(i));
      }
      D->addAttr(AnnotateAttr::Create(S.Context, "size", ArgsBuf.data(), ArgsBuf.size(), Attr.getRange()));
    } else {
      // Attach an annotate attribute to the Decl.
      D->addAttr(AnnotateAttr::Create(S.Context, "size", nullptr, 0, Attr.getRange()));
    }
    return AttributeApplied;
  }
};



} // namespace

static ParsedAttrInfoRegistry::Add<SizeAttrInfo> G_SIZE_ATTR("size", "");
