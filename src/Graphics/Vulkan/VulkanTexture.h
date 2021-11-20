//
// Created by y123456 on 2021/10/16.
//

#ifndef GAMEENGINE_VULKANTEXTURE_H
#define GAMEENGINE_VULKANTEXTURE_H

#include "VulkanContext.h"
#include <string>

class VulkanTexture {
public:
    VulkanTexture(const VulkanContext* ctx)
    : context(ctx){}
    ~VulkanTexture();

    void createCube(VkFormat format,int w,int h,int numMipLevels);
    void create2D(VkFormat format,int w,int h,int numMipLevels);

    bool loadFromFile(const std::string& file);
    inline VkImage getImage()const {return image;}
    inline VkImageView getImageView() const { return imageView;}
    inline VkImageView getMipImageView(int mip) const{return mipViews[mip];}
    inline VkFormat getImageFormat() const {return imageFormat;}
    inline VkSampler getSampler() const {return imageSampler;}
    inline size_t getWidth() const{return width;}
    inline int getWidth(int mip) const { return std::max<int>(1, width / (1 << mip)); }
    inline size_t getHeight() const{return height;}
    inline int getHeight(int mip) const { return std::max<int>(1, height / (1 << mip)); }
    inline size_t getNumLayers() const{return layers;}
    inline size_t getNumMiplevels() const{return mipLevels;}

    void clearGPUData();
    void clearCPUData();
private:
    void uploadToGPU(VkFormat format,VkImageTiling tiling,size_t size);
private:
    const VulkanContext* context;
    unsigned char* pixels = nullptr;
    int width =0;
    int height =0;
    int channels =0;
    int mipLevels = 0;
    int layers=0;

    VkFormat imageFormat {VK_FORMAT_UNDEFINED};
    VkImage image{};
    VkDeviceMemory imageMemory{};
    VkImageView imageView{};
    VkSampler  imageSampler{};
    std::vector<VkImageView> mipViews;
};


#endif //GAMEENGINE_VULKANTEXTURE_H
