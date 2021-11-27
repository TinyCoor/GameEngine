//
// Created by y123456 on 2021/10/18.
//

#ifndef GAMEENGINE_GLTEXTURE_H
#define GAMEENGINE_GLTEXTURE_H
#include "GLObject.h"
#include "Buffer/GLBuffer.hpp"
#include "TextureTraits.h"

//TODO TextureCubeMap
// GL_TEXTURE_1D,
// GL_TEXTURE_2D,
// GL_TEXTURE_3D,
// GL_TEXTURE_1D_ARRAY,
// GL_TEXTURE_2D_ARRAY,
// GL_TEXTURE_RECTANGLE,
// GL_TEXTURE_BUFFER,
// GL_TEXTURE_CUBE_MAP,
// GL_TEXTURE_CUBE_MAP_ARRAY,
// GL_TEXTURE_2D_MULTISAMPLE,
// GL_TEXTURE_2D_MULTISAMPLE_ARRAY

template<GLenum texture_type>
struct GLTextureCreatePolicy{
    static GLHANDLE Create()
    {
        GLHANDLE glhandle;
        glCreateTextures(texture_type,1,&glhandle);
        return glhandle;
    }

    static void Bind(int pos, GLHANDLE handle){
        glBindTextureUnit(pos,handle);
    }

    static void Destroy(GLHANDLE& handle){
        glDeleteTextures(1,&handle);
        handle = 0;
    }
};

template<GLenum texture_type>
struct SamplerCreatePolicy{
    static GLHANDLE create(){
        GLHANDLE sampler;
        glCreateSamplers(1,&sampler);
        return sampler;
    }

    static void bindSampler(int unit,GLHANDLE sampler){
        glBindSampler(unit,sampler);
    }

    static void destroy(GLHANDLE handle){
        glDeleteSamplers(1,&handle);
    }
};

template<GLenum texture_type, template<GLenum> class SamplerManagePolicy=SamplerCreatePolicy>
class GLSampler: GLObject {
public:
    GLSampler(): GLObject(SamplerCreatePolicy<texture_type>::create(),"Sampler"){}

    void init() {
        this->handle = SamplerCreatePolicy<texture_type>::create();
    }
    void BindSampler(int unit){
        SamplerManagePolicy<texture_type>::bindSampler(unit,this->handle);
    }

    void SetSamplerParamInt(GLHANDLE sampler,GLenum pname,const GLint param ){
        glSamplerParameteri(sampler,pname,param);
    }

    void SetSamplerParamfloat(GLHANDLE sampler,GLenum pname,const float param ){
        glSamplerParameterf(sampler,pname,param);
    }
};



// 内部有 texture buffer 对象
// 调用 Bind 默认绑定整个缓存对象
// Internal Format	Base Type	Count	Norm	Components
// GL_R8	        ubyte	      1 	YES	    R	0	0	1
// GL_R16	        ushort	      1 	YES	    R	0	0	1
// GL_R16F	        half	      1 	NO	    R	0	0	1
// GL_R32F	        float	      1 	NO	    R	0	0	1
// GL_R8I	        byte	      1 	NO	    R	0	0	1
// GL_R16I	        short	      1 	NO	    R	0	0	1
// GL_R32I	        int	          1 	NO	    R	0	0	1
// GL_R8UI	        ubyte	      1 	NO	    R	0	0	1
// GL_R16UI	        ushort	      1 	NO	    R	0	0	1
// GL_R32UI	        uint	      1 	NO	    R	0	0	1
// GL_RG8	        ubyte	      2 	YES	    R	G	0	1
// GL_RG16	        ushort	      2 	YES	    R	G	0	1
// GL_RG16F	        half	      2 	NO	    R	G	0	1
// GL_RG32F	        float	      2 	NO	    R	G	0	1
// GL_RG8I	        byte	      2 	NO	    R	G	0	1
// GL_RG16I	        short	      2 	NO	    R	G	0	1
// GL_RG32I	        int	          2 	NO	    R	G	0	1
// GL_RG8UI	        ubyte	      2 	NO	    R	G	0	1
// GL_RG16UI	    ushort	      2 	NO	    R	G	0	1
// GL_RG32UI	    uint	      2 	NO	    R	G	0	1
// GL_RGB32F	    float	      3 	NO	    R	G	B	1
// GL_RGB32I	    int	          3 	NO	    R	G	B	1
// GL_RGB32UI	    uint	      3 	NO	    R	G	B	1
// GL_RGBA8	        ubyte	      4 	YES	    R	G	B	A
// GL_RGBA16	    ushort	      4 	YES	    R	G	B	A
// GL_RGBA16F	    half	      4 	NO	    R	G	B	A
// GL_RGBA32F	    float	      4 	NO	    R	G	B	A
// GL_RGBA8I	    byte	      4 	NO	    R	G	B	A
// GL_RGBA16I	    short	      4 	NO	    R	G	B	A
// GL_RGBA32I	    int	          4 	NO	    R	G	B	A
// GL_RGBA8UI	    ubyte	      4 	NO	    R	G	B	A
// GL_RGBA16UI	    ushort	      4 	NO	    R	G	B	A
// GL_RGBA32UI	    uint	      4 	NO	    R	G	B	A

//
// TBO
//使用 TBO 需要创建
//layout(binding=0) uniform samplerBuffer buf
//使用 texelFetch访问文素
//color = texelFetch(buf,coord)

inline size_t internal_format_to_size(GLenum format){
    switch (format) {
        case GL_R8:         return sizeof(GLubyte) * 1;
        case GL_R16:        return sizeof(GLushort) * 1;
        case GL_R16F:       return sizeof(GLhalf)   * 1;
        case GL_R32F:       return sizeof(GLfloat)  * 1;
        case GL_R8I:        return sizeof(GLbyte)   * 1;
        case GL_R16I:       return sizeof(GLshort)  * 1;
        case GL_R32I:       return sizeof(GLint)    * 1;
        case GL_R8UI:       return sizeof(GLubyte)	* 1;
        case GL_R16UI:      return sizeof(GLushort) * 1;
        case GL_R32UI:      return sizeof(GLuint)   * 1;
        case GL_RG8:        return sizeof(GLubyte)  * 2;
        case GL_RG16:       return sizeof(GLushort) * 2;
        case GL_RG16F:      return sizeof(GLhalf)   * 2;
        case GL_RG32F:      return sizeof(GLfloat)  * 2;
        case GL_RG8I:       return sizeof(GLbyte)   * 2;
        case GL_RG16I:      return sizeof(GLshort)  * 2;
        case GL_RG32I:      return sizeof(GLint)    * 2;
        case GL_RG8UI:      return sizeof(GLubyte)	* 2;
        case GL_RG16UI:     return sizeof(GLushort) * 2;
        case GL_RG32UI:     return sizeof(GLuint)   * 2;
        case GL_RGB32F:     return sizeof(GLubyte)  * 3;
        case GL_RGB32I:     return sizeof(GLushort) * 3;
        case GL_RGB32UI:    return sizeof(GLhalf)   * 3;
        case GL_RGBA8:      return sizeof(GLfloat)  * 4;
        case GL_RGBA16:     return sizeof(GLbyte)   * 4;
        case GL_RGBA16F:    return sizeof(GLshort)  * 4;
        case GL_RGBA32F:    return sizeof(GLint)    * 4;
        case GL_RGBA8I:     return sizeof(GLubyte)  * 4;
        case GL_RGBA16I:    return sizeof(GLushort) * 4;
        case GL_RGBA32I:    return sizeof(GLhalf)   * 4;
        case GL_RGBA8UI:    return sizeof(GLfloat)  * 4;
        case GL_RGBA16UI:   return sizeof(GLbyte)   * 4;
        case GL_RGBA32UI:   return sizeof(GLshort)  * 4;
    }
    assert(false && "not open gl format");

    return 0;
}





/// 一个着色器程序会有多个texture_object
/// 对于一个纹理对象 需要绑定到对应的着色器单元 首先需要设置纹理对象的一些属性
/// 对于每一个纹理对象会有采样器，采样器需要绑定到对应的绑定点 可能会需要设置一些采样器的属性
/// eg layout(binding=1) uniform sampler2D texture_sampler;
///上述代码默认绑定到 纹理单元 1
///用代码实现的话 glBindSampler(1,sampler);

/// 分配texture_memory
/// glTexStorage*D 1/2/3
/// 填充数据
/// glTextureSubImage*D 1/2/3

template<GLenum textureType,
template<GLenum> class TextureCreatePolicy=GLTextureCreatePolicy>
class GLTexture: public GLObject{
private:
    int width;
    int height;
    int depth_;
    int mipLevel_;
    GLenum internal_format;
    GLSampler<textureType> sampler;
private:


public:
    static constexpr GLenum type = textureType;

    GLTexture(): GLObject(TextureCreatePolicy<textureType>::Create(),"Texture"){}

    GLTexture(int w,int h,int depth,int mipLevel,GLenum format)
        : GLObject(TextureCreatePolicy<textureType>::Create(),"Texture"),
          width(w),
          height(h),
          depth_(depth),
          mipLevel_(mipLevel),
          internal_format(format){}

    void init(int w,int h,int mip,GLenum f){
        width = w;
        height= h;
        mipLevel_= mip;
        internal_format = f;
    }

    int getWidth()const {return width;}
    int getHeight() const{ return height;}
    int getDepth() const {return depth_;}
    int getMipLevel() const { return mipLevel_;}
    int getInternalFormat() const { return internal_format;}
    ~GLTexture() {
        TextureCreatePolicy<textureType>::Destroy(this->handle);
    }

    void BindTexture(int pos){
        TextureCreatePolicy<textureType>::Bind(pos,this->handle);
        sampler.BindSampler(pos);
    }


    void SetTextureParam(GLenum pname,GLint param){
        glTexParameteri(textureType, pname, param);
    }

    // format 指定format of pixel data
    // GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA,
    // GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_BGR_INTEGER,
    // GL_RGBA_INTEGER, GL_BGRA_INTEGER, GL_STENCIL_INDEX,
    // GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL.
    void GenerateMipMap(){
        glGenerateMipmap(textureType);
    }
};


#define DEFINE_TEXTURE_FUNCTION_ALLOCATE_1D(name,type)   static void name(GLTexture<type> texture, \
          int mipLevel,GLenum internal_format ,int width,int height,int depth)      {               \
              glTexStorage1D(texture.GetHandle(),mipLevel,internal_format,width);                  \
          }

#define DEFINE_TEXTURE_FUNCTION_FILL_1D(name,type) static void name (GLTexture<type> texture, \
        int mipLevel,int xoffset,int yoffset,int zoffset,   \
        int width,int height,int depth,GLenum format,GLenum m_type,                          \
        const void* data) {                                                           \
            glTexSubImage1D(texture.GetHandle(),mipLevel,xoffset,width,format,m_type,data);                                           \
        }

#define DEFINE_TEXTURE_FUNCTION_ALLOCATE_2D(name,type)   static void name(GLTexture<type> texture, \
          int mipLevel,GLenum internal_format ,int width,int height,int depth)      {               \
               glTexStorage2D(texture.GetHandle(),mipLevel,internal_format,width,height);            \
          }

#define DEFINE_TEXTURE_FUNCTION_FILL_2D(name,type) static void name(GLTexture<type> texture,int mipLevel,int xoffset,int yoffset,int zoffset,   \
        int width,int height,int depth,GLenum format,GLenum m_type,const void* data){                                                           \
            glTexSubImage2D(texture.GetHandle(),mipLevel,xoffset,yoffset,width,height,format,m_type,data);                                           \
        }

#define DEFINE_TEXTURE_FUNCTION_ALLOCATE_3D(name,type)   static void name(GLTexture<type> texture, \
          int mipLevel,GLenum internal_format ,int width,int height,int depth)      {               \
               glTexStorage3D(texture.GetHandle(),mipLevel,internal_format,width,height,depth);      \
          }
#define DEFINE_TEXTURE_FUNCTION_FILL_3D(name,type) static void name(GLTexture<type> texture,int mipLevel,int xoffset,int yoffset,int zoffset,   \
        int width,int height,int depth,GLenum format,GLenum m_type,const void* data){                                                           \
            glTexSubImage3D(texture.GetHandle(),mipLevel,xoffset,yoffset,zoffset,width,height,height,format,m_type,data);                                           \
        }



//利用重载
// x显式设置文件
class TextureAllocator{
private:
    DEFINE_TEXTURE_FUNCTION_ALLOCATE_1D(AllocateTextureMemImpl,GL_TEXTURE_1D)
    DEFINE_TEXTURE_FUNCTION_FILL_1D(FillTextureMemImpl,GL_TEXTURE_1D)
    DEFINE_TEXTURE_FUNCTION_ALLOCATE_1D(AllocateTextureMemImpl,GL_TEXTURE_BUFFER)
    DEFINE_TEXTURE_FUNCTION_FILL_1D(FillTextureMemImpl,GL_TEXTURE_BUFFER)

    DEFINE_TEXTURE_FUNCTION_ALLOCATE_2D(AllocateTextureMemImpl,GL_TEXTURE_2D)
    DEFINE_TEXTURE_FUNCTION_FILL_2D(FillTextureMemImpl,GL_TEXTURE_2D)
    DEFINE_TEXTURE_FUNCTION_ALLOCATE_2D(AllocateTextureMemImpl,GL_TEXTURE_1D_ARRAY)
    DEFINE_TEXTURE_FUNCTION_FILL_2D(FillTextureMemImpl,GL_TEXTURE_1D_ARRAY)
    DEFINE_TEXTURE_FUNCTION_ALLOCATE_2D(AllocateTextureMemImpl,GL_TEXTURE_RECTANGLE)
    DEFINE_TEXTURE_FUNCTION_FILL_2D(FillTextureMemImpl,GL_TEXTURE_RECTANGLE)

    DEFINE_TEXTURE_FUNCTION_ALLOCATE_2D(AllocateTextureMemImpl,GL_TEXTURE_2D_MULTISAMPLE)
    DEFINE_TEXTURE_FUNCTION_FILL_2D(FillTextureMemImpl,GL_TEXTURE_2D_MULTISAMPLE)
    DEFINE_TEXTURE_FUNCTION_ALLOCATE_2D(AllocateTextureMemImpl,GL_TEXTURE_CUBE_MAP)
    DEFINE_TEXTURE_FUNCTION_FILL_2D(FillTextureMemImpl,GL_TEXTURE_CUBE_MAP)

    DEFINE_TEXTURE_FUNCTION_ALLOCATE_3D(AllocateTextureMemImpl,GL_TEXTURE_2D_ARRAY)
    DEFINE_TEXTURE_FUNCTION_FILL_3D(FillTextureMemImpl,GL_TEXTURE_2D_ARRAY)
    DEFINE_TEXTURE_FUNCTION_ALLOCATE_3D(AllocateTextureMemImpl,GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
    DEFINE_TEXTURE_FUNCTION_FILL_3D(FillTextureMemImpl,GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
    DEFINE_TEXTURE_FUNCTION_ALLOCATE_3D(AllocateTextureMemImpl,GL_TEXTURE_CUBE_MAP_ARRAY)
    DEFINE_TEXTURE_FUNCTION_FILL_3D(FillTextureMemImpl,GL_TEXTURE_CUBE_MAP_ARRAY)

public:
    template<GLenum type>
    static void Allocate(GLTexture<type> texture){
        AllocateTextureMemImpl(texture,texture.getMipLevel(),texture.getInternalFormat(),texture.getWidth(),texture.getHeight(),texture.getDepth());
    }

    template<GLenum type>
    void FillTextureMemory(GLTexture<type> texture,GLenum format,GLenum m_type,const void* data){
        FillTextureMemImpl(texture,0,0,0,texture.getWidth(),texture.getHeight(),texture.getDepth(),format,m_type,data);
    }
};

template<GLenum texture_type>
GLTexture<texture_type> createTexture(int width, int height,int mipLevel,GLenum format){
    GLTexture<texture_type> texture;
    texture.init(width,height,mipLevel,format);
    TextureAllocator::Allocate(texture);
    return texture;
}


template<GLenum texture_type>
GLSampler<texture_type> createSamler(){
    GLSampler<texture_type> sampler;
    return sampler;
}

#define GL_ARB_bindless_texture

class GLUnbindlessTexture :GLObject{
public:
    GLUnbindlessTexture(): GLObject(0,"UnbindlessTexture"){

    }
};

#endif //GAMEENGINE_GLTEXTURE_H
