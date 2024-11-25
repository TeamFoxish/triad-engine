#pragma once

#include <functional>

template <typename T> using Consumer = std::function<void(T)>;

template <typename T> using Supplier = std::function<T()>;

