#pragma once

#include <vector>

#include "jgfx/jgfx.h"

namespace jgfx {
  constexpr int MAX_SHADERS = 512;
  constexpr int MAX_PROGRAMS = 512;
  constexpr int MAX_PIPELINES = 512;
  constexpr int MAX_PASSES = 512;
  constexpr int MAX_FRAMEBUFFERS = 512;
  constexpr int MAX_BUFFERS = 4 << 10;
  constexpr int MAX_IMAGES = 4 << 10;
  constexpr int MAX_FRAMES_IN_FLIGHT = 3;

  struct RenderContext {
    virtual bool init(const InitInfo& createInfo) = 0;
    virtual void shutdown() = 0;
    virtual void updateResolution(const Resolution& resolution) = 0;

    // Objects creation
    virtual void newPipeline(PipelineHandle handle, const PipelineDesc& pipelineDesc) = 0;
    virtual void newPass(PassHandle handle, const PassDesc& passDesc) = 0;
    virtual void newShader(ShaderHandle handle, ShaderType type, const void* binData, uint32_t size) = 0;
    virtual void newProgram(ProgramHandle handle, ShaderHandle vs, ShaderHandle fs) = 0;
    virtual void newBuffer(BufferHandle handle, const void* data, uint32_t size, BufferType type) = 0;
    virtual void newUniformBuffer(UniformBufferHandle handle, uint32_t size) = 0;
    virtual void newImage(ImageHandle handle, const void* data, uint32_t size, const TextureDesc& desc) = 0;

    // cmds
    virtual void beginDefaultPass() = 0;
    virtual void beginPass(PassHandle pass) = 0;
    virtual void applyPipeline(PipelineHandle pipe) = 0;
    virtual void applyBindings(const Bindings& bindings) = 0;
    virtual void applyUniforms(ShaderStage stage, const void* data, uint32_t size) = 0;
    virtual void draw(uint32_t firstVertex, uint32_t vertexCount) = 0;
    virtual void drawIndexed(uint32_t firstIndex, uint32_t indexCount) = 0;
    virtual void endPass() = 0;
    virtual void commitFrame() = 0;
  };
}
