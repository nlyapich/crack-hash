#include "combinatorics.hpp"

namespace CrackHash
{

namespace common
{

// Подсчет общего количества комбинаций для длины от 1 до maxLength
long long Combinatorics::getTotalCount(const std::string& alphabet, int maxLength)
{
    long long total = 0;
    long long pow = 1;
    for (int i = 1; i <= maxLength; ++i)
    {
        pow *= static_cast<long long>(alphabet.size());
        total += pow;
    }
    return total;
}

// Получить строку по глобальному индексу во всем пространстве перебора
std::string Combinatorics::getByIndex(const std::string& alphabet,
                                      int maxLength,
                                      long long globalIndex)
{
    long long offset = 0;
    long long pow = 1;
    
    // Определяем длину строки
    int length = 1;
    for (; length <= maxLength; ++length)
    {
        long long countForLength = pow * static_cast<long long>(alphabet.size());
        if (globalIndex < offset + countForLength)
        {
            break;
        }
        offset += countForLength;
        pow *= static_cast<long long>(alphabet.size());
    }
    
    // Локальный индекс для данной длины
    long long localIndex = globalIndex - offset;
    
    // Генерируем строку заданной длины
    std::string result(length, ' ');
    long long base = static_cast<long long>(alphabet.size());
    
    for (int i = length - 1; i >= 0; --i)
    {
        result[i] = alphabet[localIndex % base];
        localIndex /= base;
    }
    
    return result;
}

} // namespace common

} // namespace CrackHash
