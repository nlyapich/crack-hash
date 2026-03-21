#ifndef THREAD_SAFE_MAP_HPP
#define THREAD_SAFE_MAP_HPP

#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <memory>
#include <functional>

namespace CrackHash
{

template<typename K, typename V>
class ThreadSafeMap
{
private:
    std::unordered_map<K, V> map_;
    mutable std::shared_mutex mutex_;

public:
    void insert(const K& key, const V& value)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        map_[key] = value;
    }

    std::optional<V> get(const K& key) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void update(const K& key, const V& value)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second = value;
        }
    }

    bool exists(const K& key) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.find(key) != map_.end();
    }
};

} // namespace CrackHash

// Специализация hash для std::pair<std::string, int>
namespace std
{

template<>
struct hash<std::pair<std::string, int>>
{
    std::size_t operator()(const std::pair<std::string, int>& p) const noexcept
    {
        auto h1 = std::hash<std::string>{}(p.first);
        auto h2 = std::hash<int>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

} // namespace std

#endif // THREAD_SAFE_MAP_HPP
