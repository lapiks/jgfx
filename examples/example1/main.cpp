#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <chrono>

#include "jgfx/jgfx.h"

#include "common.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class App {
public:
  static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    app->ctx.reset(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    app->_uniforms.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
    app->_uniforms.proj[1][1] *= -1;
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

    const float vertices[] = {
        -1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
         1.0, -1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
         1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,
        -1.0,  1.0, -1.0,   1.0, 0.0, 0.0, 1.0,

        -1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
         1.0, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
         1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
        -1.0,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,

        -1.0, -1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0,  1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0,  1.0,  1.0,   0.0, 0.0, 1.0, 1.0,
        -1.0, -1.0,  1.0,   0.0, 0.0, 1.0, 1.0,

        1.0, -1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
        1.0,  1.0, -1.0,    1.0, 0.5, 0.0, 1.0,
        1.0,  1.0,  1.0,    1.0, 0.5, 0.0, 1.0,
        1.0, -1.0,  1.0,    1.0, 0.5, 0.0, 1.0,

        -1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,
        -1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
         1.0, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
         1.0, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,

        -1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0,
        -1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
         1.0,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
         1.0,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0
    };

    const uint16_t indices[] = {
      0, 1, 2,  0, 2, 3,
      6, 5, 4,  7, 6, 4,
      8, 9, 10,  8, 10, 11,
      14, 13, 12,  15, 14, 12,
      16, 17, 18,  16, 18, 19,
      22, 21, 20,  23, 22, 20
    };

    jgfx::VertexAttributes attr;
    attr.begin();
    attr.add(0, jgfx::FLOAT3);
    attr.add(1, jgfx::FLOAT4);
    attr.end();

    jgfx::BufferHandle vb = ctx.newBuffer(vertices, sizeof(vertices), jgfx::VERTEX_BUFFER);
    jgfx::BufferHandle ib = ctx.newBuffer(indices, sizeof(indices), jgfx::INDEX_BUFFER);

    std::vector<char> vertBin = utils::readFile("../shaders/vert.spv");
    std::vector<char> fragBin = utils::readFile("../shaders/frag.spv");

    jgfx::ShaderHandle vs = ctx.newShader(vertBin);
    jgfx::ShaderHandle fs = ctx.newShader(fragBin);

    _pass = ctx.newPass();

    _pipeline = ctx.newPipeline(
      jgfx::PipelineDesc{
        .vs = vs,
        .fs = fs,
        .vertexAttributes = attr,
        .cullMode = jgfx::FRONT
      }
    );

    _bindings.vertexBuffers[0] = vb;
    _bindings.indexBuffer = ib;

    _uniforms.view = glm::lookAt(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    _uniforms.proj = glm::perspective(glm::radians(45.0f), WIDTH / (float)HEIGHT, 0.1f, 10.0f);
    _uniforms.proj[1][1] *= -1;
  }

  void draw() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      static auto startTime = std::chrono::high_resolution_clock::now();
      auto currentTime = std::chrono::high_resolution_clock::now();
      float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

      _uniforms.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

      // render code
      ctx.beginDefaultPass();
      ctx.applyPipeline(_pipeline);
      ctx.applyBindings(_bindings);
      ctx.applyUniforms(jgfx::VERTEX, &_uniforms, sizeof(_uniforms));
      ctx.drawIndexed(0, 36);
      ctx.endPass();
      ctx.commitFrame();
    }
  }

  void shutdown() {
    ctx.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  struct Uniforms {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

  jgfx::PassHandle _pass;
  jgfx::PipelineHandle _pipeline;
  jgfx::Bindings _bindings;
  Uniforms _uniforms;

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