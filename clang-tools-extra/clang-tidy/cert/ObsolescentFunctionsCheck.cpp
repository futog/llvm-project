//===--- ObsolescentFunctionsCheck.cpp - clang-tidy -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ObsolescentFunctionsCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include <cassert>

using namespace clang::ast_matchers;
using namespace llvm;

namespace clang {
namespace tidy {
namespace cert {

static constexpr llvm::StringLiteral FunctionNamesExprName =
    "FunctionNamesExpr";
static constexpr llvm::StringLiteral
    FunctionNamesWithAnnexKReplacementExprName =
        "FunctionNamesWithAnnexKReplacementExpr";

static constexpr llvm::StringLiteral DeclRefExprName = "DeclRefExpr";

static StringRef getRationale(StringRef FunctionName) {
  return StringSwitch<StringRef>(FunctionName)
      .Cases("asctime", "ctime", "is non-reentrant")
      .Cases("fopen", "freopen", "has no exclusive access to file")
      .Cases("rewind", "setbuf", "has no error detection")
      .Default("is obsolescent");
}

void ObsolescentFunctionsCheck::registerMatchers(MatchFinder *Finder) {

  // Matching functions that can be checked without annex K available.
  auto FunctionNamesMatcher = hasAnyName("::gets", "::rewind", "::setbuf");

  // Matching functions that have safe replacements in annex K.
  auto FunctionNamesWithAnnexKReplacementMatcher = hasAnyName(
      "::asctime", "::ctime", "::fopen", "::freopen", "::bsearch", "::fprintf",
      "::fscanf", "::fwprintf", "::fwscanf", "::getenv", "::gmtime",
      "::localtime", "::mbsrtowcs", "::mbstowcs", "::memcpy", "::memmove",
      "::printf", "::qsort", "::snprintf", "::sprintf", "::sscanf", "::strcat",
      "::strcpy", "::strerror", "::strncat", "::strncpy", "::strtok",
      "::swprintf", "::swscanf", "::vfprintf", "::vfscanf", "::vfwprintf",
      "::vfwscanf", "::vprintf", "::vscanf", "::vsnprintf", "::vsprintf",
      "::vsscanf", "::vswprintf", "::vswscanf", "::vwprintf", "::vwscanf",
      "::wcrtomb", "::wcscat", "::wcscpy", "::wcsncat", "::wcsncpy",
      "::wcsrtombs", "::wcstok", "::wcstombs", "::wctomb", "::wmemcpy",
      "::wmemmove", "::wprintf", "::wscanf");

  Finder->addMatcher(
      declRefExpr(
          to(anyOf(
              functionDecl(FunctionNamesMatcher).bind(FunctionNamesExprName),
              functionDecl(FunctionNamesWithAnnexKReplacementMatcher)
                  .bind(FunctionNamesWithAnnexKReplacementExprName))))
          .bind(DeclRefExprName),
      this);
}

void ObsolescentFunctionsCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *DeclRef = Result.Nodes.getNodeAs<DeclRefExpr>(DeclRefExprName);
  assert(DeclRef && "No matching declaration reference found.");

  SourceLocation Loc = DeclRef->getBeginLoc();
  // FIXME: I'm not sure if this can ever happen, but to be on the safe side,
  // validity is checked.
  if (Loc.isInvalid())
    return;

  StringRef FunctionName;
  Optional<std::string> ReplacementFunctionName;

  if (const auto *FuncDecl =
          Result.Nodes.getNodeAs<FunctionDecl>(FunctionNamesExprName)) {
    FunctionName = FuncDecl->getName();

    if (FunctionName == "gets") {
      diag(Loc, "function 'gets' is deprecated as of C99, removed from C11.")
          << SourceRange(Loc, DeclRef->getEndLoc());
      return;
    }

    ReplacementFunctionName =
        StringSwitch<Optional<std::string>>(FunctionName)
            .Case("rewind", Optional<std::string>{"fseek"})
            .Case("setbuf", Optional<std::string>{"setvbuf"})
            .Default(None);
  } else if (const auto *FuncDecl = Result.Nodes.getNodeAs<FunctionDecl>(
                 FunctionNamesWithAnnexKReplacementExprName)) {
    FunctionName = FuncDecl->getName();

    if (useSafeFunctionsFromAnnexK())
      ReplacementFunctionName = (Twine{FunctionName} + "_s").str();
  }

  if (!ReplacementFunctionName.hasValue())
    return;

  assert(!FunctionName.empty() && "No matching function name");

  diag(Loc, "function '%0' %1; '%2' should be used instead.")
      << FunctionName << getRationale(FunctionName)
      << ReplacementFunctionName.getValue()
      << SourceRange(Loc, DeclRef->getEndLoc());
}

void ObsolescentFunctionsCheck::registerPPCallbacks(
    const SourceManager &SM, Preprocessor *PP, Preprocessor *ModuleExpanderPP) {
  ObsolescentFunctionsCheck::PP = PP;
}

bool ObsolescentFunctionsCheck::useSafeFunctionsFromAnnexK() const {
  if (IsAnnexKAvailable.hasValue())
    return IsAnnexKAvailable.getValue();

  assert(PP && "No Preprocessor registered.");

  if (!PP->isMacroDefined("__STDC_LIB_EXT1__") ||
      !PP->isMacroDefined("__STDC_WANT_LIB_EXT1__")) {
    // Caching the result.
    return (IsAnnexKAvailable = false).getValue();
  }

  const auto *MI =
      PP->getMacroInfo(PP->getIdentifierInfo("__STDC_WANT_LIB_EXT1__"));
  if (!MI || MI->tokens_empty()) {
    // Caching the result.
    return (IsAnnexKAvailable = false).getValue();
  }

  const Token &T = MI->tokens().back();
  if (!T.isLiteral() || !T.getLiteralData()) {
    // Caching the result.
    return (IsAnnexKAvailable = false).getValue();
  }

  // Caching the result.
  IsAnnexKAvailable = StringRef(T.getLiteralData(), T.getLength()) == "1";
  return IsAnnexKAvailable.getValue();
}

} // namespace cert
} // namespace tidy
} // namespace clang
