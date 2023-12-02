#pragma once
#include "AST.h"
#include <cstdio>

namespace mercy {

std::unique_ptr<ASTNode> parse(FILE *In);

inline std::unique_ptr<ASTNode> parseFile(const std::string &Filename) {
  FILE *In = fopen(Filename.c_str(), "r");
  if (!In) {
    int Err = errno;
    llvm::errs() << "Failed to open file '" << Filename
                 << "': " << strerror(Err) << '\n';
    exit(1);
  }
  auto AST = parse(In);
  fclose(In);
  return AST;
}

inline std::unique_ptr<ASTNode> parseFileOrStdin(const std::string &Filename) {
  if (Filename.empty() || Filename == "-")
    return parse(stdin);
  return parseFile(Filename);
}

} // namespace mercy
