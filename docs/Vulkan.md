#Vulkan 
- Vulkan Object  
    - VkInstance: 
      Vulkan 实例对象，连接应用层和驱动层，
        包含应用层的一些基本信息
    - VkPhysicalDevice: 物理硬件的抽象，该对象记录物理硬件的一些信息
  包含内存信息,支持的硬件扩展,同一个实例下可以有多个物理设备
   
    - VkDevice: 基于物理设备创建的逻辑设备，保存物理设备的相应资源
    - VkCommandPool: 用于分配命令缓冲
    - VkCommandBuffer: 用于记录绘制或者计算的命令，有CommandPool分配
    - VKCommandBufferBeginInfo:携带记录启动命令缓冲的信息
    - VkSubmitInfo: 携带提交给队列执行时必要的信息，包含等待信号量列表等
    - VkQueueFamilyProperties:携带特定队列的家族属性信息，包含队列的数量和能力
    - VkQueue: 接受提交的任务，将任务交由GPU执行
    - VkFormat: 记录Vulkan用到的各种内存组织格式
    - VkExtent2D:记录宽高信息
    - VkImage: 一种设备内存的使用模式，用于存储像素, 每个Image 必须要分配对应的ImageView和VkDeviceMemory
    - VkImageView: 搭配VKImage 使用记录图像的信息
    - VkSwapChainKHR:将画面呈现到特定的平台的机制，目标平台的呈现机制的抽象
    - VkFrameBuffer:帧缓存可以包含多个附件Color Depth Stencil
    - VkBuffer: 设备内存的使用模式，储存各种数据
    - VkDescriptorBufferInfo：描述缓冲信息的结构体，包含对应缓冲，内存偏移，范围
    - VkRenderPass: 包含一次绘制需要的信息，如颜色附件，深度附件，子通道等信息
    - VkClearValue: 用于清楚颜色或深度附件的信息
    - VKRenderPassBegin:启动RenderPass需要的信息
    - VkSubpssDescription: 描述子通道的信息
    - VkDescriptorSetLayout:给出着色其中的变量，类型，绑定编号对于什么阶段
    - VkDescriptorSet:通过SetLayout将需要的资源与着色器链接，协助着色器加载资源
    - VkWriteDescriptorSet:绘制前更新Uniform变量
    - VkDescriptorPool 分配DescriptorSet
    - VkPipelineLayout:描述管线的整体布局，有哪些推送常量和descriptorSet
    - VkPipeline:渲染管线的抽象，携带渲染需要的信息
    - VkPipelineShaderStageCreateInfo:创建着色器对象所需的信息
    - VkVertexInputBindingDescription:用于描述输入顶点数据的布局和绑定位置,类似于OpenGL的VAO
    - VkPipelineCache:便于高效创建管线
    - VkFormatProperties:存储指定格式类型
    - VkPhysicalDeviceMemoryProperties:GPU内存信息
    - VkDeviceMemoey:真正的设备内存，所有的Buffer都要绑定内存
    - VkSempore:并发的同步
    - VkFence:主机和设备之间的同步
    - VkSurfaceKHR:对应平台的窗口表面，用于呈现画面
    - VkSurfaceCapacity:平台表面的能力
    - VkPresentInfoKHR：呈现画面所要的信息
##创建Vulkan app
  - 创建 Vulkan Instance
   首先初始化所需扩展列表，创建应用程序结构体实例
```cpp
//枚举实例支持的扩展  
vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
//枚举实例支持的层
vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
VkInstance createInstance(std::vector<const char*>& extensions,
  std::vector<const char*>& layers,
  VkApplicationInfo& appInfo,
  VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo ){
  VkInstance  instance;
  VkInstanceCreateInfo instanceInfo={};
  instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceInfo.pApplicationInfo = &appInfo;
  instanceInfo.enabledExtensionCount = extensions.size();
  instanceInfo.ppEnabledExtensionNames =extensions.data();
  instanceInfo.enabledLayerCount = layers.size();
  instanceInfo.ppEnabledLayerNames = layers.data();
  instanceInfo.pNext = &debugMessengerInfo;
  VK_CHECK(vkCreateInstance(&instanceInfo, nullptr,&instance),"Failed to Create Instance\n");
  return instance;
}
```
  - 获取物理设备列表
   首先获取Vulkan Instance 下的物理设备数量，获取物理设备的内存属性
  - 创建逻辑设备 Vkdevice:获取指定物理设备的队列家族数量和属性，遍历队列家族
记录支持图形工作的队列家族索引，设置逻辑设备需要的扩展，最后创建逻辑设备
  - 创建命令缓冲 ：首先创建所需要的命令缓冲池VkCommandPool  
  - 获取设备中支持图形工作的队列：根据逻辑设备及队列加载索引和队列索引获取VkQueue
  - 初始化VkSwapChain:创建对应平台的surface,遍历所有队列，找到即支持图形绘制和显示工作的队列家族
  获取surface 所支持的格式和数量，获取表面能力，支持的显示模式数量和模式，确定
  宽度和高度，获取swapchain的图像数量和图像列表，创建图像视图
  - 创建深度缓冲;
  - 创建渲染通道(VkRenderPass)：构建通道需要的附件描述结构体数组，第一个用于描述颜色附件
  第二个用于描述深度附件
  - 创建帧缓冲(VkFramebuffer)
  - 创建绘制用的物体
  - 初始化渲染管线
  - 创建栅栏和初始化呈现信息
  - 初始化基本变换矩阵，摄像机举证和投影矩阵
  - 执行绘制
  - 销毁相关对象
