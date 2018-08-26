#pragma once

#include <vector>

namespace pl0 {
enum class Instruction {
  Load = 0,
  Store,
  Call,
  Ret,
  Literal,
  Ict,
  Jmp,
  Jpc,
  Neg,
  Add,
  Sub,
  Mul,
  Div,
  Odd,
  Eq,
  Neq,
  Less,
  LessEq,
  Greater,
  GreaterEq,
  Write,
  Writeln,
};

using Program = std::vector<long long>;
} // namespace pl0
