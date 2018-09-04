#pragma once

#include <string>
#include <vector>

#include "./instruction.hpp"
#include "./lexer.hpp"
#include "./table.hpp"
#include "./token.hpp"

namespace pl0 {
class Compiler {
public:
  Compiler(const std::string &path) : lexer(path) {
    cur_token = std::move(lexer.nextToken());
    peek_token = std::move(lexer.nextToken());
  }
  Program compile();

private:
  void block(size_t func_id);
  void constDecl();
  void varDecl(size_t *var_size);
  void functionDecl();
  void statement();
  void condition();
  void expression();
  void term();
  void factor();

private:
  size_t append(Instruction instruction);
  size_t append(Instruction instruction, long long value);
  size_t append(Instruction instruction, long long first, long long second);
  size_t append(long long value);
  void backpatch(size_t target);

  void nextToken();
  void takeToken(TokenType type);

private:
  Lexer lexer;
  Program program;
  Table ident_table;

  Token cur_token;
  Token peek_token;
  size_t cur_func_id;
};
} // namespace pl0
