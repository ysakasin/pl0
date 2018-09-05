#include "llvm/IR/LegacyPassManager.h"
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

#include "./llvm_frontend.hpp"

using namespace pl0;

Frontend::Frontend(const std::string &path)
    : lexer(path), context(), module(new llvm::Module("top", context)),
      builder(context) {
  // int main()
  cur_token = std::move(lexer.nextToken());
  peek_token = std::move(lexer.nextToken());
}

void Frontend::compile() {
  auto *funcType = llvm::FunctionType::get(builder.getInt64Ty(), false);
  auto *mainFunc = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "main", module);
  auto *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
  block(mainFunc);
  builder.CreateRet(builder.getInt32(1));
}

void Frontend::block(llvm::Function *func) {
  ident_table.enterBlock();
  std::vector<std::string> vars;
  while (true) {
    if (cur_token.type == TokenType::Const) {
      constDecl();
    } else if (cur_token.type == TokenType::Var) {
      varDecl(&vars);
    } else if (cur_token.type == TokenType::Function) {
      functionDecl();
    } else {
      break;
    }
  }

  curFunc = func;
  builder.SetInsertPoint(&func->getEntryBlock());
  auto itr = func->arg_begin();
  for (size_t i = 0; i < func->arg_size(); i++) {
    auto *alloca =
        builder.CreateAlloca(builder.getInt64Ty(), 0, itr->getName());
    builder.CreateStore(itr, alloca);
    ident_table.appendVar(itr->getName(), alloca);
    itr++;
  }
  for (const auto &var : vars) {
    auto *alloca = builder.CreateAlloca(builder.getInt64Ty(), 0, var);
    ident_table.appendVar(var, alloca);
  }
  statement();
  ident_table.leaveBlock();
}

void Frontend::constDecl() {
  takeToken(TokenType::Const);
  while (true) {
    if (cur_token.type != TokenType::Ident) {
      parseError(TokenType::Ident, cur_token.type);
    }

    std::string const_name = cur_token.ident;
    nextToken();
    takeToken(TokenType::Equal);

    if (cur_token.type != TokenType::Integer) {
      parseError(TokenType::Integer, cur_token.type);
    }

    ident_table.appendConst(const_name, builder.getInt64(cur_token.integer));
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

void Frontend::varDecl(std::vector<std::string> *vars) {
  takeToken(TokenType::Var);
  while (true) {
    if (cur_token.type != TokenType::Ident) {
      parseError(TokenType::Ident, cur_token.type);
    }

    vars->push_back(cur_token.ident);
    nextToken();

    if (cur_token.type == TokenType::Colon) {
      nextToken();
      // continue;
    } else if (cur_token.type == TokenType::Semicolon) {
      nextToken();
      break;
    } else {
      error("unexpected at varDecl");
    }
  }
}
void Frontend::functionDecl() {
  takeToken(TokenType::Function);
  if (cur_token.type != TokenType::Ident) {
    parseError(TokenType::Ident, cur_token.type);
  }
  std::string func_name = std::move(cur_token.ident);
  nextToken();

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

  std::vector<llvm::Type *> param_types(params.size(), builder.getInt64Ty());
  auto *funcType =
      llvm::FunctionType::get(builder.getInt64Ty(), param_types, false);
  auto *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                      func_name, module);
  auto *bblock = llvm::BasicBlock::Create(context, "entry", func);
  ident_table.appendFunction(func_name, func);

  auto itr = func->arg_begin();
  for (size_t i = 0; i < params.size(); i++) {
    ident_table.appendParam(params[i]);
    itr->setName(params[i]);
    itr++;
  }

  block(func);
  takeToken(TokenType::Semicolon);
}

void Frontend::statement() {
  size_t backpatch_target;
  size_t start_at;
  const pl0llvm::IdInfo *info;

  llvm::Value *val;
  llvm::BasicBlock *stash;

  switch (cur_token.type) {
  case TokenType::Ident:
    statementAssign();
    return;
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
        parseError(TokenType::End, cur_token.type);
      }
    }
    return;
  case TokenType::If:
    statementIf();
    return;
  case TokenType::While:
    statementWhile();
    return;
  case TokenType::Return:
    nextToken();
    builder.CreateRet(expression());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "dummy"));
    return;
  case TokenType::Write:
    unsupportedError("write statement");
  case TokenType::Writeln:
    unsupportedError("writeln statement");
  default:;
  }
}

void Frontend::statementAssign() {
  const auto &info = ident_table.find(cur_token.ident);
  llvm::Value *assignee;
  if (info.type == pl0llvm::IdType::Var) {
    assignee = info.val;
  } else {
    error("variable is expected but it is not variable");
  }
  nextToken();
  takeToken(TokenType::Assign);
  builder.CreateStore(expression(), assignee);
  return;
}

void Frontend::statementIf() {
  takeToken(TokenType::If);

  auto *cond = condition();

  takeToken(TokenType::Then);

  auto *then_block = llvm::BasicBlock::Create(context, "if.then", curFunc);
  auto *merge_block = llvm::BasicBlock::Create(context, "if.merge");

  builder.CreateCondBr(cond, then_block, merge_block);

  builder.SetInsertPoint(then_block);
  statement();
  builder.CreateBr(merge_block);
  then_block = builder.GetInsertBlock();

  curFunc->getBasicBlockList().push_back(merge_block);
  builder.SetInsertPoint(merge_block);
}

void Frontend::statementWhile() {
  takeToken(TokenType::While);

  auto *cond_block = llvm::BasicBlock::Create(context, "while.cond", curFunc);
  auto *body_block = llvm::BasicBlock::Create(context, "while.body");
  auto *merge_block = llvm::BasicBlock::Create(context, "while.merge");

  builder.CreateBr(cond_block);

  {
    builder.SetInsertPoint(cond_block);
    auto *cond = condition();
    takeToken(TokenType::Do);
    builder.CreateCondBr(cond, body_block, merge_block);
  }

  {
    curFunc->getBasicBlockList().push_back(body_block);
    builder.SetInsertPoint(body_block);
    statement();
    builder.CreateBr(merge_block);
  }

  curFunc->getBasicBlockList().push_back(merge_block);
  builder.SetInsertPoint(merge_block);
}

llvm::CmpInst::Predicate token_to_inst(TokenType type) {
  switch (type) {
  case TokenType::Equal:
    return llvm::CmpInst::Predicate::ICMP_EQ;
  case TokenType::NotEqual:
    return llvm::CmpInst::Predicate::ICMP_NE;
  case TokenType::Less:
    return llvm::CmpInst::Predicate::ICMP_SLT;
  case TokenType::LessEqual:
    return llvm::CmpInst::Predicate::ICMP_SLE;
  case TokenType::Greater:
    return llvm::CmpInst::Predicate::ICMP_SGT;
  case TokenType::GreaterEqual:
    return llvm::CmpInst::Predicate::ICMP_SGE;
  default:
    throw "not support at token to inst";
  }
}

llvm::Value *Frontend::condition() {
  if (cur_token.type == TokenType::Odd) {
    nextToken();
    auto *lhs = builder.CreateSRem(expression(), builder.getInt64(2));
    return builder.CreateICmpEQ(lhs, builder.getInt64(0));
  } else {
    auto *lhs = expression();
    llvm::CmpInst::Predicate inst = token_to_inst(cur_token.type);
    nextToken();
    auto *rhs = expression();
    return builder.CreateICmp(inst, lhs, rhs);
  }
}

llvm::Value *Frontend::expression() {
  TokenType sign = cur_token.type;
  if (cur_token.type == TokenType::Plus || cur_token.type == TokenType::Minus) {
    nextToken();
  }

  llvm::Value *ret = term();
  if (sign == TokenType::Minus) {
    ret = builder.CreateNeg(ret);
  }

  while (true) {
    if (cur_token.type == TokenType::Plus) {
      nextToken();
      ret = builder.CreateAdd(ret, term());
    } else if (cur_token.type == TokenType::Minus) {
      nextToken();
      ret = builder.CreateSub(ret, term());
    } else {
      break;
    }
  }
  return ret;
}

llvm::Value *Frontend::term() {
  llvm::Value *ret = factor();
  while (true) {
    if (cur_token.type == TokenType::Mul) {
      nextToken();
      ret = builder.CreateMul(ret, factor());
    } else if (cur_token.type == TokenType::Div) {
      nextToken();
      ret = builder.CreateSDiv(ret, factor());
    } else {
      break;
    }
  }
  return ret;
}

llvm::Value *Frontend::factor() {
  llvm::Value *ret;
  if (cur_token.type == TokenType::Ident) {
    ret = factorIdent();
  } else if (cur_token.type == TokenType::Integer) {
    ret = builder.getInt64(cur_token.integer);
    nextToken();
  } else if (cur_token.type == TokenType::ParenL) {
    nextToken();
    ret = expression();
    takeToken(TokenType::ParenR);
  } else {
    lexer.print_head();
    error("expect factor but not");
  }
  return ret;
}

llvm::Value *Frontend::factorIdent() {
  auto &val = ident_table.find(cur_token.ident);
  takeToken(TokenType::Ident);

  switch (val.type) {
  case pl0llvm::IdType::Const:
    return val.val;
  case pl0llvm::IdType::Var:
    // TODO: ?
    return builder.CreateLoad(val.val);
  case pl0llvm::IdType::Param:
    error("Param ident can not be factor");
  case pl0llvm::IdType::Function:
    takeToken(TokenType::ParenL);
    std::vector<llvm::Value *> args;
    while (cur_token.type != TokenType::ParenR) {
      args.push_back(expression());
      if (cur_token.type == TokenType::Colon) {
        nextToken();
        // continue;
      } else {
        break;
      }
    }
    takeToken(TokenType::ParenR);
    if (args.size() != val.func->arg_size()) {
      error("argument number is wrong");
    }
    return builder.CreateCall(val.func, args);
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "usage " << argv[0] << " FILE" << std::endl;
    return 1;
  }

  // std::fstream ifs(argv[1]);
  // if (ifs.fail()) {
  //   std::cerr << "cannot read " << argv[1] << std::endl;
  //   return 1;
  // }

  // std::string code = std::string(std::istreambuf_iterator<char>(ifs),
  // std::istreambuf_iterator<char>());

  pl0::Frontend frontend(argv[1]);
  frontend.compile();

  llvm::legacy::PassManager pm;

  // generate bitcode
  std::error_code error_info;
  llvm::raw_fd_ostream raw_stream("out.ll", error_info,
                                  llvm::sys::fs::OpenFlags::F_None);
  pm.add(llvm::createPrintModulePass(raw_stream));
  pm.run(*frontend.getModule());
  raw_stream.close();

  return 0;
}
