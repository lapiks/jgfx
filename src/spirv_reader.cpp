#include "spirv_reader.h"

#include "spirv_glsl.hpp"


namespace jgfx {
  std::string read(const void* binData, uint32_t size) {
		spirv_cross::CompilerGLSL glsl(reinterpret_cast<const uint32_t*>(binData), size / 4);

		// Set some options.
		spirv_cross::CompilerGLSL::Options options;
		options.version = 450;
		options.es = false;
		glsl.set_common_options(options);

		// Compile to GLSL, ready to give to GL driver.
		return glsl.compile();
  }
}