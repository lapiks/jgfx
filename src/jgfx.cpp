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

  PipelineHandle Context::newPipeline(ShaderHandle vertex, ShaderHandle fragment, PassHandle pass, VertexAttributes attr) {
    return ctx.newPipeline(vertex, fragment, pass, attr);
  }

  PassHandle Context::newPass() {
    return ctx.newPass();
  }

  ShaderHandle Context::newShader(const std::vector<char>& binData) {
    return ctx.newShader(binData);
  }

  BufferHandle Context::newBuffer(const void* data, uint32_t size, BufferType type) {
    return ctx.newBuffer(data, size, type);
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

  void Context::applyBindings(const Bindings& bindings) {
    ctx.applyBindings(bindings);
  }

  void Context::applyUniforms(ShaderStage stage, const void* data, uint32_t size) {
    ctx.applyUniforms(stage, data, size);
  }

  void Context::draw(uint32_t firstVertex, uint32_t vertexCount) {
    ctx.draw(firstVertex, vertexCount);
  }

  void Context::drawIndexed(uint32_t firstIndex, uint32_t indexCount) {
    ctx.drawIndexed(firstIndex, indexCount);
  }

  void Context::endPass() {
    ctx.endPass();
  }

  void Context::commitFrame() {
    ctx.commitFrame();
  }

  void VertexAttributes::begin() {
    memset(_offsets, 0, sizeof(_offsets));
    memset(_types, UNKNOWN, sizeof(_types));
    _stride = 0;
    _attrCount = 0;
  }

  uint32_t getSizeOf(AttribType type) {
    switch (type) {
    case UNKNOWN: return 0;
    case FLOAT: return sizeof(float);
    case FLOAT2: return 2 * sizeof(float);
    case FLOAT3: return 3 * sizeof(float);
    case FLOAT4: return 4 * sizeof(float);
    }

    return 0;
  }

  void VertexAttributes::add(uint32_t location, AttribType type) {
    if (location >= MAX_VERTEX_ATTRIBUTES)
      return; // todo error handling

    _offsets[location] = _stride;
    _types[location] = type;
    _stride += getSizeOf(type);
    _attrCount += 1;
  }

  void VertexAttributes::end() {

  }
}

