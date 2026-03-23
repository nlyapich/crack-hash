#ifndef WORKER_HPP
#define WORKER_HPP

#include <thread>
#include <httplib.h>

#include "models.hpp" 

namespace CrackHash
{

class Worker
{
public:
    Worker(int workerPort,
           const std::string& managerUrl);
    virtual ~Worker();

    void start();

private:
    // Отправка результатов менеджеру (XML)
    bool sendResultsToManager(const Models::WorkerResponse& response);

    // Перебор хэшей в заданном диапазоне (адаптировано под Models::ManagerRequest)
    void bruteForce(const Models::ManagerRequest& task);

    // Обработка задачи от менеджера (XML)
    void handleTaskRequest(const httplib::Request& req,
                           httplib::Response& res);

    const int kWorkerPort;
    const std::string kManagerUrl;
};

}

#endif // WORKER_HPP
