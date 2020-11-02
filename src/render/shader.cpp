#include "shader.hpp"

#include <algorithm>

#include <rapidjson/document.h>

#include "io/file.hpp"

using namespace rapidjson;

#define TYPE_CHECK(type, prefix, vkType, bits)                                          \
do {                                                                                    \
    if ((type).rfind((#prefix), 0) == 0) {                                              \
        switch ((type).c_str()[type.size() - 1] - '0') {                                \
            case 1:                                                                     \
                return (vk::Format::eR##bits##S##vkType);                               \
            case 2:                                                                     \
                return (vk::Format::eR##bits##G##bits##S##vkType);                      \
            case 3:                                                                     \
                return (vk::Format::eR##bits##G##bits##B##bits##S##vkType);             \
            case 4:                                                                     \
                return (vk::Format::eR##bits##G##bits##B##bits##A##bits##S##vkType);    \
            default:                                                                    \
                throw std::runtime_error("unknown type in shader meta");                \
        }                                                                               \
    }                                                                                   \
} while (false)

inline vk::Format parseType(const std::string& type) {
    TYPE_CHECK(type, byte, int, 8);
    TYPE_CHECK(type, short, int, 16);
    TYPE_CHECK(type, int, int, 32);
    TYPE_CHECK(type, long, int, 64);

    TYPE_CHECK(type, float, float, 32);
    TYPE_CHECK(type, double, float, 64);

    return vk::Format::eUndefined;
}

inline vk::ShaderStageFlagBits parseStage(const std::string& stage) {
    if (stage == "vertex") {
        return vk::ShaderStageFlagBits::eVertex;
    }

    if (stage == "fragment") {
        return vk::ShaderStageFlagBits::eFragment;
    }

    throw std::runtime_error("unknown shader stage in shader meta");
}

Shader::Shader(vk::Device device, const std::string_view& location) {
    std::string shaderPath = std::string(location);
    File shaderMeta(shaderPath + ".json");
    auto data = shaderMeta.readFileToString();

    auto vertexCodeBuffer = File(shaderPath + ".vert.spv").readFileContent();
    vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo({}, vertexCodeBuffer.size(), reinterpret_cast<uint32_t*>(vertexCodeBuffer.data()));
    m_Vertex = device.createShaderModuleUnique(vertexShaderModuleCreateInfo);

    auto fragmentCodeBuffer = File(shaderPath + ".frag.spv").readFileContent();
    vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo({}, fragmentCodeBuffer.size(), reinterpret_cast<uint32_t*>(fragmentCodeBuffer.data()));
    m_Fragment = device.createShaderModuleUnique(fragmentShaderModuleCreateInfo);

    Document metadata;
    metadata.Parse(data.c_str(), data.size());

    m_EntryPoint = std::string(metadata["entryPoint"].GetString());

    m_ShaderStageCreateInfo = {
            vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, *m_Vertex, m_EntryPoint.c_str(), nullptr),
            vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, *m_Fragment, m_EntryPoint.c_str(), nullptr)
    };

    const auto& bindings = metadata["bindings"].GetArray();

    m_VertexInputBindingDescriptors.reserve(bindings.Size());
    std::for_each(bindings.begin(), bindings.end(), [this](const Value& binding) {
        m_VertexInputBindingDescriptors.emplace_back(binding["location"].GetUint(), binding["stride"].GetUint(), vk::VertexInputRate::eVertex);
    });

    const auto& attributes = metadata["attributes"].GetArray();
    m_VertexInputAttributeDescriptors.reserve(attributes.Size());
    std::for_each(attributes.begin(), attributes.end(), [this](const Value& attribute) {
        m_VertexInputAttributeDescriptors.emplace_back(attribute["location"].GetUint(), attribute["binding"].GetUint(), parseType(std::string(attribute["type"].GetString())),
                                                       attribute["offset"].GetUint());
    });

    std::vector<vk::PushConstantRange> pushConstantRangeDescriptors;
    const auto& pushConstantRanges = metadata["pushConstantRanges"].GetArray();
    pushConstantRangeDescriptors.reserve(pushConstantRanges.Size());
    std::for_each(pushConstantRanges.begin(), pushConstantRanges.end(), [&](const Value& range){
        pushConstantRangeDescriptors.emplace_back(parseStage(std::string(range["shader"].GetString())), range["offset"].GetUint(), range["size"].GetUint());
    });

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo({}, 0, nullptr, pushConstantRangeDescriptors.size(), pushConstantRangeDescriptors.data());
    m_Layout = device.createPipelineLayoutUnique(pipelineLayoutCreateInfo);
}
