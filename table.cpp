#include <algorithm>
#include <cassert>

#include "./table.hpp"

using namespace pl0;

void Table::enterBlock() {
  cur_level++;
  level_start_at.push_back(infos.size());
  prev_addr.push_back(cur_addr);
  cur_addr = 0;
}

void Table::leaveBlock() {
  assert(cur_level > 0);
  cur_level--;
  infos.resize(level_start_at.back());
  level_start_at.pop_back();

  cur_addr = prev_addr.back();
  prev_addr.pop_back();
}

const IdInfo &Table::find(const std::string &id) const {
  auto itr = std::find_if(infos.rbegin(), infos.rend(),
                          [&](const IdInfo &info) { return info.name == id; });
  if (itr == infos.rend()) {
    throw "not find ident";
  }

  return *itr;
}

void Table::appendVar(const std::string &id) {
  infos.emplace_back(id, cur_level, 2 + cur_addr++);
}

void Table::appendParam(const std::string &param, long long offset) {
  infos.emplace_back(param, cur_level, offset);
}

void Table::appendConst(const std::string &id, long long value) {
  infos.emplace_back(id, value);
}

size_t Table::appendFunc(const std::string &id, long long entry_point,
                         long long param_size) {
  infos.emplace_back(id, cur_level + 1, entry_point, param_size);
  return infos.size() - 1;
}
