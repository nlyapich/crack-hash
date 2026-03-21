#ifndef HASHER_HPP
#define HASHER_HPP

#include <string>

namespace CrackHash
{

namespace common
{

class Hasher
{
public:
    static std::string md5(const std::string& input);
};

} // namespace common

} // namespace CrackHash

#endif // HASHER_HPP
