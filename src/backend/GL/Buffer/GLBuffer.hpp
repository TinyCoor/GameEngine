//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLBUFFER_HPP
#define GAMEENGINE_GLBUFFER_HPP
#include "../glad.h"
#include "../GLObject.h"
#include "../GLProgram.h"

template<GLenum type>
class BufferPolicy{
public:
    static constexpr GLenum buffer_type = type;

    static GLHANDLE  createBufferObject(){
        GLHANDLE buffer_handle;
        glCreateBuffers(1,&buffer_handle);
        return buffer_handle;
    }

    static void Bind(GLHANDLE glhandle) {
        glBindBuffer(type,glhandle);
    }

    static void UnBind() {
      glBindBuffer(type,0);
    }

    static void DestroyBuffer(GLHANDLE handle){
        glDeleteBuffers(1,&handle);
    }
};

template<>
class BufferPolicy<GL_FRAMEBUFFER>{
public:
    static constexpr GLenum buffer_type = GL_FRAMEBUFFER;

    static GLHANDLE  createBufferObject(){
        GLHANDLE buffer_handle;
        glCreateFramebuffers(1,&buffer_handle);
        return buffer_handle;
    }

    static void Bind(GLHANDLE glhandle) {
        glBindFramebuffer(buffer_type,glhandle);
    }

    static void UnBind() {
        glBindFramebuffer(buffer_type,0);
    }

    static void DestroyBuffer(GLHANDLE handle){
        glDeleteFramebuffers(1,&handle);
    }
};

template<>
class BufferPolicy<GL_RENDERBUFFER>{
public:
    static constexpr GLenum buffer_type = GL_RENDERBUFFER;

    static GLHANDLE  createBufferObject(){
        GLHANDLE buffer_handle;
        glCreateRenderbuffers(1,&buffer_handle);
        return buffer_handle;
    }

    static void Bind(GLHANDLE glhandle) {
        glBindRenderbuffer(buffer_type,glhandle);
    }

    static void UnBind() {
      glBindRenderbuffer(buffer_type,0);
    }

    static void DestroyBuffer(GLHANDLE handle){
        glDeleteRenderbuffers(1,&handle);
    }
};

//TODO add comment
/**
 *
 * @tparam buf_type
 *      GL_ARRAY_BUFFER:顶点数组缓存，存放顶点数据
 *      ATOMIC_COUNTER_BUFFER
 *      GL_COPY_READ_BUFFER: 用于拷贝缓存之间的数据，不会引起OpenGL状态变化
 *      DISPATCH_INDIRECT_BUFFER
 *      GL_COPY_WRITE_BUFFER: 用于拷贝缓存之间的数据，不会引起OpenGL状态变化
 *      GL_DRAW_INDIRECT_BUFFER:如果采取间接绘制，该缓存存放间接绘制的命令
 *      PARAMETER_BUFFER
 *      QUERY_BUFFER
 *      GL_ELEMENT_ARRAY_BUFFER:存放的是顶点索引数据
 *      GL_PIXEL_PACK_BUFFER: 用于从图像中读取数据
 *      GL_PIXEL_UNPACK_BUFFER: 作为图像的数据来源
 *      GL_TEXTURE_BUFFER:纹理缓存，绑定到纹理对象的缓存,可以在shader 中读取数据
 *      GL_TRANSFORM_FEEDBACK_BUFFER:用于存放被捕获的顶点
 *      GL_SHADER_STORAGE_BUFFER: shader 读写内存
 *      GL_UNIFORM_BUFFER: uniform 缓存变量
 * @tparam BufferTypePolicy
 */
 ///buffer 是线性数组无格式与texture不同,可能用作 texture
template<GLenum buf_type,template<GLenum > class BufferTypePolicy = BufferPolicy>
class GLBuffer : public GLObject {
protected:
    size_t width_ =0;
    size_t height_ =0;
    size_t element_size_ = 0;
    size_t bufferSize = 0;
public:
    //该函数并未分配GPU buffer
    GLBuffer(): GLObject(BufferTypePolicy<buf_type>::createBufferObject(),"GLBuffer"){}

    GLBuffer(size_t capacity)
        : GLObject(BufferTypePolicy<buf_type>::createBufferObject(),"GLBuffer"),
          bufferSize(capacity)
    {
        glNamedBufferStorage(this->handle,capacity, nullptr,GL_DYNAMIC_STORAGE_BIT);
    }

    GLBuffer(size_t width,size_t height,size_t element_size):
        GLObject(BufferTypePolicy<buf_type>::createBufferObject(),"GLBuffer"),
        width_(width),height_(height),element_size_(element_size)
    {
        bufferSize = width*height *element_size;
        glNamedBufferStorage(this->handle,width_ *  height_* element_size_, nullptr,GL_DYNAMIC_STORAGE_BIT);
    }

    size_t getBufferSize(){
        return  bufferSize ;
    }

    ~GLBuffer(){
       glDeleteBuffers(1, (reinterpret_cast<const GLuint *>(&(this->handle))));
    }

    void Bind(){
        BufferTypePolicy<buf_type>::Bind(this->handle);
    }

    void UnBind(){
      BufferTypePolicy<buf_type>::UnBind();
    }

  /**
     * @param data
     *    本地数据地址，用于更新缓存数据的数据源头,如果data 为NULL 那么指挥分配缓存，不会初始化
     * @param size
     *          数据块的大小字节为单位
     * @param flag
     *      缓存用途标识符
     *      GL_DYNAMIC_STORAGE_BIT:
     *      GL_MAP_READ_BIT: 将缓存映射到 CPU端读取数据，未设置调用glMapNamedBufferRange() 会失败
     *      GL_MAP_WRITE_BIT：将缓存映射到CPU端写入数据，未设置调用glMapNamedBufferRange() 会失败
     *      GL_MAP_PERSISTENT_BIT: 对缓存的数据映射是永久的，渲染过程一直有效
     *      GL_MAP_COHERENT_BIT: 缓存数据在GPU和CPU端保持一直
     *      GL_CLIENT_STORAGE_BIT:优先选择cpu端访问
     */
    void CopyToGPU(void* data, GLbitfield flag){
        glNamedBufferStorage(this->handle, width_ *height_ * element_size_ ,data,flag);
    }

    void CopyToGPU(void* data,size_t size ,GLbitfield flag){
        bufferSize = size;
        glNamedBufferStorage(this->handle,size ,data,flag);
    }
    /**
     * 使用新数据替换缓存中的数据,将缓存从offset 开始字节数据使用 data 开始的大小为size 的数据块更新GPU缓存
     * @param offset
     * @param size
     *      数据块的大小
     * @param data
     *      本地数据地址，用于更新缓存数据的数据源头
     */
    void CopyToGPUByPart(size_t offset,size_t size,const void* data){
        glNamedBufferSubData(this->handle,offset,size,data);
    }

    //清空缓存并填充数据，主要为了重复利用缓存
    void ClearGPUDataAndFill(GLenum internalFormat,GLenum format,const void* data){
        glClearNamedBufferData(this->handle,internalFormat,format,buf_type,data);
    }

    //清空部分缓存并填充数据，主要为了重复利用缓存
    void ClearGPUDataPartAndFill(size_t offset,size_t size,GLenum internalFormat,GLenum format,const void* data){
        glClearNamedBufferSubData(this->handle,internalFormat,offset,size,format,buf_type,data);
    }


    //将缓存数据拷贝到另一个缓存数中
    template<GLenum, template<GLenum> class BufferTypePolicy2>
    void CopyGPUDataToAnotherGPUBuffer(GLBuffer<buf_type,BufferTypePolicy2> AnotherBuffer,size_t read,size_t write,size_t copy_size){
        glCopyNamedBufferSubData(this->handle,AnotherBuffer.handle,read,write,copy_size);
    }

    //获取GPU端offset开始大小为size 的数据的数据
    void GetGPUData(size_t offset,size_t size ,void* data){
        glGetNamedBufferSubData(buf_type,offset,size,data);
    }

    //将当前缓存映射到CPU端
   //GL_READ_ONLY indicates that the returned pointer may be used to read buffer object data.
   //GL_WRITE_ONLY indicates that the returned pointer may be used to modify buffer object data.
   //GL_READ_WRITE indicates that the returned pointer may be used to read and to modify buffer object data.
    void* Map(GLenum access){
        return glMapNamedBuffer(this->handle,access);
    }

    //解除内存映射
    bool UnMap(){
        return  glUnmapNamedBuffer(this->handle);
    }

    //TODO
    void* MapBufferPart(size_t offset,size_t length,GLbitfield flags){
        return glMapNamedBufferRange(this->handle, offset,length,flags);
    }

    //缓存更新立即刷洗通知GPU
    //TODO
    void Flush(){
        glFlushMappedBufferRange(this->handle,0,width_* height_ *element_size_);
    }

};


#endif //GAMEENGINE_GLBUFFER_HPP
