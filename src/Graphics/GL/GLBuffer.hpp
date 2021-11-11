//
// Created by y123456 on 2021/10/28.
//

#ifndef GAMEENGINE_GLBUFFER_HPP
#define GAMEENGINE_GLBUFFER_HPP
#include "glad.h"
#include "GLObject.h"

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

    static void DestroyBuffer(GLHANDLE handle){
        glDeleteRenderbuffers(1,&handle);
    }
};


template<GLenum buf_type,template<GLenum > class BufferTypePolicy = BufferPolicy>
class GLBuffer : public GLObject {
public:

    GLBuffer(): GLObject(BufferTypePolicy<buf_type>::createBufferObject(),"GLBuffer"){

    }

    //TODO check
    /**
     * 对于从ShaderProgram找出的Handle 不需要调用Create
     * glGetUniformLoaction() 返回的 handle
     * @param handle
     */
    GLBuffer(GLHANDLE handle): GLObject(handle,"Uniform"){}

    ~GLBuffer(){

    }

    void Bind(){
        BufferTypePolicy<buf_type>::Bind(this->handle);
    }

    /**
     *
     * @param data
     *    本地数据地址，用于更新缓存数据的数据源头,如果data 为NULL 那么指挥分配缓存，不会初始化
     * @param size
     *          数据块的大小字节为单位
     * @param flag
     *      缓存用途标识符
     *      GL_DYNAMIC_STORAGE_BIT:
     *      GL_MAP_READ_BIT: 将缓存映射到CPU端读取数据，未设置调用glMapNamedBufferRange() 会失败
     *      GL_MAP_WRITE_BIT：将缓存映射到CPU端写入数据，未设置调用glMapNamedBufferRange() 会失败
     *      GL_MAP_PERSISTENT_BIT: 对缓存的数据映射是永久的，渲染过程一直有效
     *      GL_MAP_COHERENT_BIT: 缓存数据在GPU和CPU端保持一直
     *      GL_CLIENT_STORAGE_BIT:优先选择cpu端访问
     */
    void CopyToGPU(uint8_t* data,uint32_t size, GLbitfield flag){
        glNamedBufferStorage(this->handle,size,data,flag);
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
    void*  Map(GLenum access){
       return glMapBuffer(this->handle,access);
    }

    //解除内存映射
    bool UnMap(){
      return  glUnmapNamedBuffer(this->handle);
    }

    void* MapBufferPart(size_t offset,size_t length,GLbitfield flags){
        return glMapNamedBufferRange(this->handle, offset,length,flags);
    }
    //缓存更新立即刷洗通知GPU
    void Flush(int offset,size_t length){
        glFlushMappedBufferRange(this->handle,offset,length);
    }

};


#endif //GAMEENGINE_GLBUFFER_HPP
