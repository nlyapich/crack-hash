#ifndef MODELS_HPP
#define MODELS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <uuid.h> // или используем простую генерацию

namespace CrackHash
{

namespace models
{

// Запрос от клиента к менеджеру
struct CrackRequest
{
    std::string hash;
    int maxLength;
};

// Ответ менеджера клиенту с requestId
struct CrackResponse
{
    std::string requestId;
};

// Статус запроса
enum class RequestStatus
{
    IN_PROGRESS,
    PARTIALLY_READY,
    READY,
    ERROR
};

// Ответ по статусу запроса
struct StatusResponse
{
    RequestStatus status;
    std::vector<std::string> data;
};

// Задача от менеджера воркеру
struct WorkerTask
{
    std::string requestId;
    std::string hash;
    int partNumber;
    int partCount;
    int maxLength;
    std::string alphabet;
};

// Ответ воркера менеджеру
struct WorkerResponse
{
    std::string requestId;
    std::vector<std::string> results;
};

// Сериализация JSON
inline void to_json(nlohmann::json& j, const CrackRequest& req)
{
    j = nlohmann::json{{"hash", req.hash}, {"maxLength", req.maxLength}};
}

inline void from_json(const nlohmann::json& j, CrackRequest& req)
{
    j.at("hash").get_to(req.hash);
    j.at("maxLength").get_to(req.maxLength);
}

inline void to_json(nlohmann::json& j, const CrackResponse& resp)
{
    j = nlohmann::json{{"requestId", resp.requestId}};
}

inline void from_json(const nlohmann::json& j, CrackResponse& resp)
{
    j.at("requestId").get_to(resp.requestId);
}

inline void to_json(nlohmann::json& j, const WorkerTask& task)
{
    j = nlohmann::json{
        {"requestId", task.requestId},
        {"hash", task.hash},
        {"partNumber", task.partNumber},
        {"partCount", task.partCount},
        {"maxLength", task.maxLength},
        {"alphabet", task.alphabet}
    };
}

inline void from_json(const nlohmann::json& j, WorkerTask& task)
{
    j.at("requestId").get_to(task.requestId);
    j.at("hash").get_to(task.hash);
    j.at("partNumber").get_to(task.partNumber);
    j.at("partCount").get_to(task.partCount);
    j.at("maxLength").get_to(task.maxLength);
    j.at("alphabet").get_to(task.alphabet);
}

inline void to_json(nlohmann::json& j, const WorkerResponse& resp)
{
    j = nlohmann::json{{"requestId", resp.requestId}, {"results", resp.results}};
}

inline void from_json(const nlohmann::json& j, WorkerResponse& resp)
{
    j.at("requestId").get_to(resp.requestId);
    j.at("results").get_to(resp.results);
}

inline void to_json(nlohmann::json& j, const StatusResponse& resp)
{
    std::string statusStr;
    switch (resp.status)
    {
        case RequestStatus::IN_PROGRESS: statusStr = "IN_PROGRESS"; break;
        case RequestStatus::READY: statusStr = "READY"; break;
        case RequestStatus::ERROR: statusStr = "ERROR"; break;
    }
    j = nlohmann::json{{"status", statusStr}, {"data", resp.data}};
}

} // namespace models

} // namespace CrackHash

#endif // MODELS_HPP
