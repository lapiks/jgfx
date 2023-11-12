#include "renderer_gl.h"
#include <glad/glad.h> 
#include "spirv_reader.h"

namespace jgfx::gl {
  GLint toGLShaderType(ShaderType type) {
    switch (type) {
    case ShaderType::VERTEX: return GL_VERTEX_SHADER;
    case ShaderType::FRAGMENT: return GL_FRAGMENT_SHADER;
    case ShaderType::COMPUTE: return GL_COMPUTE_SHADER;
    }

    return -1;
  }

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

  void RenderContextGL::newShader(ShaderHandle handle, ShaderType type, const void* binData, uint32_t size) {
    _shaders[handle.id].create(type, size, binData);
  }

  void RenderContextGL::newProgram(ProgramHandle handle) {
    _programs[handle.id].create();
  }

  void RenderContextGL::newBuffer(BufferHandle handle, const void* data, uint32_t size, BufferType type) {
    _buffers[handle.id].create(size, data);
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

  bool ShaderGL::create(ShaderType type, uint32_t size, const void* data) {
    const char* src = read(data, size).c_str();

    unsigned int shader;
    shader = glCreateShader(toGLShaderType(type));
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      //std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
      return false;
    }

    return true;
  }

  void ShaderGL::destroy() {
  }

  bool BufferGL::create(uint32_t size, const void* data) {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
  }

  void BufferGL::destroy() {
  }

  bool ProgramGL::create() {
    return false;
  }

  void ProgramGL::destroy() {
  }

}