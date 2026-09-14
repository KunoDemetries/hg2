#define GLFW_INCLUDE_NONE
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#include <GLFW/glfw3.h>
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

// Read only complete bounded P6 frames emitted by the connected diagnostic.
static bool read_preview(const std::filesystem::path& path,hg::GsDisplayImage& image) {
    std::ifstream input(path,std::ios::binary);
    std::string magic;unsigned width=0,height=0,maximum=0;
    if(!(input>>magic>>width>>height>>maximum) || magic!="P6" || maximum!=255 ||
       !width || !height || width>2048 || height>2048 || input.get()!='\n')return false;
    std::vector<unsigned char> rgb(std::size_t(width)*height*3);
    if(!input.read(reinterpret_cast<char*>(rgb.data()),std::streamsize(rgb.size())))return false;
    image.width=width;image.height=height;image.rgba.resize(std::size_t(width)*height);
    for(std::size_t n=0;n<image.rgba.size();++n)
        image.rgba[n]=std::uint32_t(rgb[n*3])|(std::uint32_t(rgb[n*3+1])<<8)|
                      (std::uint32_t(rgb[n*3+2])<<16)|0xff000000u;
    return true;
}

static hg::GsDisplayImage probe_scanout() {
    hg::GsRegisterState gs;gs.ensure_vram();
    for(std::uint32_t y=0;y<4;++y)for(std::uint32_t x=0;x<4;++x)
        gs.vram[hg::GsRegisterState::psmct32_word(0,64,x,y)]=0xff604020;
    for(std::uint32_t y=1;y<=2;++y)for(std::uint32_t x=1;x<=2;++x)
        gs.vram[hg::GsRegisterState::psmct32_word(0,64,x,y)]=0xff40c080;
    gs.write_privileged(0x12000000,1); // PMODE.EN1
    gs.write_privileged(0x12000070,1ull<<9); // DISPFB1: base 0, width 64, CT32.
    gs.write_privileged(0x12000080,(3ull<<32)|(3ull<<44)); // DISPLAY1: 4x4 source/output.
    return gs.display_image();
}

int main(int argc, char** argv) {
    int frame_limit = 0;
    std::filesystem::path preview;
    for(int n=1;n<argc;++n) {
        const std::string arg=argv[n];
        if(arg=="--frames" && n+1<argc)frame_limit=std::stoi(argv[++n]);
        else if(arg=="--watch-display" && n+1<argc)preview=argv[++n];
        else {std::cerr<<"Usage: hg_opengl_host [--frames count] [--watch-display external.ppm]\n";return 1;}
    }
    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW " << code << ": " << message << '\n';
    });
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    auto* window = glfwCreateWindow(960, 540, "Haunting Ground - OpenGL GS scanout probe", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    if(!preview.empty())glfwSetWindowTitle(window,"Haunting Ground - waiting for diagnostic framebuffer");
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
    bool good = true;
    GLuint program=0,vao=0,vbo=0,texture=0; ProbeGl* gl=nullptr;
    try {
        gl=new ProbeGl();program=make_probe_program(*gl);gl->gen_vertex_arrays(1,&vao);gl->gen_buffers(1,&vbo);
        const std::array<ProbeVertex,6> vertices{{{-1,-1,0,1},{1,-1,1,1},{-1,1,0,0},
                                                  {-1,1,0,0},{1,-1,1,1},{1,1,1,0}}};
        hg::GsDisplayImage scanout;
        if(preview.empty())scanout=probe_scanout();
        else {scanout.width=scanout.height=1;scanout.rgba={0xff000000u};}
        glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,GLsizei(scanout.width),GLsizei(scanout.height),0,
                     GL_RGBA,GL_UNSIGNED_BYTE,scanout.rgba.data());
        gl->bind_vertex_array(vao);gl->bind_buffer(GL_ARRAY_BUFFER_,vbo);
        gl->buffer_data(GL_ARRAY_BUFFER_,sizeof(vertices),vertices.data(),GL_STATIC_DRAW_);
        gl->enable_attrib(0);gl->attrib_pointer(0,2,GL_FLOAT,GL_FALSE,sizeof(ProbeVertex),nullptr);
        gl->enable_attrib(1);gl->attrib_pointer(1,2,GL_FLOAT,GL_FALSE,sizeof(ProbeVertex),reinterpret_cast<void*>(offsetof(ProbeVertex,u)));
    } catch(const std::exception& error) { std::cerr << error.what() << '\n';glfwDestroyWindow(window);glfwTerminate();return 1; }
    int frames = 0;
    unsigned scanout_width=preview.empty()?4:1,scanout_height=scanout_width;
    double next_preview=0;
    std::filesystem::file_time_type last_preview{};
    bool have_preview=false;
    hg::GsDisplayImage latest_preview;
    bool preview_verified=false;
    while (!glfwWindowShouldClose(window) && (!frame_limit || frames < frame_limit)) {
        if(!preview.empty() && glfwGetTime()>=next_preview) {
            next_preview=glfwGetTime()+0.25;
            std::error_code error;const auto stamp=std::filesystem::last_write_time(preview,error);
            if(!error && (!have_preview || stamp!=last_preview)) {
                hg::GsDisplayImage scanout;
                if(read_preview(preview,scanout)) {
                    glBindTexture(GL_TEXTURE_2D,texture);
                    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,GLsizei(scanout.width),GLsizei(scanout.height),0,
                                 GL_RGBA,GL_UNSIGNED_BYTE,scanout.rgba.data());
                    scanout_width=scanout.width;scanout_height=scanout.height;
                    last_preview=stamp;have_preview=true;
                    latest_preview=std::move(scanout);preview_verified=false;
                    glfwSetWindowTitle(window,"Haunting Ground - latest diagnostic framebuffer (Esc closes preview)");
                }
            }
        }
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.08f, 0.04f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if(!preview.empty() && width>0 && height>0) {
            const double scale=std::min(double(width)/scanout_width,double(height)/scanout_height);
            const int w=int(scanout_width*scale),h=int(scanout_height*scale);
            glViewport((width-w)/2,(height-h)/2,w,h);
        }
        gl->use_program(program);glBindTexture(GL_TEXTURE_2D,texture);
        gl->bind_vertex_array(vao);gl->draw_arrays(GL_TRIANGLES,0,6);
        if(have_preview && !preview_verified && width>0 && height>0) {
            GLint viewport[4]{};glGetIntegerv(GL_VIEWPORT,viewport);
            if(viewport[2]>0 && viewport[3]>0) {
                const int px=viewport[0]+viewport[2]/2,py=viewport[1]+viewport[3]/2;
                const auto x=std::min(scanout_width-1,unsigned((px-viewport[0]+0.5)*scanout_width/viewport[2]));
                const auto y=std::min(scanout_height-1,unsigned((1.0-(py-viewport[1]+0.5)/viewport[3])*scanout_height));
                const auto expected=latest_preview.rgba[std::size_t(y)*scanout_width+x];
                unsigned char pixel[4]{};glReadPixels(px,py,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
                for(unsigned c=0;c<3;++c)if(std::abs(int(pixel[c])-int((expected>>(8*c))&255))>2)good=false;
                if(!good){std::cerr<<"Diagnostic preview readback mismatch\n";break;}
                preview_verified=true;
            }
        }
        if (preview.empty() && width > 0 && height > 0 && frames == 0) {
            unsigned char pixel[4]{};
            glReadPixels(width / 2, height / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
            const auto close_enough=[](unsigned value,unsigned expected){return value+2>=expected&&value<=expected+2;};
            if (!close_enough(pixel[0],0x80)||!close_enough(pixel[1],0xc0)||!close_enough(pixel[2],0x40)||pixel[3]!=0xff) {
                std::cerr << "OpenGL GS scanout probe readback mismatch\n";
                good = false; break;
            }
        }
        if (glGetError() != GL_NO_ERROR) { good = false; break; }
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        ++frames;
    }
    glDeleteTextures(1,&texture);gl->delete_buffers(1,&vbo);gl->delete_vertex_arrays(1,&vao);gl->delete_program(program);delete gl;
    glfwDestroyWindow(window);
    glfwTerminate();
    if(!preview.empty() && frame_limit && !have_preview) {
        std::cerr<<"No complete diagnostic frame was loaded\n";good=false;
    }
    std::cout << "Presented " << frames << (preview.empty()?" GS-scanout probe frames\n":" diagnostic preview frames\n");
    return good ? 0 : 1;
}
