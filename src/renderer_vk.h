#pragma once

#include <vulkan/vulkan.h>

#include "renderer.h"

constexpr int MAX_SHADERS = 512;
constexpr int MAX_PIPELINES = 512;
constexpr int MAX_PASSES = 512;

namespace jgfx {
  struct InitInfo;
  struct PlatformData;
}

namespace jgfx::vk { 
  struct ShaderVK {
    bool create(VkDevice device, const std::vector<char>& bytecode);
    void destroy(VkDevice device);
    VkShaderModule _module;
  };

  struct PipelineVK {
    bool create(VkDevice device, const ShaderVK& vertex, const ShaderVK& fragment);
    VkPipeline _pipeline;
  };

  struct PassVK {
    bool create(VkDevice device, VkFormat swapChainImageFormat);
    void destroy(VkDevice device);
    VkRenderPass _renderPass;
  };

  struct RenderContextVK : public RenderContext {
    bool init(const InitInfo& createInfo) override;
    void shutdown() override;
    bool createSurface(const PlatformData& platformData);
    bool pickPhysicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    bool createLogicalDevice(const std::vector<const char*>& deviceExtensions);
    bool createSwapChain(const InitInfo& initInfo);
    bool createImageViews();

    void newPipeline(PipelineHandle handle, ShaderHandle vertex, ShaderHandle fragment);
    void newPass(PassHandle handle);
    void newShader(ShaderHandle handle, const std::vector<char>& bytecode) override;
    void newProgram();

  private:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue; // queue supporting draw operations
    VkQueue presentQueue; // queue supporting presentation operations
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkPipelineLayout pipelineLayout;

    ShaderVK shaders[MAX_SHADERS];
    PipelineVK pipelines[MAX_PIPELINES];
    PassVK passes[MAX_PASSES];  
  };
}