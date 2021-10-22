#include <string>
#include <inttypes.h>

namespace
{
	GL4API apiHook;
} // namespace

using PFNGETGLPROC = void* (const char*);

#define E2S( en ) Enum2String( en ).c_str()
extern std::string Enum2String( GLenum e );
void GLTracer_glCullFace(GLenum mode);
void GLTracer_glPolygonMode(GLenum face, GLenum mode);
void GLTracer_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void GLTracer_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void GLTracer_glCullFace(GLenum mode);
void GLTracer_glPolygonMode(GLenum face, GLenum mode);
void GLTracer_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void GLTracer_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void GLTracer_glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params);
void GLTracer_glTexParameteri(GLenum target, GLenum pname, GLint param);
void GLTracer_glTexParameteriv(GLenum target, GLenum pname, const GLint* params);
void GLTracer_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
void GLTracer_glClear(GLbitfield mask);
void GLTracer_glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void GLTracer_glClearStencil(GLint s);
void GLTracer_glClearDepth(GLdouble depth);
void GLTracer_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void GLTracer_glDepthMask(GLboolean flag);
void GLTracer_glDisable(GLenum cap);
void GLTracer_glEnable(GLenum cap);
void GLTracer_glFinish();
void GLTracer_glFlush();
void GLTracer_glBlendFunc(GLenum sfactor, GLenum dfactor);
void GLTracer_glDepthFunc(GLenum func);
void GLTracer_glPixelStorei(GLenum pname, GLint param);
void GLTracer_glReadBuffer(GLenum src);
void GLTracer_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
GLenum GLTracer_glGetError();
void GLTracer_glGetIntegerv(GLenum pname, GLint* data);
const GLubyte* GLTracer_glGetString(GLenum name);
void GLTracer_glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
void GLTracer_glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params)
void GLTracer_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);


void GLTracer_glDrawArrays(GLenum mode, GLint first, GLsizei count)


void GLTracer_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)


void GLTracer_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)


void GLTracer_glDeleteTextures(GLsizei n, const GLuint* textures)


void GLTracer_glGenTextures(GLsizei n, GLuint* textures)


void GLTracer_glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)


void GLTracer_glActiveTexture(GLenum texture)


void GLTracer_glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)


void GLTracer_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)


void GLTracer_glGetCompressedTexImage(GLenum target, GLint level, void* img)


void GLTracer_glGenQueries(GLsizei n, GLuint* ids)


void GLTracer_glDeleteQueries(GLsizei n, const GLuint* ids)


void GLTracer_glBeginQuery(GLenum target, GLuint id)


void GLTracer_glEndQuery(GLenum target)
void GLTracer_glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params)
void GLTracer_glBindBuffer(GLenum target, GLuint buffer)
void GLTracer_glDeleteBuffers(GLsizei n, const GLuint* buffers)
void GLTracer_glGenBuffers(GLsizei n, GLuint* buffers)
void GLTracer_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
void GLTracer_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
void GLTracer_glDrawBuffers(GLsizei n, const GLenum* bufs)
void GLTracer_glAttachShader(GLuint program, GLuint shader)
void GLTracer_glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
void GLTracer_glCompileShader(GLuint shader)
GLuint GLTracer_glCreateProgram()
GLuint GLTracer_glCreateShader(GLenum type)
void GLTracer_glDeleteProgram(GLuint program)
void GLTracer_glDeleteShader(GLuint shader)
void GLTracer_glDisableVertexAttribArray(GLuint index)
void GLTracer_glEnableVertexAttribArray(GLuint index)
void GLTracer_glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
void GLTracer_glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
GLint GLTracer_glGetAttribLocation(GLuint program, const GLchar* name)
void GLTracer_glGetProgramiv(GLuint program, GLenum pname, GLint* params)
void GLTracer_glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
void GLTracer_glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
void GLTracer_glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
GLint GLTracer_glGetUniformLocation(GLuint program, const GLchar* name)
GLboolean GLTracer_glIsProgram(GLuint program)
GLboolean GLTracer_glIsShader(GLuint shader)
void GLTracer_glLinkProgram(GLuint program)
void GLTracer_glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
void GLTracer_glUseProgram(GLuint program)
void GLTracer_glUniform1f(GLint location, GLfloat v0)
void GLTracer_glUniform1i(GLint location, GLint v0)
void GLTracer_glUniform1fv(GLint location, GLsizei count, const GLfloat* value)
void GLTracer_glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
void GLTracer_glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
void GLTracer_glUniform1iv(GLint location, GLsizei count, const GLint* value)
void GLTracer_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
void GLTracer_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
void GLTracer_glValidateProgram(GLuint program)
void GLTracer_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
void GLTracer_glEnablei(GLenum target, GLuint index)


void GLTracer_glDisablei(GLenum target, GLuint index)


void GLTracer_glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)


void GLTracer_glBindBufferBase(GLenum target, GLuint index, GLuint buffer)


void GLTracer_glBindFragDataLocation(GLuint program, GLuint color, const GLchar* name)


const GLubyte* GLTracer_glGetStringi(GLenum name, GLuint index)


void GLTracer_glBindFramebuffer(GLenum target, GLuint framebuffer)


void GLTracer_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)


void GLTracer_glGenFramebuffers(GLsizei n, GLuint* framebuffers)


GLenum GLTracer_glCheckFramebufferStatus(GLenum target)


void GLTracer_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)


void GLTracer_glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)


void GLTracer_glGenerateMipmap(GLenum target)


void GLTracer_glBindVertexArray(GLuint array)

void GLTracer_glDeleteVertexArrays(GLsizei n, const GLuint* arrays)


void GLTracer_glGenVertexArrays(GLsizei n, GLuint* arrays)


void GLTracer_glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)


void GLTracer_glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)


void GLTracer_glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)


void GLTracer_glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)


void GLTracer_glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params)


GLint GLTracer_glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar* name)


GLuint GLTracer_glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar* name)


void GLTracer_glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values)


void GLTracer_glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, GLchar* name)


void GLTracer_glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint* indices)


void GLTracer_glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint* values)


void GLTracer_glPatchParameteri(GLenum pname, GLint value)


void GLTracer_glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary)


void GLTracer_glProgramBinary(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length)


void GLTracer_glProgramParameteri(GLuint program, GLenum pname, GLint value)


void GLTracer_glProgramUniform1i(GLuint program, GLint location, GLint v0)


void GLTracer_glProgramUniform1f(GLuint program, GLint location, GLfloat v0)

void GLTracer_glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint* value)


void GLTracer_glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)

void GLTracer_glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint* value)

void GLTracer_glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)


void GLTracer_glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint* value)


void GLTracer_glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)


void GLTracer_glBindTextures(GLuint first, GLsizei count, const GLuint* textures)


void GLTracer_glCreateTransformFeedbacks(GLsizei n, GLuint* ids)


void GLTracer_glTransformFeedbackBufferBase(GLuint xfb, GLuint index, GLuint buffer)
void GLTracer_glTransformFeedbackBufferRange(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizei size);
void GLTracer_glGetTransformFeedbackiv(GLuint xfb, GLenum pname, GLint* param);
void GLTracer_glGetTransformFeedbacki_v(GLuint xfb, GLenum pname, GLuint index, GLint* param);
void GLTracer_glGetTransformFeedbacki64_v(GLuint xfb, GLenum pname, GLuint index, GLint64* param);

void GLTracer_glCreateBuffers(GLsizei n, GLuint* buffers);
void GLTracer_glNamedBufferStorage(GLuint buffer, GLsizei size, const void* data, GLbitfield flags)

void GLTracer_glNamedBufferData(GLuint buffer, GLsizei size, const void* data, GLenum usage)


void GLTracer_glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, const void* data)


void GLTracer_glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizei size)


void GLTracer_glClearNamedBufferData(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data)


void GLTracer_glClearNamedBufferSubData(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizei size, GLenum format, GLenum type, const void* data)


void* GLTracer_glMapNamedBuffer(GLuint buffer, GLenum access)


void* GLTracer_glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access)


GLboolean GLTracer_glUnmapNamedBuffer(GLuint buffer)

void GLTracer_glFlushMappedNamedBufferRange(GLuint buffer, GLintptr offset, GLsizei length)


void GLTracer_glGetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint* params)


void GLTracer_glGetNamedBufferParameteri64v(GLuint buffer, GLenum pname, GLint64* params)


void GLTracer_glGetNamedBufferPointerv(GLuint buffer, GLenum pname, void** params)


void GLTracer_glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, void* data)


void GLTracer_glCreateFramebuffers(GLsizei n, GLuint* framebuffers)


void GLTracer_glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)


void GLTracer_glNamedFramebufferParameteri(GLuint framebuffer, GLenum pname, GLint param)


void GLTracer_glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)


void GLTracer_glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)


void GLTracer_glNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buf)

void GLTracer_glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum* bufs)


void GLTracer_glNamedFramebufferReadBuffer(GLuint framebuffer, GLenum src)


void GLTracer_glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments)


void GLTracer_glInvalidateNamedFramebufferSubData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
void GLTracer_glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint* value)
void GLTracer_glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint* value)
void GLTracer_glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat* value)
void GLTracer_glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, const GLfloat depth, GLint stencil)
void GLTracer_glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
GLenum GLTracer_glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target)
void GLTracer_glGetNamedFramebufferParameteriv(GLuint framebuffer, GLenum pname, GLint* param)
void GLTracer_glGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params)
void GLTracer_glCreateRenderbuffers(GLsizei n, GLuint* renderbuffers)
void GLTracer_glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)
void GLTracer_glNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
void GLTracer_glGetNamedRenderbufferParameteriv(GLuint renderbuffer, GLenum pname, GLint* params)
void GLTracer_glCreateTextures(GLenum target, GLsizei n, GLuint* textures)
void GLTracer_glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer)
void GLTracer_glTextureBufferRange(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizei size)
void GLTracer_glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)
void GLTracer_glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
void GLTracer_glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
void GLTracer_glTextureStorage2DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
void GLTracer_glTextureStorage3DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
void GLTracer_glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
void GLTracer_glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
void GLTracer_glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
void GLTracer_glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
void GLTracer_glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
void GLTracer_glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
void GLTracer_glCopyTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
void GLTracer_glCopyTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
void GLTracer_glCopyTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
void GLTracer_glTextureParameterf(GLuint texture, GLenum pname, GLfloat param)
void GLTracer_glTextureParameterfv(GLuint texture, GLenum pname, const GLfloat* param)
void GLTracer_glTextureParameteri(GLuint texture, GLenum pname, GLint param)
void GLTracer_glTextureParameterIiv(GLuint texture, GLenum pname, const GLint* params)
void GLTracer_glTextureParameterIuiv(GLuint texture, GLenum pname, const GLuint* params)
void GLTracer_glTextureParameteriv(GLuint texture, GLenum pname, const GLint* param)
void GLTracer_glGenerateTextureMipmap(GLuint texture)
void GLTracer_glBindTextureUnit(GLuint unit, GLuint texture)
void GLTracer_glGetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels)
void GLTracer_glGetCompressedTextureImage(GLuint texture, GLint level, GLsizei bufSize, void* pixels)
void GLTracer_glGetTextureLevelParameterfv(GLuint texture, GLint level, GLenum pname, GLfloat* params);
void GLTracer_glGetTextureLevelParameteriv(GLuint texture, GLint level, GLenum pname, GLint* params);
void GLTracer_glGetTextureParameterfv(GLuint texture, GLenum pname, GLfloat* params)
void GLTracer_glGetTextureParameterIiv(GLuint texture, GLenum pname, GLint* params)
void GLTracer_glGetTextureParameterIuiv(GLuint texture, GLenum pname, GLuint* params)
void GLTracer_glGetTextureParameteriv(GLuint texture, GLenum pname, GLint* params)
void GLTracer_glCreateVertexArrays(GLsizei n, GLuint* arrays)
void GLTracer_glDisableVertexArrayAttrib(GLuint vaobj, GLuint index);
void GLTracer_glEnableVertexArrayAttrib(GLuint vaobj, GLuint index)
void GLTracer_glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer)
void GLTracer_glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
void GLTracer_glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides);
void GLTracer_glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
void GLTracer_glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
void GLTracer_glVertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
void GLTracer_glVertexArrayAttribLFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
void GLTracer_glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor);
void GLTracer_glGetVertexArrayiv(GLuint vaobj, GLenum pname, GLint* param);
void GLTracer_glGetVertexArrayIndexediv(GLuint vaobj, GLuint index, GLenum pname, GLint* param);
void GLTracer_glGetVertexArrayIndexed64iv(GLuint vaobj, GLuint index, GLenum pname, GLint64* param)
void GLTracer_glCreateSamplers(GLsizei n, GLuint* samplers);
void GLTracer_glCreateProgramPipelines(GLsizei n, GLuint* pipelines)
void GLTracer_glCreateQueries(GLenum target, GLsizei n, GLuint* ids)

#define INJECT(S) api->S = &GLTracer_##S;

void InjectAPITracer4(GL4API* api)
{
	apiHook = *api;
	INJECT(glActiveTexture);
	INJECT(glAttachShader);
	INJECT(glBeginQuery);
	INJECT(glBindAttribLocation);
	INJECT(glBindBuffer);
	INJECT(glBindBufferBase);
	INJECT(glBindBufferRange);
	INJECT(glBindFragDataLocation);
	INJECT(glBindFramebuffer);
	INJECT(glBindTextureUnit);
	INJECT(glBindTextures);
	INJECT(glBindVertexArray);
	INJECT(glBlendFunc);
	INJECT(glBlitNamedFramebuffer);
	INJECT(glBufferData);
	INJECT(glBufferSubData);
	INJECT(glCheckFramebufferStatus);
	INJECT(glCheckNamedFramebufferStatus);
	INJECT(glClear);
	INJECT(glClearColor);
	INJECT(glClearDepth);
	INJECT(glClearNamedBufferData);
	INJECT(glClearNamedBufferSubData);
	INJECT(glClearNamedFramebufferfi);
	INJECT(glClearNamedFramebufferfv);
	INJECT(glClearNamedFramebufferiv);
	INJECT(glClearNamedFramebufferuiv);
	INJECT(glClearStencil);
	INJECT(glColorMask);
	INJECT(glCompileShader);
	INJECT(glCompressedTexImage2D);
	INJECT(glCompressedTexImage3D);
	INJECT(glCompressedTextureSubImage1D);
	INJECT(glCompressedTextureSubImage2D);
	INJECT(glCompressedTextureSubImage3D);
	INJECT(glCopyNamedBufferSubData);
	INJECT(glCopyTextureSubImage1D);
	INJECT(glCopyTextureSubImage2D);
	INJECT(glCopyTextureSubImage3D);
	INJECT(glCreateBuffers);
	INJECT(glCreateFramebuffers);
	INJECT(glCreateProgram);
	INJECT(glCreateProgramPipelines);
	INJECT(glCreateQueries);
	INJECT(glCreateRenderbuffers);
	INJECT(glCreateSamplers);
	INJECT(glCreateShader);
	INJECT(glCreateTextures);
	INJECT(glCreateTransformFeedbacks);
	INJECT(glCreateVertexArrays);
	INJECT(glCullFace);
	INJECT(glDeleteBuffers);
	INJECT(glDeleteFramebuffers);
	INJECT(glDeleteProgram);
	INJECT(glDeleteQueries);
	INJECT(glDeleteShader);
	INJECT(glDeleteTextures);
	INJECT(glDeleteVertexArrays);
	INJECT(glDepthFunc);
	INJECT(glDepthMask);
	INJECT(glDisable);
	INJECT(glDisableVertexArrayAttrib);
	INJECT(glDisableVertexAttribArray);
	INJECT(glDisablei);
	INJECT(glDrawArrays);
	INJECT(glDrawArraysInstanced);
	INJECT(glDrawBuffers);
	INJECT(glDrawElements);
	INJECT(glDrawElementsInstanced);
	INJECT(glEnable);
	INJECT(glEnableVertexArrayAttrib);
	INJECT(glEnableVertexAttribArray);
	INJECT(glEnablei);
	INJECT(glEndQuery);
	INJECT(glFinish);
	INJECT(glFlush);
	INJECT(glFlushMappedNamedBufferRange);
	INJECT(glFramebufferTexture2D);
	INJECT(glFramebufferTexture3D);
	INJECT(glGenBuffers);
	INJECT(glGenFramebuffers);
	INJECT(glGenQueries);
	INJECT(glGenTextures);
	INJECT(glGenVertexArrays);
	INJECT(glGenerateMipmap);
	INJECT(glGenerateTextureMipmap);
	INJECT(glGetActiveAttrib);
	INJECT(glGetActiveSubroutineName);
	INJECT(glGetActiveSubroutineUniformiv);
	INJECT(glGetActiveUniform);
	INJECT(glGetActiveUniformBlockiv);
	INJECT(glGetAttribLocation);
	INJECT(glGetCompressedTexImage);
	INJECT(glGetCompressedTextureImage);
	INJECT(glGetIntegerv);
	INJECT(glGetNamedBufferParameteri64v);
	INJECT(glGetNamedBufferParameteriv);
	INJECT(glGetNamedBufferPointerv);
	INJECT(glGetNamedBufferSubData);
	INJECT(glGetNamedFramebufferAttachmentParameteriv);
	INJECT(glGetNamedFramebufferParameteriv);
	INJECT(glGetNamedRenderbufferParameteriv);
	INJECT(glGetProgramBinary);
	INJECT(glGetProgramInfoLog);
	INJECT(glGetProgramStageiv);
	INJECT(glGetProgramiv);
	INJECT(glGetQueryObjectiv);
	INJECT(glGetQueryObjectui64v);
	INJECT(glGetShaderInfoLog);
	INJECT(glGetShaderiv);
	INJECT(glGetSubroutineIndex);
	INJECT(glGetSubroutineUniformLocation);
	INJECT(glGetTexImage);
	INJECT(glGetTexLevelParameteriv);
	INJECT(glGetTextureImage);
	INJECT(glGetTextureLevelParameterfv);
	INJECT(glGetTextureLevelParameteriv);
	INJECT(glGetTextureParameterIiv);
	INJECT(glGetTextureParameterIuiv);
	INJECT(glGetTextureParameterfv);
	INJECT(glGetTextureParameteriv);
	INJECT(glGetTransformFeedbacki64_v);
	INJECT(glGetTransformFeedbacki_v);
	INJECT(glGetTransformFeedbackiv);
	INJECT(glGetUniformLocation);
	INJECT(glGetVertexArrayIndexed64iv);
	INJECT(glGetVertexArrayIndexediv);
	INJECT(glGetVertexArrayiv);
	INJECT(glInvalidateNamedFramebufferData);
	INJECT(glInvalidateNamedFramebufferSubData);
	INJECT(glIsProgram);
	INJECT(glIsShader);
	INJECT(glLinkProgram);
	INJECT(glMapNamedBuffer);
	INJECT(glMapNamedBufferRange);
	INJECT(glNamedBufferData);
	INJECT(glNamedBufferStorage);
	INJECT(glNamedBufferSubData);
	INJECT(glNamedFramebufferDrawBuffer);
	INJECT(glNamedFramebufferDrawBuffers);
	INJECT(glNamedFramebufferParameteri);
	INJECT(glNamedFramebufferReadBuffer);
	INJECT(glNamedFramebufferRenderbuffer);
	INJECT(glNamedFramebufferTexture);
	INJECT(glNamedFramebufferTextureLayer);
	INJECT(glNamedRenderbufferStorage);
	INJECT(glNamedRenderbufferStorageMultisample);
	INJECT(glPatchParameteri);
	INJECT(glPixelStorei);
	INJECT(glPolygonMode);
	INJECT(glProgramBinary);
	INJECT(glProgramParameteri);
	INJECT(glProgramUniform1f);
	INJECT(glProgramUniform1i);
	INJECT(glProgramUniform2fv);
	INJECT(glProgramUniform2iv);
	INJECT(glProgramUniform3fv);
	INJECT(glProgramUniform3iv);
	INJECT(glProgramUniform4fv);
	INJECT(glProgramUniform4iv);
	INJECT(glReadBuffer);
	INJECT(glReadPixels);
	INJECT(glScissor);
	INJECT(glShaderSource);
	INJECT(glTexImage2D);
	INJECT(glTexImage3D);
	INJECT(glTexParameterf);
	INJECT(glTexParameterfv);
	INJECT(glTexParameteri);
	INJECT(glTexParameteriv);
	INJECT(glTexSubImage2D);
	INJECT(glTextureBuffer);
	INJECT(glTextureBufferRange);
	INJECT(glTextureParameterIiv);
	INJECT(glTextureParameterIuiv);
	INJECT(glTextureParameterf);
	INJECT(glTextureParameterfv);
	INJECT(glTextureParameteri);
	INJECT(glTextureParameteriv);
	INJECT(glTextureStorage1D);
	INJECT(glTextureStorage2D);
	INJECT(glTextureStorage2DMultisample);
	INJECT(glTextureStorage3D);
	INJECT(glTextureStorage3DMultisample);
	INJECT(glTextureSubImage1D);
	INJECT(glTextureSubImage2D);
	INJECT(glTextureSubImage3D);
	INJECT(glTransformFeedbackBufferBase);
	INJECT(glTransformFeedbackBufferRange);
	INJECT(glUniform1f);
	INJECT(glUniform1fv);
	INJECT(glUniform1i);
	INJECT(glUniform1iv);
	INJECT(glUniform3fv);
	INJECT(glUniform4fv);
	INJECT(glUniformBlockBinding);
	INJECT(glUniformMatrix3fv);
	INJECT(glUniformMatrix4fv);
	INJECT(glUniformSubroutinesuiv);
	INJECT(glUnmapNamedBuffer);
	INJECT(glUseProgram);
	INJECT(glValidateProgram);
	INJECT(glVertexArrayAttribBinding);
	INJECT(glVertexArrayAttribFormat);
	INJECT(glVertexArrayAttribIFormat);
	INJECT(glVertexArrayAttribLFormat);
	INJECT(glVertexArrayBindingDivisor);
	INJECT(glVertexArrayElementBuffer);
	INJECT(glVertexArrayVertexBuffer);
	INJECT(glVertexArrayVertexBuffers);
	INJECT(glVertexAttribPointer);
	INJECT(glViewport);
}

#define LOAD_GL_FUNC(func) api->func = ( decltype(api->func) )GetGLProc(#func);

void GetAPI4(GL4API* api, PFNGETGLPROC GetGLProc)
{
	LOAD_GL_FUNC(glActiveTexture);
	LOAD_GL_FUNC(glAttachShader);
	LOAD_GL_FUNC(glBeginQuery);
	LOAD_GL_FUNC(glBindAttribLocation);
	LOAD_GL_FUNC(glBindBuffer);
	LOAD_GL_FUNC(glBindBufferBase);
	LOAD_GL_FUNC(glBindBufferRange);
	LOAD_GL_FUNC(glBindFragDataLocation);
	LOAD_GL_FUNC(glBindFramebuffer);
	LOAD_GL_FUNC(glBindTextureUnit);
	LOAD_GL_FUNC(glBindTextures);
	LOAD_GL_FUNC(glBindVertexArray);
	LOAD_GL_FUNC(glBlendFunc);
	LOAD_GL_FUNC(glBlitNamedFramebuffer);
	LOAD_GL_FUNC(glBufferData);
	LOAD_GL_FUNC(glBufferSubData);
	LOAD_GL_FUNC(glCheckFramebufferStatus);
	LOAD_GL_FUNC(glCheckNamedFramebufferStatus);
	LOAD_GL_FUNC(glClear);
	LOAD_GL_FUNC(glClearColor);
	LOAD_GL_FUNC(glClearDepth);
	LOAD_GL_FUNC(glClearNamedBufferData);
	LOAD_GL_FUNC(glClearNamedBufferSubData);
	LOAD_GL_FUNC(glClearNamedFramebufferfi);
	LOAD_GL_FUNC(glClearNamedFramebufferfv);
	LOAD_GL_FUNC(glClearNamedFramebufferiv);
	LOAD_GL_FUNC(glClearNamedFramebufferuiv);
	LOAD_GL_FUNC(glClearStencil);
	LOAD_GL_FUNC(glColorMask);
	LOAD_GL_FUNC(glCompileShader);
	LOAD_GL_FUNC(glCompressedTexImage2D);
	LOAD_GL_FUNC(glCompressedTexImage3D);
	LOAD_GL_FUNC(glCompressedTextureSubImage1D);
	LOAD_GL_FUNC(glCompressedTextureSubImage2D);
	LOAD_GL_FUNC(glCompressedTextureSubImage3D);
	LOAD_GL_FUNC(glCopyNamedBufferSubData);
	LOAD_GL_FUNC(glCopyTextureSubImage1D);
	LOAD_GL_FUNC(glCopyTextureSubImage2D);
	LOAD_GL_FUNC(glCopyTextureSubImage3D);
	LOAD_GL_FUNC(glCreateBuffers);
	LOAD_GL_FUNC(glCreateFramebuffers);
	LOAD_GL_FUNC(glCreateProgram);
	LOAD_GL_FUNC(glCreateProgramPipelines);
	LOAD_GL_FUNC(glCreateQueries);
	LOAD_GL_FUNC(glCreateRenderbuffers);
	LOAD_GL_FUNC(glCreateSamplers);
	LOAD_GL_FUNC(glCreateShader);
	LOAD_GL_FUNC(glCreateTextures);
	LOAD_GL_FUNC(glCreateTransformFeedbacks);
	LOAD_GL_FUNC(glCreateVertexArrays);
	LOAD_GL_FUNC(glCullFace);
	LOAD_GL_FUNC(glDeleteBuffers);
	LOAD_GL_FUNC(glDeleteFramebuffers);
	LOAD_GL_FUNC(glDeleteProgram);
	LOAD_GL_FUNC(glDeleteQueries);
	LOAD_GL_FUNC(glDeleteShader);
	LOAD_GL_FUNC(glDeleteTextures);
	LOAD_GL_FUNC(glDeleteVertexArrays);
	LOAD_GL_FUNC(glDepthFunc);
	LOAD_GL_FUNC(glDepthMask);
	LOAD_GL_FUNC(glDisable);
	LOAD_GL_FUNC(glDisableVertexArrayAttrib);
	LOAD_GL_FUNC(glDisableVertexAttribArray);
	LOAD_GL_FUNC(glDisablei);
	LOAD_GL_FUNC(glDrawArrays);
	LOAD_GL_FUNC(glDrawArraysInstanced);
	LOAD_GL_FUNC(glDrawBuffers);
	LOAD_GL_FUNC(glDrawElements);
	LOAD_GL_FUNC(glDrawElementsInstanced);
	LOAD_GL_FUNC(glEnable);
	LOAD_GL_FUNC(glEnableVertexArrayAttrib);
	LOAD_GL_FUNC(glEnableVertexAttribArray);
	LOAD_GL_FUNC(glEnablei);
	LOAD_GL_FUNC(glEndQuery);
	LOAD_GL_FUNC(glFinish);
	LOAD_GL_FUNC(glFlush);
	LOAD_GL_FUNC(glFlushMappedNamedBufferRange);
	LOAD_GL_FUNC(glFramebufferTexture2D);
	LOAD_GL_FUNC(glFramebufferTexture3D);
	LOAD_GL_FUNC(glGenBuffers);
	LOAD_GL_FUNC(glGenFramebuffers);
	LOAD_GL_FUNC(glGenQueries);
	LOAD_GL_FUNC(glGenTextures);
	LOAD_GL_FUNC(glGenVertexArrays);
	LOAD_GL_FUNC(glGenerateMipmap);
	LOAD_GL_FUNC(glGenerateTextureMipmap);
	LOAD_GL_FUNC(glGetActiveAttrib);
	LOAD_GL_FUNC(glGetActiveSubroutineName);
	LOAD_GL_FUNC(glGetActiveSubroutineUniformiv);
	LOAD_GL_FUNC(glGetActiveUniform);
	LOAD_GL_FUNC(glGetActiveUniformBlockiv);
	LOAD_GL_FUNC(glGetAttribLocation);
	LOAD_GL_FUNC(glGetCompressedTexImage);
	LOAD_GL_FUNC(glGetCompressedTextureImage);
	LOAD_GL_FUNC(glGetError);
	LOAD_GL_FUNC(glGetIntegerv);
	LOAD_GL_FUNC(glGetNamedBufferParameteri64v);
	LOAD_GL_FUNC(glGetNamedBufferParameteriv);
	LOAD_GL_FUNC(glGetNamedBufferPointerv);
	LOAD_GL_FUNC(glGetNamedBufferSubData);
	LOAD_GL_FUNC(glGetNamedFramebufferAttachmentParameteriv);
	LOAD_GL_FUNC(glGetNamedFramebufferParameteriv);
	LOAD_GL_FUNC(glGetNamedRenderbufferParameteriv);
	LOAD_GL_FUNC(glGetProgramBinary);
	LOAD_GL_FUNC(glGetProgramInfoLog);
	LOAD_GL_FUNC(glGetProgramStageiv);
	LOAD_GL_FUNC(glGetProgramiv);
	LOAD_GL_FUNC(glGetQueryObjectiv);
	LOAD_GL_FUNC(glGetQueryObjectui64v);
	LOAD_GL_FUNC(glGetShaderInfoLog);
	LOAD_GL_FUNC(glGetShaderiv);
	LOAD_GL_FUNC(glGetSubroutineIndex);
	LOAD_GL_FUNC(glGetSubroutineUniformLocation);
	LOAD_GL_FUNC(glGetTexImage);
	LOAD_GL_FUNC(glGetTexLevelParameteriv);
	LOAD_GL_FUNC(glGetTextureImage);
	LOAD_GL_FUNC(glGetTextureLevelParameterfv);
	LOAD_GL_FUNC(glGetTextureLevelParameteriv);
	LOAD_GL_FUNC(glGetTextureParameterIiv);
	LOAD_GL_FUNC(glGetTextureParameterIuiv);
	LOAD_GL_FUNC(glGetTextureParameterfv);
	LOAD_GL_FUNC(glGetTextureParameteriv);
	LOAD_GL_FUNC(glGetTransformFeedbacki64_v);
	LOAD_GL_FUNC(glGetTransformFeedbacki_v);
	LOAD_GL_FUNC(glGetTransformFeedbackiv);
	LOAD_GL_FUNC(glGetUniformLocation);
	LOAD_GL_FUNC(glGetVertexArrayIndexed64iv);
	LOAD_GL_FUNC(glGetVertexArrayIndexediv);
	LOAD_GL_FUNC(glGetVertexArrayiv);
	LOAD_GL_FUNC(glInvalidateNamedFramebufferData);
	LOAD_GL_FUNC(glInvalidateNamedFramebufferSubData);
	LOAD_GL_FUNC(glIsProgram);
	LOAD_GL_FUNC(glIsShader);
	LOAD_GL_FUNC(glLinkProgram);
	LOAD_GL_FUNC(glMapNamedBuffer);
	LOAD_GL_FUNC(glMapNamedBufferRange);
	LOAD_GL_FUNC(glNamedBufferData);
	LOAD_GL_FUNC(glNamedBufferStorage);
	LOAD_GL_FUNC(glNamedBufferSubData);
	LOAD_GL_FUNC(glNamedFramebufferDrawBuffer);
	LOAD_GL_FUNC(glNamedFramebufferDrawBuffers);
	LOAD_GL_FUNC(glNamedFramebufferParameteri);
	LOAD_GL_FUNC(glNamedFramebufferReadBuffer);
	LOAD_GL_FUNC(glNamedFramebufferRenderbuffer);
	LOAD_GL_FUNC(glNamedFramebufferTexture);
	LOAD_GL_FUNC(glNamedFramebufferTextureLayer);
	LOAD_GL_FUNC(glNamedRenderbufferStorage);
	LOAD_GL_FUNC(glNamedRenderbufferStorageMultisample);
	LOAD_GL_FUNC(glPatchParameteri);
	LOAD_GL_FUNC(glPixelStorei);
	LOAD_GL_FUNC(glPolygonMode);
	LOAD_GL_FUNC(glProgramBinary);
	LOAD_GL_FUNC(glProgramParameteri);
	LOAD_GL_FUNC(glProgramUniform1f);
	LOAD_GL_FUNC(glProgramUniform1i);
	LOAD_GL_FUNC(glProgramUniform2fv);
	LOAD_GL_FUNC(glProgramUniform2iv);
	LOAD_GL_FUNC(glProgramUniform3fv);
	LOAD_GL_FUNC(glProgramUniform3iv);
	LOAD_GL_FUNC(glProgramUniform4fv);
	LOAD_GL_FUNC(glProgramUniform4iv);
	LOAD_GL_FUNC(glReadBuffer);
	LOAD_GL_FUNC(glReadPixels);
	LOAD_GL_FUNC(glScissor);
	LOAD_GL_FUNC(glShaderSource);
	LOAD_GL_FUNC(glTexImage2D);
	LOAD_GL_FUNC(glTexImage3D);
	LOAD_GL_FUNC(glTexParameterf);
	LOAD_GL_FUNC(glTexParameterfv);
	LOAD_GL_FUNC(glTexParameteri);
	LOAD_GL_FUNC(glTexParameteriv);
	LOAD_GL_FUNC(glTexSubImage2D);
	LOAD_GL_FUNC(glTextureBuffer);
	LOAD_GL_FUNC(glTextureBufferRange);
	LOAD_GL_FUNC(glTextureParameterIiv);
	LOAD_GL_FUNC(glTextureParameterIuiv);
	LOAD_GL_FUNC(glTextureParameterf);
	LOAD_GL_FUNC(glTextureParameterfv);
	LOAD_GL_FUNC(glTextureParameteri);
	LOAD_GL_FUNC(glTextureParameteriv);
	LOAD_GL_FUNC(glTextureStorage1D);
	LOAD_GL_FUNC(glTextureStorage2D);
	LOAD_GL_FUNC(glTextureStorage2DMultisample);
	LOAD_GL_FUNC(glTextureStorage3D);
	LOAD_GL_FUNC(glTextureStorage3DMultisample);
	LOAD_GL_FUNC(glTextureSubImage1D);
	LOAD_GL_FUNC(glTextureSubImage2D);
	LOAD_GL_FUNC(glTextureSubImage3D);
	LOAD_GL_FUNC(glTransformFeedbackBufferBase);
	LOAD_GL_FUNC(glTransformFeedbackBufferRange);
	LOAD_GL_FUNC(glUniform1f);
	LOAD_GL_FUNC(glUniform1fv);
	LOAD_GL_FUNC(glUniform1i);
	LOAD_GL_FUNC(glUniform1iv);
	LOAD_GL_FUNC(glUniform3fv);
	LOAD_GL_FUNC(glUniform4fv);
	LOAD_GL_FUNC(glUniformBlockBinding);
	LOAD_GL_FUNC(glUniformMatrix3fv);
	LOAD_GL_FUNC(glUniformMatrix4fv);
	LOAD_GL_FUNC(glUniformSubroutinesuiv);
	LOAD_GL_FUNC(glUnmapNamedBuffer);
	LOAD_GL_FUNC(glUseProgram);
	LOAD_GL_FUNC(glValidateProgram);
	LOAD_GL_FUNC(glVertexArrayAttribBinding);
	LOAD_GL_FUNC(glVertexArrayAttribFormat);
	LOAD_GL_FUNC(glVertexArrayAttribIFormat);
	LOAD_GL_FUNC(glVertexArrayAttribLFormat);
	LOAD_GL_FUNC(glVertexArrayBindingDivisor);
	LOAD_GL_FUNC(glVertexArrayElementBuffer);
	LOAD_GL_FUNC(glVertexArrayVertexBuffer);
	LOAD_GL_FUNC(glVertexArrayVertexBuffers);
	LOAD_GL_FUNC(glVertexAttribPointer);
	LOAD_GL_FUNC(glViewport);
}
#endif
