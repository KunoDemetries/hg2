#include "opengl_support.hpp"
#include "gl_gs.hpp"
#include "hg/gs.hpp"
#include "hg/gs_triangle_acceleration.hpp"
#include <iostream>

int main(int argc,char** argv) {
    GLFWwindow* window=nullptr;
    try {
        if(!glfwInit())throw std::runtime_error("GLFW init");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
        glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
        window=glfwCreateWindow(16,16,"GS differential tests",nullptr,nullptr);
        if(!window)throw std::runtime_error("OpenGL4.3 unavailable");
        glfwMakeContextCurrent(window);
        std::cout<<"Renderer: "<<glGetString(GL_RENDERER)<<" version "<<glGetString(GL_VERSION)<<'\n';
        bool resident=false,triangles=false;
        for(int i=1;i<argc;++i){resident|=std::string(argv[i])=="--resident";triangles|=std::string(argv[i])=="--triangles";}
        auto gpu=hg::make_gl_sprite_accelerator(resident,triangles);unsigned cases=0;
        struct CountedTriangles final:hg::GsTriangleAccelerator {
            hg::GsTriangleAccelerator& target;unsigned accepted=0;
            explicit CountedTriangles(hg::GsTriangleAccelerator& backend):target(backend){}
            bool render_triangle(const hg::GsTriangleJob& job,hg::GsLocalMemory& memory) override {
                const bool result=target.render_triangle(job,memory);accepted+=result;return result;
            }
        } triangle_counter(*dynamic_cast<hg::GsTriangleAccelerator*>(gpu.get()));
        if(triangles)for(unsigned variant=0;variant<192;++variant) {
            const bool untextured=variant>=96;
            hg::GsRegisterState cpu;cpu.ensure_vram();
            for(unsigned i=0;i<cpu.vram.size();++i)cpu.vram[i]=i*0x97a54321u+(i>>9);
            const unsigned format=variant%3==0?0:variant%3==1?0x13:0x1b,wrap=(variant/3)%4;
            cpu.value[0x4c]=(2ull<<16)|((variant&32)?511:0)|((variant&64)?0x00f00f00ull<<32:0);
            cpu.value[0x4e]=32|((variant&1)?1ull<<24:0)|((variant&8)?1ull<<32:0);
            cpu.value[0x47]=0x10000|((1ull+(variant%3))<<17)|((variant&4)?15:0);
            cpu.value[0x40]=(127ull<<16)|(95ull<<48);
            cpu.value[6]=8192ull|(2ull<<14)|(std::uint64_t(format)<<20)|(7ull<<26)|(7ull<<30)|
                ((variant&16)?0:1ull<<34)|(std::uint64_t(variant%4)<<35);
            cpu.value[0x14]=(variant&1)?0x60:0;
            cpu.value[8]=wrap|(std::uint64_t(wrap)<<2)|(7ull<<4)|(95ull<<14)|(3ull<<24)|(79ull<<34);
            cpu.value[0x46]=variant&1;cpu.value[0x49]=(variant>>1)&1;cpu.value[0x4a]=(variant>>2)&1;
            cpu.value[0x42]=0x44ull|(193ull<<32);
            for(unsigned i=0;i<256;++i){const auto rgba=0x19537bc1u*i;
                cpu.clut[i]=std::uint16_t(rgba);cpu.clut[i+256]=std::uint16_t(rgba>>16);
                cpu.clut_valid[i]=cpu.clut_valid[i+256]=true;}
            if(untextured) {
                // TME-off must ignore invalid texture format/width/CLUT/Q state.
                cpu.value[6]=~0ull;cpu.value[8]=~0ull;cpu.clut_valid.fill(false);
            }
            for(unsigned n=0;n<8;++n){
                hg::GsDraw draw{};draw.primitive=3;draw.count=3;
                draw.prim_state=(untextured?((variant&32)?256:0):0x10)|((variant&2)?8:0)|((variant&4)?64:0);
                draw.vertices[0].x=unsigned(n*16+3);draw.vertices[0].y=9;
                draw.vertices[1].x=2013;draw.vertices[1].y=unsigned(19+n*16);
                draw.vertices[2].x=unsigned(27+n*16);draw.vertices[2].y=1517;
                for(unsigned i=0;i<3;++i){draw.vertices[i].rgba=0x7189d53fu*(i+n+1);draw.vertices[i].z=0x519ca731u*(i+n+1);
                    draw.vertices[i].q=i==1?0x40000000:0x3f800000;
                    draw.vertices[i].st=std::uint64_t(i==1?0x3fc00000:0xbe800000)|
                        (std::uint64_t(i==2?0x3f800000:0x3e800000)<<32);}
                if(variant&32)for(unsigned i=0;i<3;++i)
                    draw.vertices[i].st=(draw.vertices[i].st&0xffffffff00000000ull)|(i==1?0x3a000000u:0xb9800000u);
                if(untextured)for(auto& vertex:draw.vertices)vertex.q=0;
                if(variant&16)std::swap(draw.vertices[1],draw.vertices[2]);
                cpu.draws.push_back(draw);
            }
            auto accelerated=cpu;
            hg::gs_sprite_accelerator=nullptr;hg::gs_triangle_accelerator=nullptr;cpu.rasterize_pending_draws();
            triangle_counter.accepted=0;
            hg::gs_sprite_accelerator=gpu.get();hg::gs_triangle_accelerator=&triangle_counter;
            if(variant&1){
                const auto tail=std::vector<hg::GsDraw>(accelerated.draws.begin()+4,accelerated.draws.end());
                accelerated.draws.resize(4);accelerated.rasterize_pending_draws();
                accelerated.draws.insert(accelerated.draws.end(),tail.begin(),tail.end());
            }
            accelerated.rasterize_pending_draws();hg::gs_triangle_accelerator=nullptr;hg::gs_sprite_accelerator=nullptr;
            if(cpu.vram!=accelerated.vram)throw std::runtime_error("GPU triangle differential mismatch case "+std::to_string(variant));
            if(untextured&&!triangle_counter.accepted)throw std::runtime_error("Untextured triangle test did not exercise GPU");
            ++cases;
            if(variant%96<8) {
                // Follow one GPU batch with a different texture/frame mapping,
                // a CPU primitive, and a fault. Observe pending resident writes
                // through the ordinary memory owner and copy boundaries.
                auto mixed=cpu;mixed.draws.clear();mixed.retired_draw_count=0;mixed.rasterized_draw_count=0;
                auto first=cpu.draws.front();first.environment_captured=true;first.environment=cpu.value;
                mixed.draws.push_back(first);
                auto second=first;second.environment[6]=(2ull<<14)|(7ull<<26)|(7ull<<30)|(1ull<<34);
                second.environment[0x4c]=(2ull<<16)|96;second.environment[0x4e]=128;
                mixed.draws.push_back(second);
                hg::GsDraw point{};point.primitive=0;point.count=1;point.prim_state=0;point.environment_captured=true;point.environment=second.environment;
                point.vertices[0].x=32;point.vertices[0].y=32;point.vertices[0].rgba=0xf1b29374;point.vertices[0].z=0xffffffff;
                mixed.draws.push_back(point);
                if(variant&1){auto invalid=first;invalid.prim_state|=128;mixed.draws.push_back(invalid);}
                auto reference=mixed;std::string cpu_error,gpu_error;
                try{reference.rasterize_pending_draws();}catch(const std::runtime_error& e){cpu_error=e.what();}
                hg::gs_sprite_accelerator=gpu.get();hg::gs_triangle_accelerator=dynamic_cast<hg::GsTriangleAccelerator*>(gpu.get());
                try{mixed.rasterize_pending_draws();}catch(const std::runtime_error& e){gpu_error=e.what();}
                hg::gs_sprite_accelerator=nullptr;hg::gs_triangle_accelerator=nullptr;
                auto copied=mixed;
                if(cpu_error!=gpu_error||copied.vram!=reference.vram||mixed.rasterized_draw_count!=reference.rasterized_draw_count)
                    throw std::runtime_error("GPU triangle ordering/fault mismatch "+std::to_string(variant));
                ++cases;
            }
        }
        struct CountedSprites final:hg::GsSpriteAccelerator {
            hg::GsSpriteAccelerator& target;unsigned accepted=0;
            explicit CountedSprites(hg::GsSpriteAccelerator& backend):target(backend){}
            bool render(const hg::GsSpriteJob& job,hg::GsLocalMemory& memory) override {
                const auto result=target.render(job,memory);accepted+=result;return result;}
            void begin_batch() override {target.begin_batch();}
            void end_batch() override {target.end_batch();}
            void flush() override {target.flush();}
            void finish() override {target.finish();}
        } counted(*gpu);
        for(unsigned format:{0u,2u,10u,49u})for(unsigned shifted=0;shifted<2;++shifted) {
            hg::GsRegisterState initial;initial.ensure_vram();
            for(unsigned i=0;i<initial.vram.size();++i)initial.vram[i]=0x81739badu*i+17;
            initial.value[0x4e]=(1ull<<32)|400;initial.value[0x47]=0x30000;
            initial.value[0x46]=1;initial.value[0x40]=(127ull<<16)|(95ull<<48);
            initial.value[8]=5;initial.value[0x3b]=0x80|(0x80ull<<32);
            initial.value[0x42]=0x44ull|(97ull<<32);
            auto cpu=initial,accelerated=initial;
            const auto sequence=[&](hg::GsRegisterState& state) {
                const unsigned destinations[]={0,32,64,128,96,96,511};
                const unsigned sources[]={8192,8192,0,3072,8192,8192,8192};
                for(unsigned n=0;n<7;++n) {
                    // Independent mixed-size draws, then RAW, WAR, WAW and wrap.
                    state.value[0x4c]=(2ull<<16)|(std::uint64_t(format)<<24)|destinations[n];
                    state.value[6]=sources[n]|(2ull<<14)|(7ull<<26)|(7ull<<30)|(1ull<<34);
                    hg::GsDraw draw{};draw.primitive=6;draw.count=2;draw.prim_state=0x156;
                    draw.vertices[0].x=shifted*5*16;draw.vertices[0].y=shifted*3*16;
                    draw.vertices[1].x=(n&1?112:128)*16;draw.vertices[1].y=(n&1?80:96)*16;
                    draw.vertices[1].uv=draw.vertices[1].x|(draw.vertices[1].y<<16);
                    draw.vertices[1].rgba=0x80808080;state.rasterize_sprite(draw);
                }
            };
            sequence(cpu);counted.accepted=0;hg::gs_sprite_accelerator=&counted;
            counted.begin_batch();sequence(accelerated);counted.end_batch();hg::gs_sprite_accelerator=nullptr;
            if(counted.accepted!=7)throw std::runtime_error("sprite wave fixture did not reach GPU");
            if(cpu.vram!=accelerated.vram)throw std::runtime_error("sprite wave dependency mismatch");
            ++cases;
        }
        const unsigned source_formats[]={0,1,2,10,27,49,255},destination_formats[]={0,1,2,10,49};
        for(auto source_format:source_formats)for(auto destination_format:destination_formats)for(unsigned variant=0;variant<16;++variant) {
            hg::GsRegisterState cpu;cpu.ensure_vram();
            for(unsigned i=0;i<cpu.vram.size();++i)cpu.vram[i]=i*0x97a54321u+(i>>9);
            cpu.value[0x4c]=(2ull<<16)|(std::uint64_t(destination_format)<<24)|
                ((variant&8)?(0x81f00700ull<<32)|510ull:0);
            cpu.value[0x4e]=64|((variant&1)?1ull<<24:0)|((variant&2)?1ull<<32:0);
            cpu.value[0x47]=0x10000|((1ull+variant%3)<<17)|((variant&4)?0x4000:0)|((variant&8)?0x8000:0);
            cpu.value[0x40]=std::uint64_t(variant)|(119ull<<16)|(3ull<<32)|(103ull<<48);
            cpu.value[6]=8192ull|(2ull<<14)|(std::uint64_t(source_format==255?0:source_format)<<20)|
                (7ull<<26)|(7ull<<30)|((variant&2)?0:1ull<<34)|(std::uint64_t(variant%4)<<35);
            cpu.value[0x14]=(variant&1)?0x60:0;const auto wrap=variant%4;
            cpu.value[8]=wrap|(std::uint64_t(wrap)<<2)|(7ull<<4)|(95ull<<14)|(3ull<<24)|(79ull<<34);
            cpu.value[0x3b]=0x39|(0xc7ull<<32)|((variant&4)?0x8000:0);
            cpu.value[0x44]=0x7310526473105264ull;cpu.value[0x45]=variant&1;
            cpu.value[0x46]=variant&1;cpu.value[0x49]=(variant>>1)&1;cpu.value[0x4a]=(variant>>2)&1;
            cpu.value[0x42]=0x44ull|(193ull<<32);
            for(unsigned i=0;i<256;++i){const auto rgba=0x19537bc1u*i;
                cpu.clut[i]=std::uint16_t(rgba);cpu.clut[i+256]=std::uint16_t(rgba>>16);
                cpu.clut_valid[i]=cpu.clut_valid[i+256]=true;}
            hg::GsDraw draw{};draw.primitive=6;draw.count=2;draw.prim_state=0x106|(source_format==255?0:16)|((variant&2)?64:0);
            draw.vertices[0].x=3;draw.vertices[0].y=9;draw.vertices[0].uv=24|(40u<<16);
            draw.vertices[1].x=1997;draw.vertices[1].y=1741;draw.vertices[1].uv=1991|(1737u<<16);
            draw.vertices[1].rgba=0xc19357ef;draw.vertices[1].z=0x519ca731u*(variant+1);
            if(variant&8){std::swap(draw.vertices[0].uv,draw.vertices[1].uv);}
            cpu.draws.push_back(draw);auto accelerated=cpu;const auto accepted_before=counted.accepted;
            cpu.rasterize_pending_draws();hg::gs_sprite_accelerator=&counted;
            accelerated.rasterize_pending_draws();hg::gs_sprite_accelerator=nullptr;
            const auto name=std::to_string(source_format)+"/"+std::to_string(destination_format)+"/"+std::to_string(variant);
            if(cpu.vram!=accelerated.vram) {
                for(unsigned i=0;i<cpu.vram.size();++i)if(cpu.vram[i]!=accelerated.vram[i]){
                    std::cerr<<"word="<<i<<" cpu="<<cpu.vram[i]<<" gpu="<<accelerated.vram[i]<<'\n';break;}
                throw std::runtime_error("GPU sprite format mismatch "+name);
            }
            if(counted.accepted==accepted_before)throw std::runtime_error("GPU sprite format never accelerated "+name);
            ++cases;
            if(variant<2) {
                auto aliased=accelerated;aliased.draws.clear();aliased.retired_draw_count=0;aliased.rasterized_draw_count=0;
                aliased.value[0x4c]&=~511ull;
                if(variant==0&&source_format!=255)aliased.value[6]&=~16383ull;
                else {aliased.value[0x4e]&=~(511ull|(1ull<<32));}
                aliased.draws.push_back(draw);auto reference=aliased;const auto before=counted.accepted;
                reference.rasterize_pending_draws();hg::gs_sprite_accelerator=&counted;
                aliased.rasterize_pending_draws();hg::gs_sprite_accelerator=nullptr;
                if(reference.vram!=aliased.vram||counted.accepted!=before)
                    throw std::runtime_error("GPU sprite alias fallback mismatch "+name);
                ++cases;
            }
            if(source_format==27&&variant==0) {
                auto fault=accelerated;fault.draws.clear();fault.retired_draw_count=0;fault.rasterized_draw_count=0;
                fault.clut_valid.fill(false);fault.draws.push_back(draw);auto reference=fault;
                std::string cpu_error,gpu_error;
                try{reference.rasterize_pending_draws();}catch(const std::runtime_error& e){cpu_error=e.what();}
                hg::gs_sprite_accelerator=&counted;
                try{fault.rasterize_pending_draws();}catch(const std::runtime_error& e){gpu_error=e.what();}
                hg::gs_sprite_accelerator=nullptr;
                if(cpu_error.empty()||cpu_error!=gpu_error||reference.vram!=fault.vram)
                    throw std::runtime_error("GPU sprite CLUT fault mismatch "+name);
                ++cases;
            }
        }
        for(unsigned format: {2u,10u})for(unsigned variant=0;variant<16;++variant) {
            hg::GsRegisterState cpu;cpu.ensure_vram();
            for(unsigned i=0;i<cpu.vram.size();++i)cpu.vram[i]=i*0x97a54321u+(i>>9);
            const unsigned destination_x=(variant&1)?0:8,source_x=destination_x^8;
            const unsigned base=(variant&8)?510:384;
            cpu.value[0x4c]=base|(1ull<<16)|(std::uint64_t(format)<<24)|((variant&2)?0x8731a50full<<32:0);
            cpu.value[0x4e]=64|(1ull<<24)|(1ull<<32);
            cpu.value[0x47]=0x30000|((variant&4)?0x4000:0)|((variant&8)?0x8000:0);
            cpu.value[0x40]=(63ull<<16)|(895ull<<48);
            cpu.value[6]=(base*32)|(1ull<<14)|(std::uint64_t(format)<<20)|(6ull<<26)|(10ull<<30)|(1ull<<34);
            cpu.value[0x14]=0x60;cpu.value[8]=5;cpu.value[0x3b]=0x39|(0xc7ull<<32);
            cpu.value[0x46]=1;cpu.value[0x42]=0x44;cpu.value[0x49]=(variant>>1)&1;
            hg::GsDraw draw{};draw.primitive=6;draw.count=2;draw.prim_state=0x116|((variant&2)?64:0);
            draw.vertices[0].x=destination_x*16;draw.vertices[0].uv=(source_x*16+8)|(8u<<16);
            draw.vertices[1].x=(destination_x+8)*16;draw.vertices[1].y=896*16;
            draw.vertices[1].uv=((source_x+8)*16+8)|((896*16+8u)<<16);draw.vertices[1].rgba=0xb391578f;
            cpu.draws.push_back(draw);auto accelerated=cpu;const auto before=counted.accepted;
            cpu.rasterize_pending_draws();hg::gs_sprite_accelerator=&counted;
            accelerated.rasterize_pending_draws();hg::gs_sprite_accelerator=nullptr;
            if(cpu.vram!=accelerated.vram||counted.accepted==before)
                throw std::runtime_error("GPU opposite-lane copy mismatch "+std::to_string(format)+"/"+std::to_string(variant));
            ++cases;
            if(variant<4){
                auto feedback=accelerated;feedback.draws.clear();feedback.retired_draw_count=0;feedback.rasterized_draw_count=0;
                auto unsafe=draw;
                if(variant==0){unsafe.vertices[0].uv+=1;unsafe.vertices[1].uv+=1;} // Fractional neighbor.
                if(variant==1){unsafe.vertices[0].uv+=16u<<16;unsafe.vertices[1].uv+=16u<<16;} // Other row.
                if(variant==2){unsafe.vertices[0].uv+=16;unsafe.vertices[1].uv+=16;} // Other word.
                if(variant==3){ // The source lane itself is written.
                    unsafe.vertices[0].uv=(unsafe.vertices[0].uv&0xffff0000u)|(destination_x*16+8);
                    unsafe.vertices[1].uv=(unsafe.vertices[1].uv&0xffff0000u)|((destination_x+8)*16+8);
                }
                feedback.draws.push_back(unsafe);auto reference=feedback;const auto prior=counted.accepted;
                reference.rasterize_pending_draws();hg::gs_sprite_accelerator=&counted;
                feedback.rasterize_pending_draws();hg::gs_sprite_accelerator=nullptr;
                if(reference.vram!=feedback.vram||counted.accepted!=prior)
                    throw std::runtime_error("GPU opposite-lane rejection mismatch");
                ++cases;
            }
        }
        for(unsigned variant=0;variant<192;++variant) {
            hg::GsRegisterState cpu;cpu.ensure_vram();
            for(unsigned i=0;i<cpu.vram.size();++i)cpu.vram[i]=i*0x97a54321u+(i>>9);
            const unsigned filter=variant&1,wrap=(variant>>1)&3,function=(variant>>3)&3;
            const unsigned blend=(variant>>5)%3;
            cpu.value[0x4c]=(4ull<<16)|((variant&64)?(0x00f00f00ull<<32)|480ull:0);
            cpu.value[0x4e]=1ull<<32;cpu.value[0x47]=0x30000;
            cpu.value[0x40]=(255ull<<16)|(191ull<<48);
            cpu.value[6]=8192ull|(4ull<<14)|(8ull<<26)|(8ull<<30)|
                ((variant&128)?0:1ull<<34)|(std::uint64_t(function)<<35);
            cpu.value[0x14]=(std::uint64_t(filter)<<5)|(std::uint64_t(filter)<<6);
            cpu.value[8]=wrap|(std::uint64_t(wrap)<<2)|(7ull<<4)|(191ull<<14)|(3ull<<24)|(159ull<<34);
            cpu.value[0x46]=variant&1;cpu.value[0x49]=(variant>>1)&1;cpu.value[0x4a]=(variant>>2)&1;
            cpu.value[0x42]=0x44ull|(193ull<<32);
            if(blend==2)cpu.value[0x42]=0x9aull|(255ull<<32);
            hg::GsDraw draw{};draw.primitive=6;draw.count=2;draw.prim_state=0x116|(blend?64:0);
            draw.vertices[0].x=3;draw.vertices[0].y=9;draw.vertices[0].uv=24|(40u<<16);
            draw.vertices[1].x=4093;draw.vertices[1].y=3061;
            draw.vertices[1].uv=4071|(3057u<<16);draw.vertices[1].rgba=0xc19357efu;
            if(variant&32)std::swap(draw.vertices[0],draw.vertices[1]);
            auto accelerated=cpu;
            hg::gs_sprite_accelerator=nullptr;cpu.rasterize_sprite(draw);
            hg::gs_sprite_accelerator=gpu.get();accelerated.rasterize_sprite(draw);hg::gs_sprite_accelerator=nullptr;
            if(static_cast<const hg::GsRegisterState&>(accelerated).vram[0]!=cpu.vram[0])
                throw std::runtime_error("GPU single-word read mismatch");
            if(cpu.vram!=accelerated.vram)throw std::runtime_error("GPU differential mismatch case "+std::to_string(variant));
            ++cases;
            if(variant<8) {
                const auto texel=hg::GsRegisterState::psmct32_word(8192*64,256,10,10);
                cpu.vram[texel]=0xff12ab34;accelerated.vram[texel]=0xff12ab34;
                cpu.rasterize_sprite(draw);hg::gs_sprite_accelerator=gpu.get();
                accelerated.rasterize_sprite(draw);hg::gs_sprite_accelerator=nullptr;
                const auto registers=cpu.value;
                cpu.reset_system();accelerated.reset_system();
                if(cpu.vram!=accelerated.vram)throw std::runtime_error("GPU CPU-write/reset preservation mismatch");
                ++cases;
                // Restore draw state for the other differential fixtures below.
                cpu.value=registers;accelerated.value=registers;
            }
            if(variant<16) {
                // Consecutive dispatches read earlier GPU results, then a CPU
                // point and an aliased sprite force visibility before fallback.
                auto batch=cpu;
                auto queued=draw;queued.environment_captured=true;queued.environment=batch.value;
                batch.draws.push_back(queued);
                queued.environment[6]&=~16383ull;
                queued.environment[0x4c]|=256;
                batch.draws.push_back(queued);
                hg::GsDraw point{};point.primitive=0;point.count=1;point.prim_state=0;
                point.vertices[0].x=32;point.vertices[0].y=32;point.vertices[0].rgba=0xf1b29374;
                point.environment_captured=true;point.environment=batch.value;
                batch.draws.push_back(point);
                queued.environment[6]=(queued.environment[6]&~16383ull)|8192ull;
                batch.draws.push_back(queued); // Source and frame now overlap: CPU fallback.
                auto reference=batch;reference.rasterize_pending_draws();
                hg::gs_sprite_accelerator=gpu.get();batch.rasterize_pending_draws();hg::gs_sprite_accelerator=nullptr;
                if(reference.vram!=batch.vram||reference.rasterized_draw_count!=batch.rasterized_draw_count)
                    throw std::runtime_error("GPU batch ordering mismatch "+std::to_string(variant));
                ++cases;
                auto fault=cpu;fault.draws.clear();
                queued.environment=cpu.value;fault.draws.push_back(queued);
                hg::GsDraw invalid{};invalid.primitive=7;fault.draws.push_back(invalid);
                auto fault_reference=fault;bool cpu_failed=false,gpu_failed=false;
                try{fault_reference.rasterize_pending_draws();}catch(const std::runtime_error&){cpu_failed=true;}
                hg::gs_sprite_accelerator=gpu.get();
                try{fault.rasterize_pending_draws();}catch(const std::runtime_error&){gpu_failed=true;}
                hg::gs_sprite_accelerator=nullptr;
                if(!cpu_failed||!gpu_failed||fault.vram!=fault_reference.vram||
                   fault.rasterized_draw_count!=fault_reference.rasterized_draw_count)
                    throw std::runtime_error("GPU fault boundary mismatch");
                ++cases;
            }
            if(variant<8) {
                // Aliased source must use the row-major CPU path unchanged.
                cpu.value[6]&=~16383ull;accelerated=cpu;
                cpu.rasterize_sprite(draw);hg::gs_sprite_accelerator=gpu.get();
                accelerated.rasterize_sprite(draw);hg::gs_sprite_accelerator=nullptr;
                if(cpu.vram!=accelerated.vram)throw std::runtime_error("GPU feedback fallback mismatch");
                ++cases;
            }
        }
        for(unsigned dependency=0;dependency<13;++dependency) {
            // A CPU write between service batches can be disjoint, overwrite an
            // earlier texture input, or overwrite an earlier pending output.
            // The next draw consumes that CPU write in all three cases.
            hg::GsRegisterState cpu;cpu.ensure_vram();
            for(unsigned i=0;i<cpu.vram.size();++i)cpu.vram[i]=0x9371b9a5u*i;
            cpu.value[0x4c]=2ull<<16;cpu.value[0x4e]=1ull<<32;
            cpu.value[0x47]=0x30000;cpu.value[0x46]=1;
            cpu.value[0x40]=(127ull<<16)|(95ull<<48);
            cpu.value[6]=8192ull|(2ull<<14)|(7ull<<26)|(7ull<<30)|(1ull<<34);
            cpu.value[8]=5;
            hg::GsDraw draw{};draw.primitive=6;draw.count=2;draw.prim_state=0x116;
            draw.vertices[1].x=2048;draw.vertices[1].y=1536;
            draw.vertices[1].uv=2048|(1536u<<16);draw.vertices[1].rgba=0x80808080;
            auto accelerated=cpu;
            const unsigned kind=dependency<4?dependency:(dependency-4)%3;
            const unsigned block=kind==0?4096:kind==1?8192:0;
            const auto write_word=block*64;
            const auto second=[&](hg::GsRegisterState& state){
                if(dependency<4)state.vram[write_word]=0xff193b75;
                else if(dependency<7){
                    auto* span=state.vram.write_span(write_word,16);
                    for(unsigned i=0;i<16;++i)span[i]=0xff193b75+i;
                }else if(dependency<10){
                    state.value[0x50]=(std::uint64_t(block)<<32)|(2ull<<48)|(0x13ull<<56);
                    state.value[0x52]=16|(1ull<<32);state.transfer_x=0;state.transfer_y=0;
                    state.transfer_width=16;state.transfer_remaining=16;
                    state.gif_to_vram_active=true;state.host_transfer_completed=false;
                    state.write_image_qword(0x123456789abcdef0ull,0xfedcba9876543210ull);
                }else{
                    auto* span=state.vram.write_span(write_word+2040,16);
                    for(unsigned i=0;i<16;++i)span[i]=0xff193b75+i;
                }
                state.value[6]=(state.value[6]&~16383ull)|block;
                state.value[0x4c]|=384;
                state.rasterize_sprite(draw);
            };
            cpu.rasterize_sprite(draw);second(cpu);
            hg::gs_sprite_accelerator=gpu.get();gpu->begin_batch();
            accelerated.rasterize_sprite(draw);gpu->end_batch();
            if(dependency==3)for(unsigned format:{0u,1u,2u,10u}) {
                // Read committed GPU output without executing any guest draws.
                // The wrap fixture crosses both coordinate and physical VRAM ends.
                const unsigned base=511,offset=2040;
                accelerated.privileged_pmode=1;
                accelerated.privileged_dispfb[0]=base|(2ull<<9)|(std::uint64_t(format)<<15)|
                    (std::uint64_t(offset)<<32)|(std::uint64_t(offset)<<43);
                accelerated.privileged_display[0]=(134ull<<32)|(69ull<<44);
                hg::GsDraw uncommitted{};uncommitted.primitive=7;
                accelerated.draws.push_back(uncommitted);
                const auto retired=accelerated.rasterized_draw_count;
                const auto image=accelerated.display_image();
                if(accelerated.draws.size()!=1||accelerated.rasterized_draw_count!=retired)
                    throw std::runtime_error("scanout executed an uncommitted guest draw");
                accelerated.draws.clear();
                // Compare conversion against the existing scalar readers.
                const auto& scalar=accelerated;
                for(unsigned y=0;y<70;++y)for(unsigned x=0;x<135;++x) {
                    std::uint32_t expected;
                    if(format==0||format==1) {
                        expected=scalar.pixel32(base*2048,128,offset+x,offset+y);
                        if(format==1)expected=(expected&0xffffffu)|0x80000000u;
                    } else {
                        const auto p=format==2?scalar.pixel16(base*2048,128,offset+x,offset+y):
                                              scalar.pixel16s(base*2048,128,offset+x,offset+y);
                        const auto expand=[](unsigned v){return (v<<3)|(v>>2);};
                        expected=expand(p&31)|(expand((p>>5)&31)<<8)|(expand((p>>10)&31)<<16)|
                            ((p&0x8000)?0x80000000u:0u);
                    }
                    if(image.rgba[y*135+x]!=expected)throw std::runtime_error("bulk scanout pixel mismatch");
                }
                ++cases;
            }
            // Synchronous mode commits at service return; resident mode can
            // keep this work queued and must observe the subsequent CPU write.
            gpu->begin_batch();second(accelerated);gpu->end_batch();
            hg::gs_sprite_accelerator=nullptr;
            if(cpu.vram!=accelerated.vram)throw std::runtime_error("GPU CPU write dependency mismatch "+std::to_string(dependency));
            ++cases;
        }
        for(unsigned dependency=0;dependency<4;++dependency) {
            hg::GsRegisterState initial;initial.ensure_vram();
            for(unsigned i=0;i<initial.vram.size();++i)initial.vram[i]=0x81a35719u*i;
            initial.value[0x4c]=2ull<<16;initial.value[0x4e]=1ull<<32;
            initial.value[0x47]=0x30000;initial.value[0x46]=1;
            initial.value[0x40]=(127ull<<16)|(95ull<<48);
            initial.value[6]=8192ull|(2ull<<14)|(7ull<<26)|(7ull<<30)|(1ull<<34);
            initial.value[8]=5;
            hg::GsDraw draw{};draw.primitive=6;draw.count=2;draw.prim_state=0x116;
            draw.vertices[1].x=2048;draw.vertices[1].y=1536;
            draw.vertices[1].uv=2048|(1536u<<16);draw.vertices[1].rgba=0x80808080;
            auto cpu=initial,accelerated=initial;
            const auto sequence=[&](hg::GsRegisterState& state) {
                state.rasterize_sprite(draw);
                // This 8x8 sprite is deliberately below the GPU size threshold.
                // Exercise disjoint work, GPU output reads, GPU input writes,
                // and GPU output writes before the service batch has ended.
                state.value[0x4c]=(2ull<<16)|(dependency==2?256:dependency==3?0:384);
                state.value[0x4e]=(1ull<<32)|400;
                state.value[6]=(state.value[6]&~16383ull)|(dependency==1?0:4096);
                auto cpu_draw=draw;cpu_draw.vertices[1].x=cpu_draw.vertices[1].y=128;
                cpu_draw.vertices[1].uv=128|(128u<<16);state.rasterize_sprite(cpu_draw);
                state.value[0x4c]=(2ull<<16)|64;
                state.value[6]=initial.value[6];state.rasterize_sprite(draw);
            };
            sequence(cpu);
            hg::gs_sprite_accelerator=gpu.get();gpu->begin_batch();
            sequence(accelerated);gpu->end_batch();hg::gs_sprite_accelerator=nullptr;
            if(cpu.vram!=accelerated.vram)throw std::runtime_error("bounded CPU fallback dependency mismatch "+std::to_string(dependency));
            ++cases;
        }
        for(unsigned variant=0;variant<24;++variant) {
            hg::GsRegisterState initial;initial.ensure_vram();
            for(unsigned i=0;i<initial.vram.size();++i)initial.vram[i]=i*0x3197abefu+0x715923a1u;
            initial.value[0x4c]=2ull<<16;initial.value[0x4e]=(1ull<<32)|128;
            initial.value[0x47]=0x30000;initial.value[0x46]=1;
            initial.value[0x40]=(127ull<<16)|(95ull<<48);
            initial.value[6]=8192ull|(2ull<<14)|(7ull<<26)|(7ull<<30)|(1ull<<34);
            initial.value[8]=5;
            hg::GsDraw draw{};draw.primitive=6;draw.count=2;draw.prim_state=0x116;
            draw.vertices[1].x=2048;draw.vertices[1].y=1536;
            draw.vertices[1].uv=2048|(1536u<<16);draw.vertices[1].rgba=0x80808080;
            auto cpu=initial,accelerated=initial;cpu.rasterize_sprite(draw);
            hg::gs_sprite_accelerator=gpu.get();gpu->begin_batch();accelerated.rasterize_sprite(draw);gpu->end_batch();hg::gs_sprite_accelerator=nullptr;
            // Force actual resident backend acceptance, merge repeated arbitrary
            // masks and duplicate indices, then exercise the PSMT8 caller.
            const std::uint32_t indices[4]={0,3,0,15};
            for(unsigned step=0;step<8;++step){
                const auto mask=step&1?0x0f00f0ffu:0xf0ff0f00u;
                const std::uint32_t values[4]={0x12345678u+step,0x87654321u-step,0x391757abu*step,~step};
                const bool accepted=accelerated.vram.try_masked_write(indices,values,4,mask);
                if(resident&&!accepted)throw std::runtime_error("resident masked fixture did not reach GPU");
                for(unsigned i=0;i<4;++i){
                    cpu.vram[indices[i]]=(cpu.vram[indices[i]]&~mask)|(values[i]&mask);
                    if(!accepted)accelerated.vram[indices[i]]=(accelerated.vram[indices[i]]&~mask)|(values[i]&mask);
                }
            }
            const auto transfer=[&](hg::GsRegisterState& state){
                for(unsigned pass=0;pass<3;++pass){
                    state.value[0x50]=(2ull<<48)|(0x13ull<<56);
                    state.value[0x52]=32|(4ull<<32);state.transfer_x=(variant&1)?16:0;state.transfer_y=variant%4;
                    state.transfer_width=32;state.transfer_remaining=128;
                    state.gif_to_vram_active=true;state.host_transfer_completed=false;
                    for(unsigned n=0;n<(variant&2?3u:8u);++n){
                        state.write_image_qword(0x3197abef715923a1ull*(n+pass+1),0xfedcba9876543210ull+n);
                        if((variant&4)&&n==1){const auto& memory=state.vram;volatile auto observed=memory[0];(void)observed;}
                    }
                }
            };
            transfer(cpu);transfer(accelerated);
            if(cpu.transfer_remaining!=accelerated.transfer_remaining||cpu.gif_to_vram_active!=accelerated.gif_to_vram_active||
               cpu.host_transfer_completed!=accelerated.host_transfer_completed)throw std::runtime_error("masked transfer progress mismatch");
            if(variant&8){
                cpu.value[6]=(2ull<<14)|(7ull<<26)|(7ull<<30)|(1ull<<34);cpu.value[0x4c]=(2ull<<16)|64;
                accelerated.value[6]=cpu.value[6];accelerated.value[0x4c]=cpu.value[0x4c];
                cpu.rasterize_sprite(draw);hg::gs_sprite_accelerator=gpu.get();gpu->begin_batch();
                accelerated.rasterize_sprite(draw);gpu->end_batch();hg::gs_sprite_accelerator=nullptr;
            }
            auto copied=accelerated;
            if(cpu.vram!=copied.vram||cpu.vram!=accelerated.vram)throw std::runtime_error("masked transfer dependency mismatch "+std::to_string(variant));
            ++cases;
        }
        { // Memory may outlive its accelerator: teardown must materialize writes
          // and leave the shared observer proxy safe for later CPU access.
            hg::GsRegisterState surviving;surviving.ensure_vram();
            surviving.value[0x4c]=4ull<<16;surviving.value[0x4e]=1ull<<32;
            surviving.value[0x47]=0x30000;surviving.value[0x46]=1;
            surviving.value[0x40]=(255ull<<16)|(191ull<<48);
            surviving.value[6]=8192ull|(4ull<<14)|(8ull<<26)|(8ull<<30)|(1ull<<34);
            surviving.value[8]=5;
            std::fill(surviving.vram.begin()+8192*64,surviving.vram.begin()+8192*64+65536,0x819357e1);
            hg::GsDraw draw{};draw.primitive=6;draw.count=2;draw.prim_state=0x116;
            draw.vertices[1].x=4096;draw.vertices[1].y=3072;draw.vertices[1].uv=4096|(3072u<<16);
            draw.vertices[1].rgba=0x80808080;
            auto reference=surviving;reference.rasterize_sprite(draw);
            auto temporary=hg::make_gl_sprite_accelerator(true);hg::gs_sprite_accelerator=temporary.get();
            surviving.rasterize_sprite(draw);hg::gs_sprite_accelerator=nullptr;
            const std::uint32_t at=0,value=0x7159a3c7u,mask=0xff00ff00u;
            if(!surviving.vram.try_masked_write(&at,&value,1,mask))throw std::runtime_error("teardown masked upload not accepted");
            reference.vram[0]=(reference.vram[0]&~mask)|(value&mask);
            temporary.reset();
            if(surviving.vram!=reference.vram)throw std::runtime_error("GPU observer lifetime mismatch");
            surviving.vram[0]=7;if(surviving.vram[0]!=7)throw std::runtime_error("inactive GPU observer access");
            ++cases;
        }
        gpu->finish();gpu.reset();std::cout<<cases<<" GPU/CPU cases passed\n";
        glfwDestroyWindow(window);glfwTerminate();return 0;
    }catch(const std::exception& e){hg::gs_sprite_accelerator=nullptr;std::cerr<<e.what()<<'\n';
        if(window)glfwDestroyWindow(window);glfwTerminate();return 1;}
}
