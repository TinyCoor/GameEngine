//
// Created by y123456 on 2021/10/16.
//

#ifndef GAMEENGINE_VULKANTEXTURE_H
#define GAMEENGINE_VULKANTEXTURE_H

#include "VulkanRenderContext.h"
#include <string>

class VulkanTexture {
public:
    VulkanTexture(const VulkanRenderContext& ctx)
    : context(ctx){}
    ~VulkanTexture();

    bool loadHDRFromFile(const std::string& file);
    bool loadFromFile(const std::string& file);
    inline VkImage getImage()const {return image;}
    inline VkImageView getImageView() const { return imageView;}
    inline VkSampler getSampler() const {return imageSampler;}

    void clearGPUData();
    void clearCPUData();
private:
    void uploadToGPU(VkFormat format,size_t size);
private:
    VulkanRenderContext context;
    unsigned char* pixels = nullptr;
    int width =0;
    int height =0;
    int channels =0;
    int mipLevels = 0;

    VkImage image{};
    VkDeviceMemory imageMemory{};
    VkImageView imageView{};
    VkSampler  imageSampler{};
};


#endif //GAMEENGINE_VULKANTEXTURE_H
