#include "hash.hpp"

#include <openssl/md5.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace CrackHash
{

namespace common
{

namespace Hash
{

std::string md5(const std::string& input)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), digest);
    
    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return ss.str();
}

} // namespace Hash

} // namespace common

} // namespace CrackHash
