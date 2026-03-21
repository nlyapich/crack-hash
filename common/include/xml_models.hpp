#ifndef XML_MODELS_HPP
#define XML_MODELS_HPP

#include <sstream>
#include <string>
#include <vector>

namespace CrackHash
{

namespace xml_models
{

// Пространства имён из XSD
constexpr const char* NS_REQUEST = "http://ccfit.nsu.ru/schema/crack-hash-request";
constexpr const char* NS_RESPONSE = "http://ccfit.nsu.ru/schema/crack-hash-response";

// Запрос менеджера к воркеру
struct ManagerRequest
{
    std::string requestId;
    int partNumber = 0;
    int partCount = 1;
    std::string hash;
    int maxLength = 0;
    std::vector<std::string> alphabetSymbols;
    
    // Сериализация в XML
    std::string toXml() const;
    
    // Десериализация из XML
    static ManagerRequest fromXml(const std::string& xml);
};

// Ответ воркера менеджеру
struct WorkerResponse
{
    std::string requestId;
    std::vector<std::string> results;
    
    // Сериализация в XML
    std::string toXml() const;
    
    // Десериализация из XML
    static WorkerResponse fromXml(const std::string& xml);
};

} // namespace xml_models

} // namespace CrackHash

#endif // XML_MODELS_HPP
