#include "jgfx/jgfx.h"
#include "jgfx_impl.h"

namespace jgfx 
{
  static ContextImpl ctx; 

  bool Context::init(const CreateInfo& init) {
    return ctx.init(init);
  }

  void Context::shutdown() {
    ctx.shutdown();
  }
}

