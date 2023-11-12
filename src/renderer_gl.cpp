#include "renderer_gl.h"
#include <glad/glad.h> 

namespace jgfx::gl {
  bool RenderContextGL::init(const InitInfo& createInfo) {

    return true;
  }

  void RenderContextGL::shutdown() {

  }

  void RenderContextGL::updateResolution(const Resolution& resolution) {
    glViewport(0, 0, resolution.width, resolution.height);
  }

  void RenderContextGL::newPipeline(PipelineHandle handle, const PipelineDesc& pipelineDesc) {

  }

  void RenderContextGL::newPass(PassHandle handle, const PassDesc& passDesc) {

  }

  void RenderContextGL::newShader(ShaderHandle handle, const void* binData, uint32_t size) {
    _shaders[handle.id].create();
  }

  void RenderContextGL::newProgram(ProgramHandle handle) {
    _programs[handle.id].create();
  }

  void RenderContextGL::newBuffer(BufferHandle handle, const void* data, uint32_t size, BufferType type) {
    _buffers[handle.id].create();
  }

  void RenderContextGL::newUniformBuffer(UniformBufferHandle handle, uint32_t size) {

  }

  void RenderContextGL::newImage(ImageHandle handle, const void* data, uint32_t size, const TextureDesc& desc) {
    _textures[handle.id].create();
  }

  void RenderContextGL::beginDefaultPass() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void RenderContextGL::beginPass(PassHandle pass) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void RenderContextGL::applyPipeline(PipelineHandle pipe) {

  }

  void RenderContextGL::applyBindings(const Bindings& bindings) {

  }

  void RenderContextGL::applyUniforms(ShaderStage stage, const void* data, uint32_t size) {

  }

  void RenderContextGL::draw(uint32_t firstVertex, uint32_t vertexCount) {

  }

  void RenderContextGL::drawIndexed(uint32_t firstIndex, uint32_t indexCount) {

  }

  void RenderContextGL::endPass() {

  }

  void RenderContextGL::commitFrame() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  bool TextureGL::create() {
    return false;
  }

  void TextureGL::destroy() {
  }

  bool FramebufferGL::create() {
    return false;
  }

  void FramebufferGL::destroy() {
  }

  bool ShaderGL::create() {
    return false;
  }

  void ShaderGL::destroy() {
  }

  bool BufferGL::create() {
    return false;
  }

  void BufferGL::destroy() {
  }

  bool ProgramGL::create() {
    return false;
  }

  void ProgramGL::destroy() {
  }

}