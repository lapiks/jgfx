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

    jgfx::CreateInfo initInfos;
    initInfos.platformData = pd;
    initInfos.extensionCount = extensionCount;
    initInfos.extensionNames = glfwExtensions;

    jgfx::Context ctx;
    ctx.init(initInfos);
  }

  void draw() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      // render

    }
  }

  void shutdown() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }
  
private:
  GLFWwindow* window = nullptr;
};

int main() {
  App app;
  app.init(); 

  app.draw();

  return 0;
}