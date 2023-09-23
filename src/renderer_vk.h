#pragma once

#include <vulkan/vulkan.h>

#include "renderer.h"

constexpr int MAX_SHADERS = 512;
constexpr int MAX_PIPELINES = 512;
constexpr int MAX_PASSES = 512;
constexpr int MAX_FRAMEBUFFERS = 512;

namespace jgfx {
  struct InitInfo;
  struct PlatformData;
}

namespace jgfx::vk { 
  struct FramebufferVK {
    bool create(VkDevice device, const std::vector<VkImageView>& attachments, VkExtent2D swapChainExtent, VkRenderPass renderPass);
    void destroy(VkDevice device);
    VkFramebuffer _framebuffer;
  };

  struct SwapChainVK {
    bool createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, const Resolution& resolution);
    bool createSurface(VkInstance instance, const PlatformData& platformData);
    bool createImageViews(VkDevice device);
    bool createFramebuffers(VkDevice device);
    void destroy(VkDevice device, VkInstance instance);
    VkSurfaceKHR _surface;
    VkSwapchainKHR _swapChain;
    VkFormat _imageFormat;
    VkExtent2D _extent;
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imageViews;
    std::vector<FramebufferVK> _framebuffers;
  };

  struct ShaderVK {
    bool create(VkDevice device, const std::vector<char>& bytecode);
    void destroy(VkDevice device);
    VkShaderModule _module;
  };

  struct PassVK {
    bool create(VkDevice device, VkFormat swapChainImageFormat);
    void destroy(VkDevice device);
    VkRenderPass _renderPass;
  };

  struct PipelineVK {
    bool create(VkDevice device, const ShaderVK& vertex, const ShaderVK& fragment, const PassVK& pass);
    void destroy(VkDevice device);
    VkPipeline _graphicsPipeline;
    VkPipelineLayout _pipelineLayout;
  };

  struct RenderContextVK : public RenderContext {
    bool init(const InitInfo& createInfo) override;
    void shutdown() override;
    bool pickPhysicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    bool createLogicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    bool createCommandPool();
    bool createCommandBuffer();

    void newPipeline(PipelineHandle handle, ShaderHandle vertex, ShaderHandle fragment, PassHandle pass);
    void newPass(PassHandle handle);
    void newShader(ShaderHandle handle, const std::vector<char>& bytecode) override;

  private:
    VkInstance _instance;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device;
    VkQueue _graphicsQueue; // queue supporting draw operations
    VkQueue _presentQueue; // queue supporting presentation operations
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;

    SwapChainVK _swapChain;
    ShaderVK _shaders[MAX_SHADERS];
    PipelineVK _pipelines[MAX_PIPELINES];
    PassVK _passes[MAX_PASSES];  
  };
}