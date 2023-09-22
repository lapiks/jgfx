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

  PipelineHandle Context::newPipeline(ShaderHandle vertex, ShaderHandle fragment) {
    return ctx.newPipeline(vertex, fragment);
  }

  PassHandle Context::newPass() {
    return ctx.newPass();
  }

  ShaderHandle Context::newShader(const std::vector<char>& binData) {
    return ctx.newShader(binData);
  }

  ProgramHandle Context::newProgram(const std::vector<char>& binData) {
    return ctx.newProgram();
  }
}

