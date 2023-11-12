#pragma once
#include "jgfx_impl.h"

#include "jgfx/jgfx.h"

#include "renderer_vk.h"
#include "renderer_gl.h"

namespace jgfx {
  bool ContextImpl::init(const InitInfo& initInfo) {
    if (_ctx) // already initialized
      return false;

    switch (initInfo.api)
    {
    case GraphicsAPI::Vulkan: _ctx = std::make_unique<vk::RenderContextVK>(); break;
    case GraphicsAPI::OpenGL: _ctx = std::make_unique<gl::RenderContextGL>(); break;
    }

    _initInfo = initInfo;
    return _ctx->init(initInfo);
  }

  void ContextImpl::shutdown() {
    _ctx->shutdown();
  }

  void ContextImpl::reset(uint32_t width, uint32_t height) {
    _initInfo.resolution.width = width;
    _initInfo.resolution.height = height;
    _reset = true;
  }

  PipelineHandle ContextImpl::newPipeline(const PipelineDesc& pipelineDesc) {
    CommandBuffer& cmdBuf = startCommand(CommandType::NewPipeline);
    PipelineHandle handle;
    pipelineHandleAlloc.allocate(handle);
    cmdBuf.write(handle);
    cmdBuf.write(pipelineDesc);

    return handle;
  }

  PassHandle ContextImpl::newPass(const PassDesc& passDesc) {
    CommandBuffer& cmdBuf = startCommand(CommandType::NewPass);
    PassHandle handle;
    passHandleAlloc.allocate(handle);
    cmdBuf.write(handle);
    cmdBuf.write(passDesc);

    return handle;
  }

  ShaderHandle ContextImpl::newShader(ShaderType type, const void* binData, uint32_t size) {
    CommandBuffer& cmdBuf = startCommand(CommandType::NewShader);
    ShaderHandle handle;
    shaderHandleAlloc.allocate(handle);
    cmdBuf.write(handle);
    cmdBuf.write(type);
    cmdBuf.write(binData);
    cmdBuf.write(size);

    return handle;
  }

  BufferHandle ContextImpl::newBuffer(const void* data, uint32_t size, BufferType type) {
    CommandBuffer& cmdBuf = startCommand(CommandType::NewBuffer);
    BufferHandle handle;
    bufferHandleAlloc.allocate(handle);
    cmdBuf.write(handle);
    cmdBuf.write(data);
    cmdBuf.write(size);
    cmdBuf.write(type);

    return handle;
  }

  UniformBufferHandle ContextImpl::newUniformBuffer(uint32_t size) {
    CommandBuffer& cmdBuf = startCommand(CommandType::NewUniformBuffer);
    UniformBufferHandle handle;
    uniformBufferHandleAlloc.allocate(handle);
    cmdBuf.write(handle);
    cmdBuf.write(size);

    return handle;
  }

  ImageHandle ContextImpl::newImage(const void* data, uint32_t size, const TextureDesc& desc) {
    CommandBuffer& cmdBuf = startCommand(CommandType::NewImage);
    ImageHandle handle;
    imageHandleAlloc.allocate(handle);
    cmdBuf.write(handle);
    cmdBuf.write(data);
    cmdBuf.write(size);
    cmdBuf.write(desc);

    return handle;
  }

  void ContextImpl::beginDefaultPass() {
    startCommand(CommandType::BeginDefaultPass);
  }

  void ContextImpl::beginPass(PassHandle pass) {
    CommandBuffer& cmdBuf = startCommand(CommandType::BeginPass);
    cmdBuf.write(pass);
  }

  void ContextImpl::applyPipeline(PipelineHandle pipe) {
    CommandBuffer& cmdBuf = startCommand(CommandType::ApplyPipeline);
    cmdBuf.write(pipe);
  }

  void ContextImpl::applyBindings(const Bindings& bindings) {
    CommandBuffer& cmdBuf = startCommand(CommandType::ApplyBindings);
    cmdBuf.write(bindings);
  }

  void ContextImpl::applyUniforms(ShaderStage stage, const void* data, uint32_t size) {
    CommandBuffer& cmdBuf = startCommand(CommandType::ApplyUniforms);
    cmdBuf.write(stage);
    cmdBuf.write(data);
    cmdBuf.write(size);
  }

  void ContextImpl::draw(uint32_t firstVertex, uint32_t vertexCount) {
    CommandBuffer& cmdBuf = startCommand(CommandType::Draw);
    cmdBuf.write(firstVertex);
    cmdBuf.write(vertexCount);
  }

  void ContextImpl::drawIndexed(uint32_t firstIndex, uint32_t indexCount) {
    CommandBuffer& cmdBuf = startCommand(CommandType::DrawIndexed);
    cmdBuf.write(firstIndex);
    cmdBuf.write(indexCount);
  }

  void ContextImpl::endPass() {
    startCommand(CommandType::EndPass);
  }

  void ContextImpl::commitFrame() {
    if (_reset) {
      _ctx->updateResolution(_initInfo.resolution);
      _reset = false;
    }
    executeCommands();
    _ctx->commitFrame();
  }

  CommandBuffer& ContextImpl::startCommand(CommandType cmdType) {
    _cmdBuffer.write(cmdType);
    return _cmdBuffer;
  }

  void ContextImpl::executeCommands()
  {
    startCommand(CommandType::End);
    _cmdBuffer.reset();

    bool end = false;
    do {
      CommandType type;
      _cmdBuffer.read(type);
 
      switch (type) {
      case NewPipeline: {
        PipelineHandle handle;
        _cmdBuffer.read(handle);
        PipelineDesc desc;
        _cmdBuffer.read(desc);
        _ctx->newPipeline(handle, desc);
      }
        break;
      case NewPass: {
        PassHandle handle;
        _cmdBuffer.read(handle);
        PassDesc desc;
        _cmdBuffer.read(desc);
        _ctx->newPass(handle, desc);
      }
        break;
      case NewShader: {
        ShaderHandle handle;
        _cmdBuffer.read(handle);
        ShaderType type;
        _cmdBuffer.read(type);
        void* data = nullptr;
        _cmdBuffer.read(data);
        uint32_t size;
        _cmdBuffer.read(size);
        _ctx->newShader(handle, type, data, size);
      }
        break;
      case NewBuffer: {
        BufferHandle handle;
        _cmdBuffer.read(handle);
        void* data = nullptr;
        _cmdBuffer.read(data);
        uint32_t size;
        _cmdBuffer.read(size);
        BufferType type;
        _cmdBuffer.read(type);
        _ctx->newBuffer(handle, data, size, type);
      }
        break;
      case NewUniformBuffer: {
        UniformBufferHandle handle;
        _cmdBuffer.read(handle);
        uint32_t size;
        _cmdBuffer.read(size);
        _ctx->newUniformBuffer(handle, size);
      }
        break;
      case NewImage: {
        ImageHandle handle;
        _cmdBuffer.read(handle);
        uint8_t* data = nullptr;
        _cmdBuffer.read(data);
        uint32_t size;
        _cmdBuffer.read(size);
        TextureDesc desc;
        _cmdBuffer.read(desc);
        _ctx->newImage(handle, data, size, desc);
      }
        break;
      case BeginDefaultPass: {
        _ctx->beginDefaultPass();
      }
        break;
      case BeginPass: {
        PassHandle pass;
        _cmdBuffer.read(pass);
        _ctx->beginPass(pass);
      }
        break;
      case ApplyPipeline: {
        PipelineHandle pipe;
        _cmdBuffer.read(pipe);
        _ctx->applyPipeline(pipe);
      }
        break;
      case ApplyBindings: {
        Bindings bindings;
        _cmdBuffer.read(bindings);
        _ctx->applyBindings(bindings);
      }
        break;
      case ApplyUniforms: {
        ShaderStage stage;
        _cmdBuffer.read(stage);
        void* data;
        _cmdBuffer.read(data);
        uint32_t size;
        _cmdBuffer.read(size);
        _ctx->applyUniforms(stage, data, size);
      }
        break;
      case Draw: {
        uint32_t firstVertex;
        _cmdBuffer.read(firstVertex);
        uint32_t vertexCount;
        _cmdBuffer.read(vertexCount);
        _ctx->draw(firstVertex, vertexCount);
      }
        break;
      case DrawIndexed: {
        uint32_t firstIndex;
        _cmdBuffer.read(firstIndex);
        uint32_t indexCount;
        _cmdBuffer.read(indexCount);
        _ctx->drawIndexed(firstIndex, indexCount);
      }
        break;
      case EndPass: {
        _ctx->endPass();
      }
        break;
      case End: {
        end = true;
      }
        break;
      }
    } while (!end);

    _cmdBuffer.reset();
  }
}
