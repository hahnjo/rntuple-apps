#ifndef AOS_SOA_H
#define AOS_SOA_H

#include <cstdint>
#include <vector>

struct S {
  std::int32_t f1;
  std::int32_t f2;
  std::int32_t f3;
  std::int32_t f4;
  std::int32_t f5;
};

using AoS = std::vector<S>;

struct SoA {
  std::vector<std::int32_t> f1;
  std::vector<std::int32_t> f2;
  std::vector<std::int32_t> f3;
  std::vector<std::int32_t> f4;
  std::vector<std::int32_t> f5;
};

#endif
