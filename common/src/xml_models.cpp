#include "xml_models.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <pugixml.hpp>

namespace CrackHash
{

namespace xml_models
{

std::string ManagerRequest::toXml() const
{
    pugi::xml_document doc;
    
    // Корневой элемент с пространством имён
    auto root = doc.append_child("CrackHashManagerRequest");
    root.append_attribute("xmlns") = NS_REQUEST;
    
    root.append_child("RequestId").text() = requestId.c_str();
    root.append_child("PartNumber").text() = partNumber;
    root.append_child("PartCount").text() = partCount;
    root.append_child("Hash").text() = hash.c_str();
    root.append_child("MaxLength").text() = maxLength;
    
    auto alphabet = root.append_child("Alphabet");
    for (const auto& symbol : alphabetSymbols) {
        alphabet.append_child("symbols").text() = symbol.c_str();
    }

    // Сериализация в строку
    std::ostringstream oss;
    pugi::xml_writer_stream writer(oss);
    doc.print(writer, "", pugi::format_indent, pugi::encoding_utf8);
    return oss.str();
}

ManagerRequest ManagerRequest::fromXml(const std::string& xml)
{
    ManagerRequest req;
    pugi::xml_document doc;
    auto result = doc.load_string(xml.c_str());
    
    if (!result)
    {
        throw std::runtime_error("Failed to parse XML: " + std::string(result.description()));
    }
    
    auto root = doc.child("CrackHashManagerRequest");
    if (!root)
    {            
        // Пробуем найти с пространством имён
        for (auto node : doc.children())
        {
            if (std::string(node.name()) == "CrackHashManagerRequest")
            {
                root = node;
                break;
            }
        }
    }

    if (!root)
    {
        throw std::runtime_error("Root element not found in XML");
    }
    
    req.requestId = root.child_value("RequestId");
    
    auto partNumberNode = root.child("PartNumber");
    req.partNumber = partNumberNode ? partNumberNode.text().as_int() : 0;
    
    auto partCountNode = root.child("PartCount");
    req.partCount = partCountNode ? partCountNode.text().as_int() : 1;
    
    req.hash = root.child_value("Hash");
    
    auto maxLengthNode = root.child("MaxLength");
    req.maxLength = maxLengthNode ? maxLengthNode.text().as_int() : 0;
    
    auto alphabet = root.child("Alphabet");
    for (auto symbol : alphabet.children("symbols"))
    {
        req.alphabetSymbols.push_back(symbol.text().as_string());
    }
    
    return req;
}

std::string WorkerResponse::toXml() const
{
    pugi::xml_document doc;
    
    auto root = doc.append_child("CrackHashWorkerResponse");
    root.append_attribute("xmlns") = NS_RESPONSE;
    
    root.append_child("RequestId").text() = requestId.c_str();
    
    auto resultsNode = root.append_child("Results");
    for (const auto& word : results)
    {
        resultsNode.append_child("word").text() = word.c_str();
    }

    // Сериализация в строку
    std::ostringstream oss;
    pugi::xml_writer_stream writer(oss);
    doc.print(writer, "", pugi::format_indent, pugi::encoding_utf8);
    return oss.str();
}

WorkerResponse WorkerResponse::fromXml(const std::string& xml)
{
    WorkerResponse resp;
    pugi::xml_document doc;
    auto result = doc.load_string(xml.c_str());
    
    if (!result)
    {
        throw std::runtime_error("Failed to parse XML: " + std::string(result.description()));
    }
    
    auto root = doc.child("CrackHashWorkerResponse");
    if (!root)
    {
        for (auto node : doc.children())
        {
            if (std::string(node.name()) == "CrackHashWorkerResponse")
            {
                root = node;
                break;
            }
        }
    }

    if (!root)
    {
        throw std::runtime_error("Root element not found in XML");
    }
    
    resp.requestId = root.child_value("RequestId");
    
    auto resultsNode = root.child("Results");
    for (auto word : resultsNode.children("word"))
    {
        resp.results.push_back(word.text().as_string());
    }
    
    return resp;
}

} // namespace xml_models

} // namespace CrackHash
