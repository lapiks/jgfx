#pragma once
#include "jgfx_impl.h"

#include "jgfx/jgfx.h"

namespace jgfx {
  bool ContextImpl::init(const InitInfo& initInfo) {
    if (vkCtx) // already initialized
      return false;

    vkCtx = std::make_unique<vk::RenderContextVK>();

    _initInfo = initInfo;
    return vkCtx->init(initInfo);
  }

  void ContextImpl::shutdown() {
    vkCtx->shutdown();
  }

  void ContextImpl::reset(uint32_t width, uint32_t height) {
    _initInfo.resolution.width = width;
    _initInfo.resolution.height = height;
    _reset = true;
  }

  PipelineHandle ContextImpl::newPipeline(const PipelineDesc& pipelineDesc) {
    PipelineHandle handle;
    pipelineHandleAlloc.allocate(handle);

    vkCtx->newPipeline(handle, pipelineDesc);

    return handle;
  }

  PassHandle ContextImpl::newPass(const PassDesc& passDesc) {
    PassHandle handle;
    passHandleAlloc.allocate(handle);

    vkCtx->newPass(handle, passDesc);

    return handle;
  }

  ShaderHandle ContextImpl::newShader(const void* binData, uint32_t size) {
    ShaderHandle handle;
    shaderHandleAlloc.allocate(handle);

    vkCtx->newShader(handle, binData, size);

    return handle;
  }

  BufferHandle ContextImpl::newBuffer(const void* data, uint32_t size, BufferType type) {
    BufferHandle handle;
    bufferHandleAlloc.allocate(handle);

    vkCtx->newBuffer(handle, data, size, type);

    return handle;
  }

  UniformBufferHandle ContextImpl::newUniformBuffer(uint32_t size) {
    UniformBufferHandle handle;
    uniformBufferHandleAlloc.allocate(handle);

    vkCtx->newUniformBuffer(handle, size);

    return handle;
  }

  ImageHandle ContextImpl::newImage(const void* data, uint32_t size, const TextureDesc& desc) {
    ImageHandle handle;
    imageHandleAlloc.allocate(handle);

    vkCtx->newImage(handle, data, size, desc);

    return handle;
  }

  void ContextImpl::beginDefaultPass() {
    vkCtx->beginDefaultPass();
  }

  void ContextImpl::beginPass(PassHandle pass) {
    vkCtx->beginPass(pass);
  }

  void ContextImpl::applyPipeline(PipelineHandle pipe) {
    vkCtx->applyPipeline(pipe);
  }

  void ContextImpl::applyBindings(const Bindings& bindings) {
    vkCtx->applyBindings(bindings);
  }

  void ContextImpl::applyUniforms(ShaderStage stage, const void* data, uint32_t size) {
    vkCtx->applyUniforms(stage, data, size);
  }

  void ContextImpl::draw(uint32_t firstVertex, uint32_t vertexCount) {
    vkCtx->draw(firstVertex, vertexCount);
  }

  void ContextImpl::drawIndexed(uint32_t firstIndex, uint32_t indexCount) {
    vkCtx->drawIndexed(firstIndex, indexCount);
  }

  void ContextImpl::endPass() {
    vkCtx->endPass();
  }

  void ContextImpl::commitFrame() {
    if (_reset) {
      vkCtx->updateResolution(_initInfo.resolution);
      _reset = false;
    }
    vkCtx->commitFrame();
  }

}
