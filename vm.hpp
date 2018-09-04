#pragma once

#include "./instruction.hpp"
#include <vector>

namespace pl0 {
class VM {
public:
  VM(const Program &program) : program(program) {
    stack.push_back(0);
    stack.push_back(program.size());
  };
  void eval();

private:
  long long pop() {
    long long x = stack.back();
    stack.pop_back();
    return x;
  }

private:
  Program program;
  size_t pc;

  std::vector<long long> stack;
  size_t top;
  long long display[100];
};
} // namespace pl0
