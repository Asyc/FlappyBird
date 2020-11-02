#include "pipeline.hpp"

Pipeline::Pipeline(vk::Device device, vk::RenderPass renderPass, Shader&& shader) : m_Layout(std::move(shader.m_Layout)) {

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo(
            {},
            shader.m_VertexInputBindingDescriptors.size(),
            shader.m_VertexInputBindingDescriptors.data(),
            shader.m_VertexInputAttributeDescriptors.size(),
            shader.m_VertexInputAttributeDescriptors.data()
    );

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo({}, 1, nullptr, 1, nullptr);

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(
            {},
            VK_FALSE,
            VK_FALSE,
            vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eNone,
            vk::FrontFace::eCounterClockwise,
            VK_FALSE,
            {},
            {},
            {},
            1.0f
    );

    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE);

    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(
            VK_TRUE,
            vk::BlendFactor::eSrcAlpha,
            vk::BlendFactor::eOneMinusSrcAlpha,
            vk::BlendOp::eAdd,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            {vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA}
    );

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eClear, 1, &colorBlendAttachmentState);

    std::array<vk::DynamicState, 2> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates.size(), dynamicStates.data());

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
            {},
            shader.m_ShaderStageCreateInfo.size(),
            shader.m_ShaderStageCreateInfo.data(),
            &vertexInputStateCreateInfo,
            &inputAssemblyStateCreateInfo,
            nullptr,
            &viewportStateCreateInfo,
            &rasterizationStateCreateInfo,
            &multisampleStateCreateInfo,
            nullptr,
            &colorBlendStateCreateInfo,
            &dynamicStateCreateInfo,
            *m_Layout,
            renderPass
    );

    m_Pipeline = device.createGraphicsPipelineUnique({}, pipelineCreateInfo);
}
