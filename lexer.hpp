#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace pl0 {
class Token;
enum class TokenType;

class Lexer {
public:
  Lexer(const std::string &path);
  Token nextToken();
  Token take(TokenType type);
  void untake(Token &&token);
  void print_all();
  void print_head();

private:
  void skip_blank();
  char peekc() { return source_program[head]; }
  char readc() { return source_program[head++]; }
  bool try_readc(char c);

  Token read_number();
  Token read_ident();

private:
  static std::map<std::string, TokenType> keywords;
  static void init_keywords();

private:
  std::string source_program;
  std::string path;
  size_t head = 0;
  std::vector<Token> buffer;
};
} // namespace pl0
