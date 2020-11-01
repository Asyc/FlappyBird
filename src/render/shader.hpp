#ifndef FLAPPYBIRD_SHADER_HPP
#define FLAPPYBIRD_SHADER_HPP

#include <array>
#include <string_view>
#include <vector>

#include <vulkan/vulkan.hpp>

class Shader {
public:
    explicit Shader(vk::Device device, const std::string_view& location);
private:
    vk::UniqueShaderModule m_Vertex, m_Fragment;

    std::vector<vk::VertexInputBindingDescription> m_VertexInputBindingDescriptors;
    std::vector<vk::VertexInputAttributeDescription> m_VertexInputAttributeDescriptors;
    vk::UniquePipelineLayout m_Layout;

    std::array<vk::PipelineShaderStageCreateInfo, 2> m_ShaderStageCreateInfo;
};


#endif //FLAPPYBIRD_SHADER_HPP
