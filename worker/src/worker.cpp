#include "worker.hpp"

#include <thread>
#include <spdlog/spdlog.h>
#include <httplib.h>

#include "models.hpp"
#include "hash.hpp"
#include "combinatorics.hpp"
#include "xml_models.hpp" 

namespace CrackHash
{

Worker::Worker(int workerPort,
               const std::string& managerUrl)
: kWorkerPort{workerPort},
  kManagerUrl{managerUrl} {}

Worker::~Worker() {}

void Worker::start()
{
    spdlog::info("Worker starting on port {}", kWorkerPort);
    spdlog::info("Manager URL: {}", kManagerUrl);

    httplib::Server server;
    
    // API для менеджера
    server.Post("/internal/api/worker/hash/crack/task",
                [this](const httplib::Request& req, httplib::Response& res)
                {
                    handleTaskRequest(req, res);
                }
    );
    
    server.Get("/internal/api/worker/health",
               [](const httplib::Request& req, httplib::Response& res)
               {
                   res.set_content(R"({"status":"ok"})", "application/xml");
               }
    );

    // Запуск сервера
    if (server.listen("0.0.0.0", kWorkerPort))
    {
        spdlog::info("Worker started successfully");
    }
    else
    {
        spdlog::error("Worker failed to start");
        throw std::runtime_error("Worker failed to start");
    }
}

// Отправка результатов менеджеру (XML)
bool Worker::sendResultsToManager(const CrackHash::xml_models::WorkerResponse& response)
{
    try
    {
        httplib::Client cli(kManagerUrl);
        cli.set_read_timeout(30, 0);
        
        // Сериализация в XML
        std::string xmlBody = response.toXml();

        spdlog::info("Sending results to manager\nResults:\n{}\n", xmlBody);
        
        auto res = cli.Patch("/internal/api/manager/hash/crack/request", 
                            xmlBody, 
                            "application/xml");  // <-- Content-Type: application/xml
        
        if (res && res->status == 200)
        {
            spdlog::info("Results sent to manager for request: {}\n", response.requestId);
            return true;
        }
        spdlog::error("Failed to send results to manager\n");
        return false;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Exception sending results: {}\n", e.what());
        return false;
    }
}

// Перебор хэшей в заданном диапазоне (адаптировано под xml_models::ManagerRequest)
void Worker::bruteForce(const CrackHash::xml_models::ManagerRequest& task)
{
    std::vector<std::string> results;
    
    // Восстановление алфавита из символов
    std::string alphabet;
    for (const auto& sym : task.alphabetSymbols)
    {
        if (!sym.empty())
        {
            alphabet += sym[0];
        }
    }
    
    long long totalCount = CrackHash::Combinatorics::getTotalCount(alphabet, task.maxLength);
    long long chunkSize = totalCount / task.partCount;
    
    long long startIndex = task.partNumber * chunkSize;
    long long endIndex = (task.partNumber == task.partCount - 1) 
                        ? totalCount 
                        : (task.partNumber + 1) * chunkSize;
    
    spdlog::info("Worker starting brute force: part {}/{} [{} - {}]\n", 
            task.partNumber + 1, task.partCount, startIndex, endIndex);
    
    for (long long i = startIndex; i < endIndex; ++i)
    {
        std::string word = CrackHash::Combinatorics::getByIndex(alphabet, task.maxLength, i);
        std::string hash = CrackHash::Hash::md5(word);
        
        if (hash == task.hash)
        {
            results.push_back(word);
            spdlog::info("Found match: {} -> {}\n", word, hash);
        }
        
        if (i % 100000 == 0)
        {
            std::this_thread::yield();
        }
    }
    
    // Отправка результатов менеджеру (XML)
    CrackHash::xml_models::WorkerResponse response;
    response.requestId = task.requestId;
    response.results = results;
    
    sendResultsToManager(response);
}

// Обработка задачи от менеджера (XML)
void Worker::handleTaskRequest(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // Десериализация XML в модель
        CrackHash::xml_models::ManagerRequest task = CrackHash::xml_models::ManagerRequest::fromXml(req.body);
        
        spdlog::info("Received task for request: {}\n", task.requestId);
        
        // Запуск перебора в отдельном потоке
        std::thread bruteForceThread(
            [this, task = std::move(task)]()
            {
                this->bruteForce(task);
            }
        );
        bruteForceThread.detach();
        
        res.status = 200;
        res.set_content("<?xml version=\"1.0\"?><status>accepted</status>", "application/xml");
        
    }
    catch (const std::exception& e)
    {
        spdlog::error("Error handling task: {}\n", e.what());
        res.status = 500;
        res.set_content("<?xml version=\"1.0\"?><error>" + std::string(e.what()) + "</error>", "application/xml");
    }
}

}
