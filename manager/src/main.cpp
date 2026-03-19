#include <spdlog/spdlog.h>

#include "manager.hpp"
#include "env_utils.hpp"

int main()
{
    // const std::vector<std::string> kWorkerUrls    = Env::get(
    //     "WORKER_URLS", 
    //     std::vector<std::string>{"http://worker1:8081"}
    // );
    
    // const int                      kManagerPort   = Env::get("MANAGER_PORT", 8080);
    // const int                      kTimeoutSecond = Env::get("MANAGER_TIMEOUT_SECONDS", 300);
    // const std::string              kAlphabet      = Env::get(
    //     "ALPHABET", 
    //     std::string("abcdefghijklmnopqrstuvwxyz0123456789")
    // );
    
    const std::vector<std::string> kWorkerUrls    = Env::getRequired<std::vector<std::string>>("WORKER_URLS"); 
    const int                      kManagerPort   = Env::getRequired<int>("MANAGER_PORT");
    const int                      kTimeoutSecond = Env::getRequired<int>("MANAGER_TIMEOUT_SECONDS");
    const std::string              kAlphabet      = Env::getRequired<std::string>("ALPHABET");

    CrackHash::Manager manager(kWorkerUrls,
                               kManagerPort,
                               kAlphabet,
                               kTimeoutSecond);
    try
    {
        manager.start();
    }
    catch(const std::exception& e)
    {
        spdlog::critical(e.what());
        return -1;
    }
    
    return 0;
}
