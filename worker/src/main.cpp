#include "worker.hpp"
#include "env_utils.hpp"

int main()
{
    const int         kWorkerPort = Env::get("WORKER_PORT", 8081);
    const std::string kManagerUrl = Env::get(
        "MANAGER_URL", 
        std::string("http://manager:8080")
    );

    CrackHash::Worker worker(kWorkerPort, kManagerUrl);
    
    try
    {
        worker.start();
    }
    catch(const std::exception& e)
    {
        spdlog::critical(e.what());
        return -1;
    }
    
    return 0;
}
