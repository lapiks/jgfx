#include "jgfx/jgfx.h"
#include "jgfx_impl.h"

namespace jgfx 
{
  static ContextImpl ctx; 

  bool Context::init(const InitInfo& initInfo) {
    return ctx.init(initInfo);
  }

  void Context::shutdown() {
    ctx.shutdown();
  }

  void Context::reset(uint32_t width, uint32_t height) {
    ctx.reset(width, height);
  }

  PipelineHandle Context::newPipeline(ShaderHandle vertex, ShaderHandle fragment, PassHandle pass) {
    return ctx.newPipeline(vertex, fragment, pass);
  }

  PassHandle Context::newPass() {
    return ctx.newPass();
  }

  ShaderHandle Context::newShader(const std::vector<char>& binData) {
    return ctx.newShader(binData);
  }

  void Context::beginDefaultPass() {
    ctx.beginDefaultPass();
  }

  void Context::beginPass(PassHandle pass) {
    ctx.beginPass(pass);
  }

  void Context::applyPipeline(PipelineHandle pipe) {
    ctx.applyPipeline(pipe);
  }

  void Context::draw(uint32_t firstVertex, uint32_t vertexCount) {
    ctx.draw(firstVertex, vertexCount);
  }

  void Context::endPass() {
    ctx.endPass();
  }

  void Context::commitFrame() {
    ctx.commitFrame();
  }

}

