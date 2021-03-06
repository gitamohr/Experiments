#pragma once

#include <iostream>

inline auto getLastIdx() noexcept
{
    static auto lastIdx(0u);
    return lastIdx++;
}

template <typename T>
struct TypeId
{
    static std::size_t id;
};
template <typename T>
std::size_t TypeId<T>::id{getLastIdx()};

inline void check(std::size_t expected, std::size_t x)
{
    std::cout << expected << " == " << x << "\n";

    if(expected != x) std::terminate();
}

void print1();
void print2();