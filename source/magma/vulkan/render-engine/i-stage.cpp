#include "./i-stage.hpp"

#include "../render-engine-impl.hpp"

using namespace lava::magma;

IStage::IStage(RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_renderPass{m_engine.device().vk()}
    , m_pipelineLayout{m_engine.device().vk()}
    , m_pipeline{m_engine.device().vk()}
{
}
