#include "hash.hpp"

#include <openssl/evp.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <spdlog/spdlog.h>

namespace CrackHash
{

namespace Hash
{

std::string md5(const std::string& input)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        spdlog::error("Failed to create EVP_MD_CTX");
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }
    
    if (EVP_DigestInit_ex(ctx, EVP_md5(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, input.c_str(), input.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1)
    {
        EVP_MD_CTX_free(ctx);
        spdlog::error("MD5 computation failed");
        throw std::runtime_error("MD5 computation failed");
    }
    
    EVP_MD_CTX_free(ctx);
    
    std::stringstream ss;
    for (unsigned int i = 0; i < digest_len; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return ss.str();
}

} // namespace Hash

} // namespace CrackHash
