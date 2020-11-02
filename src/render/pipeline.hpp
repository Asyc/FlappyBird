#ifndef FLAPPYBIRD_PIPELINE_HPP
#define FLAPPYBIRD_PIPELINE_HPP

#include "render/shader.hpp"

class Pipeline {
public:
    Pipeline(vk::Device device, vk::RenderPass renderPass, Shader&& shader);
private:
    vk::UniquePipelineLayout m_Layout;
    vk::UniquePipeline m_Pipeline;
};

#endif //FLAPPYBIRD_PIPELINE_HPP
