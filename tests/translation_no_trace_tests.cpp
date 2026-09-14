#include "hg/image.hpp"
#include <iostream>
#define CHECK(x) do {if(!(x)){std::cerr<<"Failed line "<<__LINE__<<'\n';return 1;}}while(0)
int main() {
    hg::State s;
    CHECK(!hg::compiled_ee_instruction_trace());
    s.pc=0x1a000;s.w(1,3);
    hg::run_burst(s,6);
    CHECK(s.pc==0x1a00c && s.r(1)==0 && s.r(2)==3 && s.pc_trace_cursor==0);
    s.add_trace_watch(0x1a000);
    bool rejected=false;
    try{hg::run(s,1);}catch(const hg::Fault& e){rejected=std::string(e.what()).find("history is disabled")!=std::string::npos;}
    CHECK(rejected && s.pc==0x1a00c && s.r(1)==0);
    s.trace_watches.clear();s.pc=0xdead000;
    rejected=false;
    try{hg::run(s,1);}catch(const hg::Fault& e){rejected=std::string(e.what()).find("no static translation")!=std::string::npos;}
    CHECK(rejected);
    return 0;
}
