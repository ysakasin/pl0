#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "./error.hpp"
#include "./lexer.hpp"
#include "./llvm_table.hpp"
#include "./token.hpp"

namespace pl0 {
class Frontend {
public:
  Frontend(const std::string &path);
  ~Frontend() { delete module; }

  void compile();
  llvm::Module *getModule() { return module; }

public:
  void block(llvm::Function *func);

  void constDecl();
  void varDecl(std::vector<std::string> *vars);
  void functionDecl();
  void statement();
  void statementAssign();
  void statementIf();
  void statementWhile();

  llvm::Value *condition();
  llvm::Value *expression();
  llvm::Value *term();
  llvm::Value *factor();
  llvm::Value *factorIdent();

private:
  void nextToken() {
    cur_token = std::move(peek_token);
    peek_token = std::move(lexer.nextToken());
  }

  void takeToken(TokenType type) {
    if (cur_token.type != type) {
      parseError(type, cur_token.type);
    }
    nextToken();
  }

private:
  llvm::LLVMContext context;
  llvm::Module *module;
  llvm::IRBuilder<> builder;

  llvm::Function *curFunc;
  llvm::Function *writeFunc;
  llvm::Function *writelnFunc;

  Lexer lexer;

  Token cur_token;
  Token peek_token;
  pl0llvm::Table ident_table;
};
} // namespace pl0
