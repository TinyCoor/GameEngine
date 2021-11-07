//
// Created by y123456 on 2021/10/11.
//

#ifndef GAMEENGINE_VULKANSWAPCHAIN_H
#define GAMEENGINE_VULKANSWAPCHAIN_H
#include "VulkanRenderContext.h"
#include <volk.h>
#include <vector>


struct VulkanRenderFrame
{

    VkDescriptorSet swapchainDescriptorSet{VK_NULL_HANDLE};

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VkFramebuffer frameBuffer{VK_NULL_HANDLE};

    VkBuffer uniformBuffers{VK_NULL_HANDLE};
    VkDeviceMemory uniformBuffersMemory{VK_NULL_HANDLE};

};

struct RenderState;

class VulkanSwapChain{
public:
    VulkanSwapChain(const VulkanRenderContext& ctx);
    virtual ~VulkanSwapChain();

    void init(VkDeviceSize uboSize,int width,int height);
    bool Acquire(const RenderState& state,VulkanRenderFrame& frame);

    bool  Present( VulkanRenderFrame& frame);

    void shutdown();

    inline VkExtent2D getExtent() const{ return swapChainExtent;}
    inline VkRenderPass getRenderPass() const{return renderPass;}
    inline VkDescriptorSetLayout getDescriptorSetLayout() const{return descriptorSetLayout;}
    inline uint32_t getNumImages()const {return swapChainImageViews.size();}

private:

    struct SupportedDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct Settings{
        VkSurfaceFormatKHR format;
        VkPresentModeKHR presentMode;
        VkExtent2D extent;
    };


    SupportedDetails fetchSwapchainSupportedDetails(VkPhysicalDevice& physical_device,
                                                             VkSurfaceKHR& surface);
    Settings selectOptimalSwapchainSettings(SupportedDetails& details,int width,int height);

private:
    void initFrame(VkDeviceSize uboSize);
    void shutFrame();
private:
    VulkanRenderContext context;
    VkSwapchainKHR  swapchain{VK_NULL_HANDLE};

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};


    //
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkFormat swapChainImageFormat; //color format
    VkExtent2D swapChainExtent;
    VkDeviceSize uboSize;


    VkImage colorImage;
    VkImageView colorImageView;
    VkDeviceMemory colorImageMemory;

    VkImage depthImage;
    VkFormat depthFormat;
    VkImageView depthImageView;
    VkDeviceMemory depthImageMemory;

    //
    std::vector<VulkanRenderFrame> frames;

    //SwapChain

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    enum {
        MAX_FRAME_IN_FLIGHT =2,
    };

    uint32_t imageIndex =0;
    size_t currentFrame =0;

};


#endif //GAMEENGINE_VULKANAPPLICATION_H
