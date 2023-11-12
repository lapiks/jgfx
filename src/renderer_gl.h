#pragma once

#include "renderer.h"

namespace jgfx::gl {
  struct TextureGL {
    bool create();
    void destroy();
  };

  struct FramebufferGL {
    bool create();
    void destroy();

    unsigned int _id;
  };

  struct ShaderGL {
    bool create(ShaderType type, uint32_t size, const void* data);
    void destroy();

    unsigned int _id;
  };

  struct ProgramGL {
    bool create(const ShaderGL& vs, const ShaderGL& fs);
    void destroy();

    unsigned int _id;
  };

  struct BufferGL {
    bool create(uint32_t size, const void* data);
    void destroy();

    unsigned int _id;
  };

  struct RenderContextGL : public RenderContext {
    // Initialization
    bool init(const InitInfo& createInfo) override;
    void shutdown() override;
    void updateResolution(const Resolution& resolution) override;

    // ObjectGL creation
    void newPipeline(PipelineHandle handle, const PipelineDesc& pipelineDesc) override;
    void newPass(PassHandle handle, const PassDesc& passDesc) override;
    void newShader(ShaderHandle handle, ShaderType type, const void* binData, uint32_t size) override;
    void newProgram(ProgramHandle handle, ShaderHandle vsHandle, ShaderHandle fsHandle) override;
    void newBuffer(BufferHandle handle, const void* data, uint32_t size, BufferType type) override;
    void newUniformBuffer(UniformBufferHandle handle, uint32_t size) override;
    void newImage(ImageHandle handle, const void* data, uint32_t size, const TextureDesc& desc) override;

    // cmds
    void beginDefaultPass() override;
    void beginPass(PassHandle pass) override;
    void applyPipeline(PipelineHandle pipe) override;
    void applyBindings(const Bindings& bindings) override;
    void applyUniforms(ShaderStage stage, const void* data, uint32_t size) override;
    void draw(uint32_t firstVertex, uint32_t vertexCount) override;
    void drawIndexed(uint32_t firstIndex, uint32_t indexCount) override;
    void endPass() override;
    void commitFrame() override;

    ShaderGL _shaders[MAX_SHADERS];
    ProgramGL _programs[MAX_PROGRAMS];
    BufferGL _buffers[MAX_BUFFERS];
    TextureGL _textures[MAX_IMAGES];
    FramebufferGL _framebuffer[MAX_FRAMEBUFFERS];
  };
}