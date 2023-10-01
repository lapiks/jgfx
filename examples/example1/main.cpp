#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#include <iostream>

#include "jgfx/jgfx.h"

#include "common.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class App {
public:
  static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    app->ctx.reset(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
  }

  GLFWwindow* initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "jgfx", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    return window;
  }

  void init() {
    window = initWindow();

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";

    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    jgfx::PlatformData pd;
    pd.nativeWindowHandle = glfwGetWin32Window(window);

    jgfx::InitInfo initInfos;
    initInfos.platformData = pd;
    initInfos.extensionNames = extensions;
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