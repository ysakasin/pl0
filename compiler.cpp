#include <cassert>
#include <string>

#include "./compiler.hpp"
#include "./token.hpp"

using namespace pl0;

Program Compiler::compile() {
  ident_table.appendFunc("main", 0, 0);
  block(0);
  return program;
}

void Compiler::block(size_t func_id) {
  size_t var_size = 0;
  size_t backpatch_target = append(Instruction::Jmp, 0);
  while (true) {
    if (cur_token.type == TokenType::Const) {
      constDecl();
    } else if (cur_token.type == TokenType::Var) {
      varDecl(&var_size);
    } else if (cur_token.type == TokenType::Function) {
      functionDecl();
    } else {
      break;
    }
  }
  backpatch(backpatch_target);

  append(Instruction::Ict, var_size);

  cur_func_id = func_id;
  statement();
}

void Compiler::constDecl() {
  takeToken(TokenType::Const);
  while (true) {
    if (cur_token.type != TokenType::Ident) {
      throw "expected ident";
    }

    std::string const_name = cur_token.ident;
    nextToken();

    if (cur_token.type != TokenType::Equal) {
      throw "expected equal";
    }
    nextToken();

    if (cur_token.type != TokenType::Integer) {
      throw "expected integer";
    }

    ident_table.appendConst(const_name, cur_token.integer);
    nextToken();

    if (cur_token.type == TokenType::Colon) {
      nextToken();
      // continue;
    } else if (cur_token.type == TokenType::Semicolon) {
      nextToken();
      break;
    } else {
      throw "unexpected at constDecl";
    }
  }
}

void Compiler::varDecl(size_t *var_size) {
  takeToken(TokenType::Var);
  while (true) {
    if (cur_token.type != TokenType::Ident) {
      throw "expected ident";
    }

    ident_table.appendVar(cur_token.ident);
    (*var_size)++;
    nextToken();

    if (cur_token.type == TokenType::Colon) {
      nextToken();
      // continue;
    } else if (cur_token.type == TokenType::Semicolon) {
      nextToken();
      break;
    } else {
      throw "unexpected at varDecl";
    }
  }
}

void Compiler::functionDecl() {
  takeToken(TokenType::Function);
  if (cur_token.type != TokenType::Ident) {
    throw "expected ident but";
  }
  std::string func_name = std::move(cur_token.ident);
  nextToken();

  size_t entry_point = program.size();
  size_t backpatch_target = append(Instruction::Jmp, 0);

  std::vector<std::string> params;
  takeToken(TokenType::ParenL);
  while (true) {
    if (cur_token.type != TokenType::Ident) {
      break;
    }

    params.push_back(cur_token.ident);
    nextToken();
    if (cur_token.type == TokenType::Colon) {
      nextToken();
      // continue;
    } else {
      break;
    }
  }
  takeToken(TokenType::ParenR);

  size_t func_id =
      ident_table.appendFunc(func_name, entry_point, params.size());

  ident_table.enterBlock();
  long long offset = -params.size();
  for (const auto &param : params) {
    ident_table.appendParam(param, offset++);
  }

  backpatch(backpatch_target);
  block(func_id);
  takeToken(TokenType::Semicolon);
  append(Instruction::Ret, ident_table.getLevel(), params.size());
  ident_table.leaveBlock();
}

void Compiler::statement() {
  size_t backpatch_target;
  size_t start_at;
  const IdInfo *info;

  switch (cur_token.type) {
  case TokenType::Ident:
    info = &ident_table.find(cur_token.ident);
    nextToken();
    takeToken(TokenType::Assign);
    expression();
    append(Instruction::Store, info->level, info->addr);
    break;
  case TokenType::Begin:
    nextToken();
    while (true) {
      statement();
      if (cur_token.type == TokenType::Semicolon) {
        takeToken(TokenType::Semicolon);
        // continue;
      } else if (cur_token.type == TokenType::End) {
        takeToken(TokenType::End);
        break;
      } else {
        lexer.print_head();
        std::cout << cur_token << std::endl;
        std::cout << peek_token << std::endl;
        throw "expect semicolon or end but not";
      }
    }
    break;
  case TokenType::If:
    nextToken();

    condition();
    takeToken(TokenType::Then);
    backpatch_target = append(Instruction::Jpc, 0);
    statement();
    backpatch(backpatch_target);
    break;
  case TokenType::While:
    nextToken();

    start_at = program.size();
    condition();
    takeToken(TokenType::Do);
    backpatch_target = append(Instruction::Jpc, 0);
    statement();
    append(Instruction::Jmp, start_at);
    backpatch(backpatch_target);
    break;
  case TokenType::Return:
    nextToken();
    expression();
    info = &ident_table.get(cur_func_id);
    append(Instruction::Ret, ident_table.getLevel(), info->param_size);
    break;
  case TokenType::Write:
    nextToken();
    expression();
    append(Instruction::Write);
    break;
  case TokenType::Writeln:
    nextToken();
    append(Instruction::Writeln);
    break;
  default:;
  }
}

Instruction token_to_inst(TokenType type) {
  switch (type) {
  case TokenType::Equal:
    return Instruction::Eq;
  case TokenType::NotEqual:
    return Instruction::Neq;
  case TokenType::Less:
    return Instruction::Less;
  case TokenType::LessEqual:
    return Instruction::LessEq;
  case TokenType::Greater:
    return Instruction::Greater;
  case TokenType::GreaterEqual:
    return Instruction::GreaterEq;
  default:
    throw "not support at token to inst";
  }
}

void Compiler::condition() {
  if (cur_token.type == TokenType::Odd) {
    nextToken();
    expression();
    append(Instruction::Odd);
  } else {
    expression();
    Instruction inst = token_to_inst(cur_token.type);
    nextToken();
    expression();
    append(inst);
  }
}

void Compiler::expression() {
  TokenType sign = cur_token.type;
  if (cur_token.type == TokenType::Plus || cur_token.type == TokenType::Minus) {
    nextToken();
  }

  term();
  if (sign == TokenType::Minus) {
    append(Instruction::Neg);
  }

  while (true) {
    if (cur_token.type == TokenType::Plus) {
      nextToken();
      term();
      append(Instruction::Add);
    } else if (cur_token.type == TokenType::Minus) {
      nextToken();
      term();
      append(Instruction::Sub);
    } else {
      break;
    }
  }
}

void Compiler::term() {
  factor();
  while (true) {
    if (cur_token.type == TokenType::Mul) {
      nextToken();
      factor();
      append(Instruction::Mul);
    } else if (cur_token.type == TokenType::Div) {
      nextToken();
      factor();
      append(Instruction::Div);
    } else {
      break;
    }
  }
}

void Compiler::factor() {
  if (cur_token.type == TokenType::Ident) {
    const auto &info = ident_table.find(cur_token.ident);
    nextToken();
    size_t params = 0;
    switch (info.type) {
    case IdType::Const:
      append(Instruction::Literal, info.value);
      break;
    case IdType::Function:
      takeToken(TokenType::ParenL);
      while (cur_token.type != TokenType::ParenR) {
        params++;
        expression();
        if (cur_token.type == TokenType::Colon) {
          nextToken();
          // continue;
        } else {
          break;
        }
      }
      takeToken(TokenType::ParenR);
      if (params != info.param_size) {
        throw "params not same";
      }
      append(Instruction::Call, info.level, info.entry_point);
      break;
    case IdType::Var:
      append(Instruction::Load, info.level, info.addr);
      break;
    }
  } else if (cur_token.type == TokenType::Integer) {
    append(Instruction::Literal, cur_token.integer);
    nextToken();
  } else if (cur_token.type == TokenType::ParenL) {
    nextToken();
    expression();
    takeToken(TokenType::ParenR);
  } else {
    lexer.print_head();
    std::cout << cur_token << std::endl;
    throw "expect factr but";
  }
}

size_t Compiler::append(Instruction instruction) {
  return append(static_cast<long long>(instruction));
}

size_t Compiler::append(Instruction instruction, long long value) {
  assert(operand_size(instruction) == 1);
  append(instruction);
  return append(value);
}

size_t Compiler::append(Instruction instruction, long long first,
                        long long second) {
  assert(operand_size(instruction) == 2);
  append(instruction);
  append(first);
  return append(second);
}

size_t Compiler::append(long long value) {
  program.push_back(value);
  return program.size() - 1;
}

void Compiler::backpatch(size_t target) { program[target] = program.size(); }

void Compiler::nextToken() {
  cur_token = std::move(peek_token);
  peek_token = std::move(lexer.nextToken());
}

void Compiler::takeToken(TokenType type) {
  if (cur_token.type != type) {
    std::cerr << cur_token.type << std::endl;
    throw "unexpected token";
  }
  nextToken();
}

void pl0::print_program(const Program &program) {
  size_t i = 0;
  while (i < program.size()) {
    std::cout << i << ": ";
    Instruction inst = static_cast<Instruction>(program[i++]);
    std::cout << inst;

    size_t size = operand_size(inst);
    while (size--) {
      std::cout << ' ' << program[i++];
    }
    std::cout << std::endl;
  }
}
