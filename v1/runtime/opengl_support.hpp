#pragma once
#define GLFW_INCLUDE_NONE
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <ShObjIdl_core.h>
#include <propkey.h>
#include <propvarutil.h>
#endif
#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#include <GL/gl.h>
#include "hg/presentation.hpp"
#include <array>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <vector>

// Windows' legacy GL header deliberately stops at OpenGL 1.1.  Keep the small
// 3.3 loader local rather than depending on a loader hidden inside GLFW.
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
using PFNGLCREATESHADERPROC=GLuint(APIENTRYP)(GLenum);
using PFNGLSHADERSOURCEPROC=void(APIENTRYP)(GLuint,GLsizei,const char* const*,const GLint*);
using PFNGLCOMPILESHADERPROC=void(APIENTRYP)(GLuint);
using PFNGLGETSHADERIVPROC=void(APIENTRYP)(GLuint,GLenum,GLint*);
using PFNGLGETSHADERINFOLOGPROC=void(APIENTRYP)(GLuint,GLsizei,GLsizei*,char*);
using PFNGLDELETESHADERPROC=void(APIENTRYP)(GLuint);
using PFNGLCREATEPROGRAMPROC=GLuint(APIENTRYP)();
using PFNGLATTACHSHADERPROC=void(APIENTRYP)(GLuint,GLuint);
using PFNGLLINKPROGRAMPROC=void(APIENTRYP)(GLuint);
using PFNGLGETPROGRAMIVPROC=void(APIENTRYP)(GLuint,GLenum,GLint*);
using PFNGLGETPROGRAMINFOLOGPROC=void(APIENTRYP)(GLuint,GLsizei,GLsizei*,char*);
using PFNGLDELETEPROGRAMPROC=void(APIENTRYP)(GLuint);
using PFNGLUSEPROGRAMPROC=void(APIENTRYP)(GLuint);
using PFNGLGETUNIFORMLOCATIONPROC=GLint(APIENTRYP)(GLuint,const char*);
using PFNGLUNIFORM2FPROC=void(APIENTRYP)(GLint,GLfloat,GLfloat);
using PFNGLGENVERTEXARRAYSPROC=void(APIENTRYP)(GLsizei,GLuint*);
using PFNGLBINDVERTEXARRAYPROC=void(APIENTRYP)(GLuint);
using PFNGLDELETEVERTEXARRAYSPROC=void(APIENTRYP)(GLsizei,const GLuint*);
using PFNGLGENBUFFERSPROC=void(APIENTRYP)(GLsizei,GLuint*);
using PFNGLBINDBUFFERPROC=void(APIENTRYP)(GLenum,GLuint);
using PFNGLBUFFERDATAPROC=void(APIENTRYP)(GLenum,std::ptrdiff_t,const void*,GLenum);
using PFNGLDELETEBUFFERSPROC=void(APIENTRYP)(GLsizei,const GLuint*);
using PFNGLENABLEVERTEXATTRIBARRAYPROC=void(APIENTRYP)(GLuint);
using PFNGLVERTEXATTRIBPOINTERPROC=void(APIENTRYP)(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*);
using PFNGLDRAWARRAYSPROC=void(APIENTRYP)(GLenum,GLint,GLsizei);

constexpr GLenum GL_ARRAY_BUFFER_=0x8892,GL_STATIC_DRAW_=0x88e4;
constexpr GLenum GL_VERTEX_SHADER_=0x8b31,GL_FRAGMENT_SHADER_=0x8b30,GL_COMPILE_STATUS_=0x8b81,GL_LINK_STATUS_=0x8b82;

template<class T> T gl_proc(const char* name) {
    const auto proc=glfwGetProcAddress(name);
    if(!proc)throw std::runtime_error(std::string("missing OpenGL entry point: ")+name);
    return reinterpret_cast<T>(proc);
}

struct ProbeGl {
    PFNGLCREATESHADERPROC create_shader=gl_proc<PFNGLCREATESHADERPROC>("glCreateShader");
    PFNGLSHADERSOURCEPROC shader_source=gl_proc<PFNGLSHADERSOURCEPROC>("glShaderSource");
    PFNGLCOMPILESHADERPROC compile_shader=gl_proc<PFNGLCOMPILESHADERPROC>("glCompileShader");
    PFNGLGETSHADERIVPROC get_shader_iv=gl_proc<PFNGLGETSHADERIVPROC>("glGetShaderiv");
    PFNGLGETSHADERINFOLOGPROC get_shader_log=gl_proc<PFNGLGETSHADERINFOLOGPROC>("glGetShaderInfoLog");
    PFNGLDELETESHADERPROC delete_shader=gl_proc<PFNGLDELETESHADERPROC>("glDeleteShader");
    PFNGLCREATEPROGRAMPROC create_program=gl_proc<PFNGLCREATEPROGRAMPROC>("glCreateProgram");
    PFNGLATTACHSHADERPROC attach_shader=gl_proc<PFNGLATTACHSHADERPROC>("glAttachShader");
    PFNGLLINKPROGRAMPROC link_program=gl_proc<PFNGLLINKPROGRAMPROC>("glLinkProgram");
    PFNGLGETPROGRAMIVPROC get_program_iv=gl_proc<PFNGLGETPROGRAMIVPROC>("glGetProgramiv");
    PFNGLGETPROGRAMINFOLOGPROC get_program_log=gl_proc<PFNGLGETPROGRAMINFOLOGPROC>("glGetProgramInfoLog");
    PFNGLDELETEPROGRAMPROC delete_program=gl_proc<PFNGLDELETEPROGRAMPROC>("glDeleteProgram");
    PFNGLUSEPROGRAMPROC use_program=gl_proc<PFNGLUSEPROGRAMPROC>("glUseProgram");
    PFNGLGETUNIFORMLOCATIONPROC get_uniform=gl_proc<PFNGLGETUNIFORMLOCATIONPROC>("glGetUniformLocation");
    PFNGLUNIFORM2FPROC uniform2f=gl_proc<PFNGLUNIFORM2FPROC>("glUniform2f");
    PFNGLGENVERTEXARRAYSPROC gen_vertex_arrays=gl_proc<PFNGLGENVERTEXARRAYSPROC>("glGenVertexArrays");
    PFNGLBINDVERTEXARRAYPROC bind_vertex_array=gl_proc<PFNGLBINDVERTEXARRAYPROC>("glBindVertexArray");
    PFNGLDELETEVERTEXARRAYSPROC delete_vertex_arrays=gl_proc<PFNGLDELETEVERTEXARRAYSPROC>("glDeleteVertexArrays");
    PFNGLGENBUFFERSPROC gen_buffers=gl_proc<PFNGLGENBUFFERSPROC>("glGenBuffers");
    PFNGLBINDBUFFERPROC bind_buffer=gl_proc<PFNGLBINDBUFFERPROC>("glBindBuffer");
    PFNGLBUFFERDATAPROC buffer_data=gl_proc<PFNGLBUFFERDATAPROC>("glBufferData");
    PFNGLDELETEBUFFERSPROC delete_buffers=gl_proc<PFNGLDELETEBUFFERSPROC>("glDeleteBuffers");
    PFNGLENABLEVERTEXATTRIBARRAYPROC enable_attrib=gl_proc<PFNGLENABLEVERTEXATTRIBARRAYPROC>("glEnableVertexAttribArray");
    PFNGLVERTEXATTRIBPOINTERPROC attrib_pointer=gl_proc<PFNGLVERTEXATTRIBPOINTERPROC>("glVertexAttribPointer");
    PFNGLDRAWARRAYSPROC draw_arrays=gl_proc<PFNGLDRAWARRAYSPROC>("glDrawArrays");
};

static GLuint compile_shader(ProbeGl& gl,GLenum type,const char* source) {
    const auto shader=gl.create_shader(type);
    gl.shader_source(shader,1,&source,nullptr);gl.compile_shader(shader);
    GLint okay=0;gl.get_shader_iv(shader,GL_COMPILE_STATUS_,&okay);
    if(okay)return shader;
    std::array<char,1024> log{};GLsizei length=0;gl.get_shader_log(shader,GLsizei(log.size()),&length,log.data());
    gl.delete_shader(shader);throw std::runtime_error(std::string("shader compile failed: ")+log.data());
}

static GLuint make_probe_program(ProbeGl& gl) {
    constexpr auto vertex=R"(#version 330 core
layout(location=0) in vec2 position; layout(location=1) in vec2 texture_coordinate;
out vec2 uv;
void main(){ gl_Position=vec4(position,0.0,1.0); uv=texture_coordinate; })";
    constexpr auto fragment=R"(#version 330 core
in vec2 uv; uniform sampler2D scanout; out vec4 output_color;
void main(){ output_color=texture(scanout,uv); })";
    const auto vs=compile_shader(gl,GL_VERTEX_SHADER_,vertex),fs=compile_shader(gl,GL_FRAGMENT_SHADER_,fragment);
    const auto program=gl.create_program();gl.attach_shader(program,vs);gl.attach_shader(program,fs);gl.link_program(program);
    gl.delete_shader(vs);gl.delete_shader(fs);GLint okay=0;gl.get_program_iv(program,GL_LINK_STATUS_,&okay);
    if(okay)return program;
    std::array<char,1024> log{};GLsizei length=0;gl.get_program_log(program,GLsizei(log.size()),&length,log.data());
    gl.delete_program(program);throw std::runtime_error(std::string("program link failed: ")+log.data());
}

struct ProbeVertex {float x,y,u,v;};

#ifdef _WIN32
static void configure_taskbar_window(GLFWwindow* window) {
    const auto hwnd=glfwGetWin32Window(window);
    if(!hwnd)return;
    // Configure the real GLFW HWND as the only normal unowned top-level app
    // window and give that exact window the shell identity Explorer groups.
    auto ex_style=GetWindowLongPtrW(hwnd,GWL_EXSTYLE);
    ex_style&=~LONG_PTR(WS_EX_TOOLWINDOW);
    ex_style|=LONG_PTR(WS_EX_APPWINDOW);
    SetWindowLongPtrW(hwnd,GWL_EXSTYLE,ex_style);
    SetWindowLongPtrW(hwnd,GWLP_HWNDPARENT,0);
    IPropertyStore* properties=nullptr;
    if(SUCCEEDED(SHGetPropertyStoreForWindow(hwnd,IID_PPV_ARGS(&properties)))) {
        PROPVARIANT app_id{};
        if(SUCCEEDED(InitPropVariantFromString(L"HauntingGround.StaticRecomp.Viewer",&app_id))) {
            properties->SetValue(PKEY_AppUserModel_ID,app_id);
            properties->Commit();
            PropVariantClear(&app_id);
        }
        properties->Release();
    }
    POINT origin{0,0};
    const auto monitor=MonitorFromPoint(origin,MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO info{sizeof(info)};
    if(monitor && GetMonitorInfoW(monitor,&info)) {
        constexpr int width=960,height=540;
        const int x=info.rcWork.left+(info.rcWork.right-info.rcWork.left-width)/2;
        const int y=info.rcWork.top+(info.rcWork.bottom-info.rcWork.top-height)/2;
        SetWindowPos(hwnd,HWND_TOP,x,y,width,height,SWP_FRAMECHANGED|SWP_NOACTIVATE);
    } else {
        SetWindowPos(hwnd,HWND_TOP,100,100,960,540,SWP_FRAMECHANGED|SWP_NOACTIVATE);
    }
}

static void register_taskbar_window(GLFWwindow* window) {
    const auto hwnd=glfwGetWin32Window(window);
    if(!hwnd)return;
    const auto com=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    ITaskbarList* taskbar=nullptr;
    if(SUCCEEDED(CoCreateInstance(CLSID_TaskbarList,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&taskbar)))) {
        if(SUCCEEDED(taskbar->HrInit())) {
            taskbar->DeleteTab(hwnd);
            taskbar->AddTab(hwnd);
        }
        taskbar->Release();
    }
    FLASHWINFO flash{sizeof(FLASHWINFO),hwnd,FLASHW_TRAY,2,0};
    FlashWindowEx(&flash);
    if(com==S_OK || com==S_FALSE)CoUninitialize();
}
#endif

