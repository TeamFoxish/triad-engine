#pragma once

#include <functional>

template <typename T> using Consumer = std::function<void(T)>;
template <typename T, typename E> using BiConsumer = std::function<void(T, E)>;

template <typename T> using Supplier = std::function<T()>;

