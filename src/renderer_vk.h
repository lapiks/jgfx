#pragma once

#include <vulkan/vulkan.h>

#include "renderer.h"

constexpr int MAX_SHADERS = 512;
constexpr int MAX_PIPELINES = 512;
constexpr int MAX_PASSES = 512;
constexpr int MAX_FRAMEBUFFERS = 512;
constexpr int MAX_BUFFERS = 4<<10;
constexpr int MAX_IMAGES = 4<<10;
constexpr int MAX_FRAMES_IN_FLIGHT = 3;

namespace jgfx {
  struct InitInfo;
  struct PlatformData;
}

namespace jgfx::vk { 
  struct FramebufferVK {
    bool create(VkDevice device, const VkImageView* attachments, VkExtent2D swapChainExtent, VkRenderPass renderPass);
    void destroy(VkDevice device);
    VkFramebuffer _framebuffer;
  };

  struct SwapChainVK {
    bool createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, const Resolution& resolution);
    bool createSurface(VkInstance instance, const PlatformData& platformData);
    bool createImageViews(VkDevice device);
    bool createFramebuffers(VkDevice device, VkRenderPass renderPass);
    void destroy(VkDevice device);
    void destroySurface(VkInstance instance);
    void update(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass renderPass);
    void acquire(VkDevice device);
    void present();
    void setWaitSemaphore(VkSemaphore waitSemaphore);
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
    VkQueue _presentQueue = VK_NULL_HANDLE; // queue supporting presentation operations
    VkFormat _imageFormat;
    VkExtent2D _extent;
    VkSemaphore _imageAvailableSemaphore = VK_NULL_HANDLE; // signal that an image has been acquired from swap chain and is ready for rendering
    VkSemaphore _waitSemaphore = VK_NULL_HANDLE;
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imageViews;
    std::vector<FramebufferVK> _framebuffers;
    uint32_t _currentImageIdx;
    Resolution _resolution;
    bool _needRecreation = false;
  };

  struct ShaderVK {
    bool create(VkDevice device, const void* binData, uint32_t size);
    bool createDescriptorSetLayout(VkDevice device);
    void destroy(VkDevice device);
    VkDescriptorSetLayout _descriptorSetLayout; // describes the kind of descriptors that can be bound
    VkShaderModule _module = VK_NULL_HANDLE;
    VkDescriptorSetLayoutBinding binding;
  };

  struct PassVK {
    bool create(VkDevice device, VkFormat swapChainImageFormat);
    void destroy(VkDevice device);
    VkRenderPass _renderPass = VK_NULL_HANDLE;
  };

  struct PipelineVK {
    bool create(VkDevice device, const ShaderVK& vertex, const ShaderVK& fragment, const PassVK& pass, VertexAttributes attr);
    void destroy(VkDevice device);
    VkPipeline _graphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
  };

  struct BufferVK {
    bool create(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t size, BufferType type, void** mappedMemory);
    void unmapMemory(VkDevice device);
    void destroy(VkDevice device);
    VkBuffer _buffer = VK_NULL_HANDLE;
    VkDeviceMemory _memory = VK_NULL_HANDLE;
    uint32_t _size = 0;
  };

  struct UniformBufferVK {
    bool create(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t size);
    void update(const void* data, uint32_t size, uint32_t currentFrame);
    bool createDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t currentFram);
    void destroy(VkDevice device);
    BufferVK _buffers[MAX_FRAMES_IN_FLIGHT];
    void* _mappedMemory[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSet _descriptorSets[MAX_FRAMES_IN_FLIGHT];
  };

  struct CommandQueueVK {
    void begin();
    void end();
    bool createCommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    bool createCommandBuffers(VkDevice device);
    bool createSyncObjects(VkDevice device);
    void destroy(VkDevice device);
    void beginPass(VkRenderPass pass, VkFramebuffer framebuffer, const VkExtent2D& extent);
    void endPass();
    void applyPipeline(VkPipeline pipeline, const VkExtent2D& extent);
    void bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* vertexBuffers);
    void bindIndexBuffer(VkBuffer indexBuffe);
    void bindDescriptorSets(VkPipelineLayout pipelineLayout, const VkDescriptorSet* descriptorSets);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void drawIndexed(uint32_t firstIndex, uint32_t indexCount);
    void submit();
    void newFrame(VkDevice device);
    void setWaitSemaphore(VkSemaphore waitSemaphore);
    VkCommandPool _commandPool = VK_NULL_HANDLE;
    VkCommandBuffer _commandBuffers[MAX_FRAMES_IN_FLIGHT];
    VkFence _inFlightFences[MAX_FRAMES_IN_FLIGHT]; // wait for frame ending to start a new one
    VkQueue _graphicsQueue = VK_NULL_HANDLE; // queue supporting draw operations
    VkSemaphore _renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT]; // signal that rendering has finished and presentation can happen
    VkSemaphore _waitSemaphore = VK_NULL_HANDLE;
    uint32_t _currentFrame = 0;
  };

  struct ImageVK {

  };

  struct RenderContextVK : public RenderContext {
    // Initialization
    bool init(const InitInfo& createInfo) override;
    void shutdown() override;
    bool pickPhysicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    bool createLogicalDevice(VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
    bool createDescriptorPool();
    VkResult createDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);
    void updateResolution(const Resolution& resolution);

    // ObjectVK creation
    void newPipeline(PipelineHandle handle, const PipelineDesc& pipelineDesc);
    void newPass(PassHandle handle, const PassDesc& passDesc);
    void newShader(ShaderHandle handle, const void* binData, uint32_t size) override;
    void newBuffer(BufferHandle handle, const void* data, uint32_t size, BufferType type);
    void newUniformBuffer(UniformBufferHandle handle, uint32_t size);
    void newImage(ImageHandle handle);

    // cmds
    void beginDefaultPass();
    void beginPass(PassHandle pass);
    void applyPipeline(PipelineHandle pipe);
    void applyBindings(const Bindings& bindings);
    void applyUniforms(ShaderStage stage, const void* data, uint32_t size);
    void draw(uint32_t firstVertex, uint32_t vertexCount);
    void drawIndexed(uint32_t firstIndex, uint32_t indexCount);
    void endPass();
    void commitFrame();
    
  private:
    VkInstance _instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
    
    // TODO: pas fou
    PipelineHandle _currentPipeline;
    ShaderHandle _currentVertexShader;
    ShaderHandle _currentFragmentShader;

    SwapChainVK _swapChain;
    CommandQueueVK _cmdQueue;
    PassVK _defaultPass;
    ShaderVK _shaders[MAX_SHADERS];
    PipelineVK _pipelines[MAX_PIPELINES];
    PassVK _passes[MAX_PASSES];  
    BufferVK _buffers[MAX_BUFFERS];
    UniformBufferVK _uniformBuffers[MAX_BUFFERS];
    ImageVK _images[MAX_IMAGES];
    uint32_t _currentUniformBufferId;
  };
}