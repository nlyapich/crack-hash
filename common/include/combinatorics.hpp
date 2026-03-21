#ifndef COMINATORICS_HPP
#define COMINATORICS_HPP

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace CrackHash
{

class Combinatorics
{
public:
    // Подсчет общего количества комбинаций для длины от 1 до maxLength
    static long long getTotalCount(const std::string& alphabet, int maxLength);

    // Получить строку по глобальному индексу во всем пространстве перебора
    static std::string getByIndex(const std::string& alphabet,
                                  int maxLength,
                                  long long globalIndex);
};

} // namespace CrackHash

#endif // COMINATORICS_HPP
