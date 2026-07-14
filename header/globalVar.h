#ifndef _GLOBALVAR_H
#define _GLOBALVAR_H

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>
#include <set>
#include <string>
#include "type.h"
#include <array>
#include <cstddef>
#include <iomanip>
using namespace std;



namespace Config{
    inline constexpr int MAX_QUERY_GRAPH_NUMBER = 200;
    inline constexpr int MAX_QUERY_GRAPH_SIZE = 20;//Only for memory reservation, safe to change it
    inline constexpr int UPDATE_BLOCK_SIZE = 4*1024*3;
    enum class OutputMode {
        NONE,   // 不输出
        SIMPLE, // 简要输出
        ALL     // 全部输出
    };
    inline constexpr OutputMode OUTPUT_MODE = OutputMode::NONE;
    constexpr std::array<int, 13> factTable = {
        1,
        1,
        2,
        6,
        24,
        120,
        720,
        5040,
        40320,
        362880,
        3628800,
        39916800,
        479001600
    };

    // 前13个阶乘：0! ~ 12!


    // constexpr inline 函数（C++17 起支持 inline variables/functions）
    constexpr int factorial(int n) {
        return factTable[n];
    }
}
namespace BasicComputation{
    constexpr int MAX_COMBINATION = 30;

    // 编译期计算组合数表
    constexpr auto precomputeCombinations() {
        std::array<std::array<int, MAX_COMBINATION + 1>, MAX_COMBINATION + 1> comb{};
        for (int n = 0; n <= MAX_COMBINATION; ++n) {
            comb[n][0] = 1;
            comb[n][n] = 1;
            for (int k = 1; k < n; ++k) {
                comb[n][k] = comb[n - 1][k - 1] + comb[n - 1][k];
            }
        }

        return comb;
    }
    constexpr auto COMB_TABLE = precomputeCombinations();
    constexpr int combination(int n, int k) {
        return (k > n || k < 0) ? 0 : COMB_TABLE[n][k];
    }
    inline void printCombinationTable() {
        const int width = 12; // 每个数字占的宽度，用于对齐显示

        for (int n = 0; n <= MAX_COMBINATION; ++n) {
            std::cout << "n = " << std::setw(2) << n << ": ";
            for (int k = 0; k <= n; ++k) {
                std::cout << std::setw(width) << combination(n, k);
            }
            std::cout << '\n';
        }
    }
}
// extern int updateSize;
// extern int QGNum;

#endif