#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>
#include  <queue>
#include <utility>
#include <httplib.h>

#include "models.hpp"
#include "thread_safe_map.hpp"
#include "xml_models.hpp"

namespace CrackHash
{

// Структура состояния запроса
struct RequestState
{
    models::RequestStatus status = models::RequestStatus::IN_PROGRESS;
    std::vector<std::string> results;
    int workersCompleted = 0;
    int totalWorkers = 0;
    std::chrono::steady_clock::time_point createdAt;
    std::mutex resultsMutex;
};
    
class Manager
{
public:
    Manager(const std::vector<std::string>& workerUrls,
            int managerPort,
            const std::string& alphabet,
            int timeoutSeconds);
    virtual ~Manager();

    void start();

private:
    // Генерация UUID (упрощенная)
    static std::string generateUUID();

    // Отправка задачи воркеру (XML вместо JSON)
    bool sendTaskToWorker(const std::string& workerUrl,
                          const CrackHash::xml_models::ManagerRequest& task);

    // Обработка запроса на взлом хэша
    void handleCrackRequest(const httplib::Request& req, httplib::Response& res);

    // Обработка запроса статуса
    void handleStatusRequest(const httplib::Request& req, httplib::Response& res);

    // Обработка ответа от воркера (XML)
    void handleWorkerResponse(const httplib::Request& req, httplib::Response& res);

    bool checkWorkerHealth(const std::string& workerUrl);
    
    void startHealthCheck();

    void startQueue();

    ThreadSafeMap<std::string, std::shared_ptr<RequestState>> requestStates;
    ThreadSafeMap<std::pair<std::string, int>, std::string> cacheRequests;
    ThreadSafeMap<std::string, bool> healthCheck;
    ThreadSafeMap<std::string, bool> busyWorkers;

    std::queue<CrackHash::xml_models::ManagerRequest> tasks;

    std::atomic<int> healthyWorkers;
    std::vector<std::string> workerUrls;
    const std::string kAlphabet;
    const int kTimeoutSeconds;
    const int kManagerPort;
};

}

#endif // MANAGER_HPP