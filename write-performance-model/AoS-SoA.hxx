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

#include <type_traits>

static_assert(std::is_same_v<decltype(S::f1), decltype(SoA::f1)::value_type>);
static_assert(std::is_same_v<decltype(S::f2), decltype(SoA::f2)::value_type>);
static_assert(std::is_same_v<decltype(S::f3), decltype(SoA::f3)::value_type>);
static_assert(std::is_same_v<decltype(S::f4), decltype(SoA::f4)::value_type>);
static_assert(std::is_same_v<decltype(S::f5), decltype(SoA::f5)::value_type>);

#endif
