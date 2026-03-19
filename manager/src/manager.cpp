#include "manager.hpp"

#include <thread>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <uuid/uuid.h>

#include "models.hpp"
#include "hasher.hpp"
#include "combinatorics.hpp"
#include "thread_safe_map.hpp"
#include "xml_models.hpp"

using json = nlohmann::json;

namespace CrackHash
{

Manager::Manager(const std::vector<std::string>& workerUrls,
                 int managerPort,
                 const std::string& alphabet,
                 int timeoutSeconds)
: workerUrls{workerUrls},
  kManagerPort{managerPort},
  kAlphabet{alphabet},
  kTimeoutSeconds{timeoutSeconds} {}

Manager::~Manager() {}

void Manager::start()
{
    spdlog::info("Manager found {} worker(s)\n", workerUrls.size());
    for (const auto& url : workerUrls)
    {
        spdlog::info("   - {}\n", url);
    }
    
    spdlog::info("Manager starting on port {}", kManagerPort);

    spdlog::info("Health-check starting");
    startHealthCheck();
    startQueue();
    httplib::Server server;
    
    // API для клиентов
    server.Post("/api/hash/crack",
                [this](const httplib::Request& req, httplib::Response& res)
                {
                    handleCrackRequest(req, res);
                }
    );

    server.Get("/api/hash/status",
               [this](const httplib::Request& req, httplib::Response& res)
               {
                   handleStatusRequest(req, res);
               }
    );

    // API для воркеров
    server.Patch("/internal/api/manager/hash/crack/request",
                 [this](const httplib::Request& req, httplib::Response& res)
                 {
                     handleWorkerResponse(req, res);
                 }
    );
    
    // Запуск сервера
    if (server.listen("0.0.0.0", kManagerPort))
    {
        spdlog::info("Manager started successfully");
    }
    else
    {
        spdlog::error("Manager failed to start");
        throw std::runtime_error("Manager failed to start");
    }
}

bool Manager::checkWorkerHealth(const std::string& workerUrl)
{
    try
    {
        httplib::Client cli(workerUrl);
        cli.set_read_timeout(5, 0);  // 5 секунд таймаут
        
        auto res = cli.Get("/internal/api/worker/health");
        return res && res->status == 200;
    }
    catch (...)
    {
        return false;
    }
}

void Manager::startHealthCheck()
{
    std::thread([this]()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            for (const auto& url : workerUrls)
            {
                bool healthy = checkWorkerHealth(url);
                healthCheck.insert(url, healthy);
                spdlog::info("Worker {} health: {}", url, healthy ? "OK" : "FAILED");
            }
        }
    }).detach();
}

void Manager::startQueue()
{
    std::thread([this]()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            if (!tasks.empty())
            {
                for (const auto& url : workerUrls)
                {
                    bool healthy = *healthCheck.get(url);
                    if (!healthy)
                    {
                        continue;
                    }

                    if (*busyWorkers.get(url))
                    {
                        continue;
                    }

                    healthCheck.insert(url, healthy);
                    sendTaskToWorker(url, tasks.front());
                    tasks.pop();
                }
            }
        }
    }).detach();
}

// Генерация UUID
std::string Manager::generateUUID()
{
    uuid_t uuid;
    uuid_generate_random(uuid);  // Генерация случайного UUID
    
    char uuid_str[37];  // 36 символов + null-терминатор
    uuid_unparse_lower(uuid, uuid_str);  // Преобразование в строку
    
    return std::string(uuid_str);
}

// Отправка задачи воркеру (XML)
bool Manager::sendTaskToWorker(const std::string& workerUrl,
                               const CrackHash::xml_models::ManagerRequest& task)
{
    try
    {
        httplib::Client cli(workerUrl);
        cli.set_read_timeout(30, 0);
        
        // Сериализация в XML
        std::string xmlBody = task.toXml();
        
        spdlog::info("Sending task to worker: {}\nTask:\n{}\n", workerUrl, xmlBody);

        auto res = cli.Post("/internal/api/worker/hash/crack/task", 
                           xmlBody, 
                           "application/xml");
        
        if (res && res->status == 200)
        {
            spdlog::info("Task sent to worker: {}\n", workerUrl);
            return true;
        }
        spdlog::error("Failed to send task to worker: {}\n", workerUrl);
        return false;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Exception sending task: {}\n", e.what());
        return false;
    }
}

// Обработка запроса на взлом хэша
void Manager::handleCrackRequest(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        models::CrackRequest crackReq;
        if (!req.body.empty())
        {
            crackReq = nlohmann::json::parse(req.body);
        }
        
        if (crackReq.hash.empty() || crackReq.maxLength <= 0)
        {
            res.status = 400;
            res.set_content(json{{"error", "Invalid request"}}.dump(), "application/json");
            return;
        }
        
        auto cacheRequestId = cacheRequests.get({crackReq.hash, crackReq.maxLength});
        if (cacheRequestId)
        {
            models::CrackResponse crackResp;
            crackResp.requestId = *cacheRequestId;
            res.set_content(json(crackResp).dump(), "application/json");
            spdlog::info("Cached crack request: {}", *cacheRequestId);
            return;
        }

        std::string requestId = generateUUID();
        auto state = std::make_shared<RequestState>();
        state->createdAt = std::chrono::steady_clock::now();
        state->totalWorkers = static_cast<int>(workerUrls.size());
        
        cacheRequests.insert({crackReq.hash, crackReq.maxLength}, requestId);
        requestStates.insert(requestId, state);
        
        // Распределение задач между воркерами
        long long totalCount = common::Combinatorics::getTotalCount(kAlphabet, crackReq.maxLength);
        long long chunkSize = totalCount / state->totalWorkers;
        
        for (size_t i = 0; i < workerUrls.size(); ++i)
        {
            CrackHash::xml_models::ManagerRequest task;
            task.requestId = requestId;
            task.hash = crackReq.hash;
            task.partNumber = static_cast<int>(i);
            task.partCount = static_cast<int>(workerUrls.size());
            task.maxLength = crackReq.maxLength;
    
            // Генерация алфавита из строки
            for (char c : kAlphabet)
            {
                task.alphabetSymbols.push_back(std::string(1, c));
            }
            
            tasks.push(task);
            // sendTaskToWorker(workerUrls[i], task);
        }
        
        // Ответ клиенту
        models::CrackResponse crackResp;
        crackResp.requestId = requestId;
        
        res.set_content(json(crackResp).dump(), "application/json");
        spdlog::info("New crack request: {}", requestId);
        
    }
    catch (const std::exception& e)
    {
        spdlog::error("Error handling crack request: {}", e.what());
        res.status = 500;
        res.set_content(json{{"error", e.what()}}.dump(), "application/json");
    }
}

// Обработка запроса статуса
void Manager::handleStatusRequest(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        auto it = req.params.find("requestId");
        
        if (it == req.params.end())
        {
            res.status = 400;
            res.set_content(nlohmann::json{{"error", "Missing requestId"}}.dump(), "application/json");
            return;
        }
        
        std::string requestId = it->second;
        auto stateOpt = requestStates.get(requestId);
        
        if (!stateOpt)
        {
            res.status = 404;
            res.set_content(json{{"error", "Request not found"}}.dump(), "application/json");
            return;
        }
        
        auto state = *stateOpt;
        
        // Проверка таймаута
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - state->createdAt).count();
        
        if (state->status == models::RequestStatus::IN_PROGRESS && elapsed > kTimeoutSeconds)
        {
            state->status = models::RequestStatus::ERROR;
        }
        
        models::StatusResponse statusResp;
        statusResp.status = state->status;
        statusResp.data = state->results;
        
        res.set_content(json(statusResp).dump(), "application/json");        
    }
    catch (const std::exception& e)
    {
        spdlog::error("Error handling status request: {}", e.what());
        res.status = 500;
        res.set_content(json{{"error", e.what()}}.dump(), "application/json");
    }
}

// Обработка ответа от воркера (XML)
void Manager::handleWorkerResponse(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // Десериализация XML
        CrackHash::xml_models::WorkerResponse workerResp = CrackHash::xml_models::WorkerResponse::fromXml(req.body);
        spdlog::info(req.body + "\n");
        
        auto stateOpt = requestStates.get(workerResp.requestId);
        if (!stateOpt)
        {
            res.status = 404;
            res.set_content("<?xml version=\"1.0\"?><error>Request not found</error>", "application/xml");
            return;
        }
        
        auto state = *stateOpt;
        
        // Добавление результатов
        {
            std::lock_guard<std::mutex> lock(state->resultsMutex);
            state->results.insert(state->results.end(), 
                                 workerResp.results.begin(), 
                                 workerResp.results.end());
            state->workersCompleted++;
            if (state->status == models::RequestStatus::IN_PROGRESS && !workerResp.results.empty())
            {
                state->status = models::RequestStatus::PARTIALLY_READY;
            }
            
            if (state->workersCompleted >= state->totalWorkers) {
                state->status = CrackHash::models::RequestStatus::READY;
            }
        }
        
        res.status = 200;
        res.set_content("<?xml version=\"1.0\"?><status>ok</status>", "application/xml");
        spdlog::info("Received results from worker for request: {}\n", workerResp.requestId);
    }
    catch (const std::exception& e)
    {
        spdlog::error("Error handling worker response: {}\n", e.what());
        res.status = 500;
        res.set_content("<?xml version=\"1.0\"?><error>" + std::string(e.what()) + "</error>", "application/xml");
    }
}

} // namespace CrackHash
