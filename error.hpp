#pragma once

#include <iostream>
#include <string>

#include "./token.hpp"

static void unsupportedError(const std::string &syntax) {
  std::cerr << "error: " << syntax << " is unsupported" << std::endl;
  exit(1);
}

static void undefinedError(const std::string &ident) {
  std::cerr << "undefined error: " << ident << " is undefined" << std::endl;
  exit(1);
}

static void parseError(pl0::TokenType expect, pl0::TokenType actual) {
  std::cerr << "parse error: " << expect << " is expected, but actual is "
            << actual << std::endl;
  exit(1);
}

static void error(const std::string &msg) {
  std::cerr << "error: " << msg << std::endl;
  exit(1);
}
