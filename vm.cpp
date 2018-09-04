#include "./vm.hpp"
#include <iostream>

using namespace pl0;

// stack layout
// 0 [ param       ]
// 1 [ param       ]
// 2 [ display pos ] <- display
// 3 [ Return addr ]
// 4 [ local val   ]
// 5 [ local val   ]
// 6 [             ]

// int lim = 0;

void VM::eval() {
  long long lhs, rhs;
  long long level, addr;
  long long display_p, before_display;
  while (pc < program.size()) {
    Instruction inst = static_cast<Instruction>(program[pc++]);
    switch (inst) {
    case Instruction::Load:
      level = program[pc++];
      addr = program[pc++];
      stack.push_back(stack[display[level] + addr]);
      break;
    case Instruction::Store:
      lhs = pop();

      level = program[pc++];
      addr = program[pc++];
      stack[display[level] + addr] = lhs;
      break;
    case Instruction::Call:
      level = program[pc++];
      addr = program[pc++];
      stack.push_back(display[level]);
      stack.push_back(pc);

      display[level] = stack.size() - 2;

      pc = addr;
      break;
    case Instruction::Ret:
      lhs = pop();
      level = program[pc++];
      display_p = display[level];
      addr = stack[display_p + 1];

      display[level] = stack[display_p];
      stack.resize(display_p + 1 - program[pc++]);
      stack.push_back(lhs);

      pc = addr;
      break;
    case Instruction::Literal:
      stack.push_back(program[pc++]);
      break;
    case Instruction::Ict:
      stack.resize(stack.size() + program[pc++]);
      break;
    case Instruction::Jmp:
      pc = program[pc];
      break;
    case Instruction::Jpc:
      addr = program[pc++];
      lhs = pop();
      if (!lhs) {
        pc = addr;
      }
      break;
    case Instruction::Neg:
      stack.back() -= 1;
      break;
    case Instruction::Add:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs + rhs);
      break;
    case Instruction::Sub:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs - rhs);
      break;
    case Instruction::Mul:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs * rhs);
      break;
    case Instruction::Div:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs / rhs);
      break;
    case Instruction::Odd:
      lhs = pop();
      stack.push_back(lhs % 2);
      break;
    case Instruction::Eq:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs == rhs);
      break;
    case Instruction::Neq:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs != rhs);
      break;
    case Instruction::Less:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs < rhs);
      break;
    case Instruction::LessEq:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs <= rhs);
      break;
    case Instruction::Greater:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs > rhs);
      break;
    case Instruction::GreaterEq:
      rhs = pop();
      lhs = pop();
      stack.push_back(lhs >= rhs);
      break;
    case Instruction::Write:
      lhs = pop();
      std::cout << lhs << std::endl;
      break;
    case Instruction::Writeln:
      std::cout << std::endl;
      break;
    }
  }
}
