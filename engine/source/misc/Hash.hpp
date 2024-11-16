#pragma once

#include <functional>

// orig: https://github.com/skaarj1989/FrameGraph-Example/blob/e75800f35eabcc0e2c9adc25543229e125c09d44/src/Hash.hpp

template <typename T, typename... Rest>
static inline void hashCombine(std::size_t &seed, const T &v,
                               const Rest &...rest) {
  // https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hashCombine(seed, rest), ...);
}
