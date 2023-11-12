#include "renderer_gl.h"

namespace jgfx::gl {
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

  bool RenderContextGL::init(const InitInfo& createInfo) {
    return false;
  }

  void RenderContextGL::shutdown() {

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

  void RenderContextGL::newImage(ImageHandle handle, const void* data, uint32_t size, const TextureDesc& desc) {
    _textures[handle.id].create();
  }

}