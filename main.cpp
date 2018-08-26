#include <iostream>

#include "./lexer.hpp"
#include "./token.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "error: no input file" << std::endl;
    exit(1);
  }

  pl0::Lexer lexer(argv[1]);
  lexer.print_all();
}
