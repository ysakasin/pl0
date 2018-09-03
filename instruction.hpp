#pragma once

#include <ostream>
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

static std::ostream &operator<<(std::ostream &out, const Instruction inst) {
  switch (inst) {
  case Instruction::Load:
    return out << "Load";
  case Instruction::Store:
    return out << "Store";
  case Instruction::Call:
    return out << "Call";
  case Instruction::Ret:
    return out << "Ret";
  case Instruction::Literal:
    return out << "Literal";
  case Instruction::Ict:
    return out << "Ict";
  case Instruction::Jmp:
    return out << "Jmp";
  case Instruction::Jpc:
    return out << "Jpc";
  case Instruction::Neg:
    return out << "Neg";
  case Instruction::Add:
    return out << "Add";
  case Instruction::Sub:
    return out << "Sub";
  case Instruction::Mul:
    return out << "Mul";
  case Instruction::Div:
    return out << "Div";
  case Instruction::Odd:
    return out << "Odd";
  case Instruction::Eq:
    return out << "Eq";
  case Instruction::Neq:
    return out << "Neq";
  case Instruction::Less:
    return out << "Less";
  case Instruction::LessEq:
    return out << "LessEq";
  case Instruction::Greater:
    return out << "Greater";
  case Instruction::GreaterEq:
    return out << "GreaterEq";
  case Instruction::Write:
    return out << "Write";
  case Instruction::Writeln:
    return out << "Writeln";
  }
}

static size_t operand_size(Instruction inst) {
  switch (inst) {
  // 2
  case Instruction::Load:
  case Instruction::Store:
  case Instruction::Call:
  case Instruction::Ret:
    return 2;

  // 1
  case Instruction::Literal:
  case Instruction::Ict:
  case Instruction::Jmp:
  case Instruction::Jpc:
    return 1;

  // 0
  case Instruction::Neg:
  case Instruction::Add:
  case Instruction::Sub:
  case Instruction::Mul:
  case Instruction::Div:
  case Instruction::Odd:
  case Instruction::Eq:
  case Instruction::Neq:
  case Instruction::Less:
  case Instruction::LessEq:
  case Instruction::Greater:
  case Instruction::GreaterEq:
  case Instruction::Write:
  case Instruction::Writeln:
    return 0;
  }
}

void print_program(const Program &program);
} // namespace pl0
