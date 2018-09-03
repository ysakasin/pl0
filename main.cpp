#include <iostream>

#include "./compiler.hpp"
#include "./token.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: no input file" << std::endl;
    exit(1);
  }

  // pl0::Lexer lexer(argv[1]);
  // lexer.print_all();
  pl0::Compiler compiler(argv[1]);
  auto program = compiler.compile();
  pl0::print_program(program);
}
