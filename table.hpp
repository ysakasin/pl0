#pragma once

#include <string>
#include <vector>

namespace pl0 {
enum class IdType {
  Const,
  Var,
  Function,
};

class IdInfo {
public:
  IdInfo() {}
  IdInfo(const std::string &name, long long value)
      : name(name), type(IdType::Const), value(value) {}
  IdInfo(const std::string &name, size_t level, size_t addr)
      : name(name), type(IdType::Var), level(level), addr(addr) {}
  IdInfo(const std::string &name, long long level, long long entry_point,
         long long param_size)
      : name(name), type(IdType::Function), level(level),
        entry_point(entry_point), param_size(param_size) {}

public:
  std::string name;
  IdType type;
  size_t level;
  size_t addr;
  long long value;
  long long entry_point;
  long long param_size;
};

class Table {
public:
  void enterBlock();
  void leaveBlock();

  const IdInfo &find(const std::string &id) const;
  const IdInfo &get(size_t id) const { return infos[id]; }
  void appendVar(const std::string &id);
  void appendParam(const std::string &param, long long offset);
  void appendConst(const std::string &id, long long value);
  size_t appendFunc(const std::string &id, long long entry_point,
                    long long param_size);

  size_t getLevel() const { return cur_level; }

private:
  std::vector<IdInfo> infos;
  std::vector<size_t> level_start_at;
  std::vector<size_t> prev_addr;
  size_t cur_level;
  size_t cur_addr;
  size_t index[100];
};
} // namespace pl0
