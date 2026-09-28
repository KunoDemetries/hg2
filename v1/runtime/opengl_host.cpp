#include "opengl_support.hpp"

struct PreviewImage {
    unsigned width=0,height=0;
    std::vector<unsigned char> rgb;
};

// Retain the publisher's packed RGB bytes for upload and pixel verification.
// The incoming and displayed buffers are reused; incomplete input never replaces
// the displayed image. No guest rendering or timing is involved here.
static bool read_preview(const std::filesystem::path& path,PreviewImage& image) {
    std::ifstream input(path,std::ios::binary);
    std::string magic;unsigned width=0,height=0,maximum=0;
    if(!(input>>magic>>width>>height>>maximum) || magic!="P6" || maximum!=255 ||
       !width || !height || width>2048 || height>2048 || input.get()!='\n')return false;
    image.rgb.resize(std::size_t(width)*height*3);
    if(!input.read(reinterpret_cast<char*>(image.rgb.data()),std::streamsize(image.rgb.size())))return false;
    // Release the file before comparing bytes or submitting the OpenGL upload.
    input.close();
    image.width=width;image.height=height;
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
#ifdef _WIN32
    // Give Explorer a stable application identity for taskbar grouping.
    SetCurrentProcessExplicitAppUserModelID(L"HauntingGround.StaticRecomp.Viewer");
#endif
    int frame_limit = 0;
    std::filesystem::path preview,presentation_log;
    for(int n=1;n<argc;++n) {
        const std::string arg=argv[n];
        if(arg=="--frames" && n+1<argc)frame_limit=std::stoi(argv[++n]);
        else if(arg=="--watch-display" && n+1<argc)preview=argv[++n];
        else if(arg=="--presentation-log" && n+1<argc)presentation_log=argv[++n];
        else {std::cerr<<"Usage: hg_opengl_host [--frames count] [--watch-display external.ppm] [--presentation-log external.csv]\n";return 1;}
    }
    // HG-DIAG-011: optional host-only evidence of changed images submitted for
    // display. Buffer swaps alone do not measure guest image cadence.
    std::ofstream presentation_events;
    if(!presentation_log.empty()) {
        const auto target=std::filesystem::absolute(presentation_log).lexically_normal();
        const auto relative=target.lexically_relative(std::filesystem::current_path().lexically_normal());
        if(preview.empty() || relative.empty() || *relative.begin()!="..") {
            std::cerr<<"Presentation log requires --watch-display and a path outside the repository\n";return 1;
        }
        presentation_events.open(target,std::ios::out|std::ios::trunc);
        if(!presentation_events){std::cerr<<"Cannot open presentation log\n";return 1;}
        presentation_events<<"change,swap,host_ns,elapsed_seconds,width,height\n"<<std::setprecision(12);
    }
    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW " << code << ": " << message << '\n';
    });
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED,GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);
#ifdef _WIN32
    glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
#endif
    auto* window = glfwCreateWindow(960, 540, "Haunting Ground - OpenGL GS scanout probe", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwSetKeyCallback(window,[](GLFWwindow* target,int key,int,int action,int) {
        if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)glfwSetWindowShouldClose(target,GLFW_TRUE);
    });
    if(!preview.empty())glfwSetWindowTitle(window,"Haunting Ground - waiting for diagnostic framebuffer");
#ifdef _WIN32
    configure_taskbar_window(window);
    glfwShowWindow(window);
    // Showing a previously hidden GLFW window can let Windows restore a stale
    // off-screen placement. Set the GLFW-owned position after ShowWindow so
    // GLFW and Win32 agree on an on-screen primary-monitor location.
    glfwSetWindowPos(window,100,100);
    glfwFocusWindow(window);
    register_taskbar_window(window);
#endif
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
    std::filesystem::file_time_type last_preview{};
    bool have_preview=false;
    PreviewImage latest_preview,incoming_preview;
    bool preview_verified=false,preview_pending_presentation=false;
    bool refresh_requested=true;
    glfwSetWindowUserPointer(window,&refresh_requested);
    glfwSetWindowRefreshCallback(window,[](GLFWwindow* target) {
        *static_cast<bool*>(glfwGetWindowUserPointer(target))=true;
    });
    int last_framebuffer_width=-1,last_framebuffer_height=-1;
    std::uint64_t preview_reads=0,changed_images=0,idle_polls=0;
    while (!glfwWindowShouldClose(window) && (!frame_limit || frames < frame_limit)) {
        bool image_changed=false;
        // Check on every swap-paced iteration. A 250ms timer previously limited
        // visible updates to four per second regardless of native throughput.
        if(!preview.empty()) {
            std::error_code error;const auto stamp=std::filesystem::last_write_time(preview,error);
            if(!error && (!have_preview || stamp!=last_preview)) {
                auto& scanout=incoming_preview;
                if(read_preview(preview,scanout)) {
                    ++preview_reads;
                    image_changed=!have_preview || scanout.width!=scanout_width ||
                                  scanout.height!=scanout_height || scanout.rgb!=latest_preview.rgb;
                    if(image_changed) {
                        glBindTexture(GL_TEXTURE_2D,texture);
                        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
                        if(scanout.width==scanout_width && scanout.height==scanout_height)
                            glTexSubImage2D(GL_TEXTURE_2D,0,0,0,GLsizei(scanout.width),GLsizei(scanout.height),
                                           GL_RGB,GL_UNSIGNED_BYTE,scanout.rgb.data());
                        else
                            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,GLsizei(scanout.width),GLsizei(scanout.height),0,
                                         GL_RGB,GL_UNSIGNED_BYTE,scanout.rgb.data());
                        scanout_width=scanout.width;scanout_height=scanout.height;
                        std::swap(latest_preview,scanout);preview_verified=false;
                        preview_pending_presentation=true;
                    }
                    if(!have_preview) {
                        glfwSetWindowTitle(window,"Haunting Ground - latest diagnostic framebuffer (Esc closes preview)");
#ifdef _WIN32
                        register_taskbar_window(window);
#endif
                    }
                    last_preview=stamp;have_preview=true;
                }
            }
        }
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        // An unchanged diagnostic image needs no new draw or buffer swap.
        // Poll the external publisher with a bounded host-only wait, and still
        // repaint after resize/exposure. Bounded --frames probes deliberately
        // retain their requested number of swaps. Changed images keep every
        // existing readback and OpenGL error check below.
        if(!preview.empty() && !frame_limit && !image_changed && !refresh_requested &&
           width==last_framebuffer_width && height==last_framebuffer_height) {
            ++idle_polls;
            glfwWaitEventsTimeout(0.004);
            continue;
        }
        refresh_requested=false;
        last_framebuffer_width=width;last_framebuffer_height=height;
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
                unsigned char pixel[4]{};glReadPixels(px,py,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
                // Nearest filtering changes texels at half-texel boundaries. A
                // framebuffer pixel center that lands exactly on one of those
                // boundaries can legally select the adjacent source texel on a
                // driver. Preserve the live-preview correctness check while
                // accepting that one-texel neighborhood; the synthetic probe
                // below still validates an exact rendered color independently.
                bool matched=false;
                for(int dy=-1;dy<=1 && !matched;++dy)for(int dx=-1;dx<=1 && !matched;++dx) {
                    const auto sx=std::clamp<int>(int(x)+dx,0,int(scanout_width)-1);
                    const auto sy=std::clamp<int>(int(y)+dy,0,int(scanout_height)-1);
                    const auto expected=(std::size_t(sy)*scanout_width+std::size_t(sx))*3;
                    matched=true;
                    for(unsigned c=0;c<3;++c)
                        if(std::abs(int(pixel[c])-int(latest_preview.rgb[expected+c]))>2){matched=false;break;}
                }
                if(!matched){
                    const auto expected=(std::size_t(y)*scanout_width+x)*3;
                    std::cerr<<"Diagnostic preview readback mismatch: got "<<unsigned(pixel[0])<<','<<unsigned(pixel[1])<<','<<unsigned(pixel[2])
                             <<" expected-near "<<unsigned(latest_preview.rgb[expected])<<','<<unsigned(latest_preview.rgb[expected+1])<<','<<unsigned(latest_preview.rgb[expected+2])
                             <<" at source "<<x<<','<<y<<"\n";
                    good=false;break;
                }
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
        if(preview_pending_presentation && width>0 && height>0) {
            preview_pending_presentation=false;
            ++changed_images;
            if(presentation_events.is_open()) {
                const auto host_ns=std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                presentation_events<<changed_images<<','<<frames+1<<','<<host_ns<<','<<glfwGetTime()<<','
                                   <<scanout_width<<','<<scanout_height<<'\n';
            }
        }
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
    if(presentation_events.is_open()) {
        presentation_events.close();
        if(!presentation_events){std::cerr<<"Cannot write presentation log\n";good=false;}
    }
    std::cout << "Completed " << frames << " buffer swaps";
    if(!preview.empty())std::cout<<", loaded "<<preview_reads<<" complete previews, presented "<<changed_images
                              <<" changed images, "<<idle_polls<<" idle polls";
    std::cout<<'\n';
    return good ? 0 : 1;
}
