#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "jgfx/jgfx.h"

#include "common.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

GLFWwindow* initWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  return glfwCreateWindow(WIDTH, HEIGHT, "jgfx", nullptr, nullptr);
}

class App {
public:
  void init() {
    window = initWindow();

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";

    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    jgfx::PlatformData pd;
    pd.nativeWindowHandle = glfwGetWin32Window(window);

    jgfx::InitInfo initInfos;
    initInfos.platformData = pd;
    initInfos.extensionCount = extensionCount;
    initInfos.extensionNames = glfwExtensions;
    initInfos.resolution = { WIDTH, HEIGHT };

    ctx.init(initInfos);

    std::vector<char> vertBin = utils::readFile("../shaders/vert.spv");
    std::vector<char> fragBin = utils::readFile("../shaders/frag.spv");

    jgfx::ShaderHandle vs = ctx.newShader(vertBin);
    jgfx::ShaderHandle fs = ctx.newShader(fragBin);

    _pass = ctx.newPass();
    _pipeline = ctx.newPipeline(vs, fs, _pass);
  }

  void draw() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      // render code
      ctx.beginDefaultPass();
      ctx.applyPipeline(_pipeline);
      ctx.draw(0, 3);
      ctx.endPass();
      ctx.commitFrame();
    }
  }

  void shutdown() {
    ctx.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  jgfx::PassHandle _pass;
  jgfx::PipelineHandle _pipeline;

private:
  GLFWwindow* window = nullptr;
  jgfx::Context ctx;
};

int main() {
  App app;
  app.init();
  app.draw();
  app.shutdown();

  return 0;
}