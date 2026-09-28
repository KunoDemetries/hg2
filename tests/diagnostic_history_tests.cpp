#include "hg/diagnostic_history.hpp"
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

struct Trace {bool valid=false,return_seen=false;unsigned id=0,arm=0;};
static void require(bool value,const char* message) {
    if(!value)throw std::runtime_error(message);
}
template<std::size_t N>
static std::vector<unsigned> completed(const std::array<Trace,N>& traces,std::size_t cursor) {
    std::vector<unsigned> result;
    const auto count=std::min(cursor,N);
    for(std::size_t n=0;n<count;++n) {
        const auto& trace=traces[(cursor+N-count+n)%N];
        if(trace.valid && trace.return_seen)result.push_back(trace.id);
    }
    return result;
}
int main() {
    try {
        std::array<Trace,4> traces{};std::size_t cursor=0;
        hg::DiagnosticReturnRefresh<4> refresh;
        require(refresh.needs_scan(traces,cursor),"initial scan missing");
        require(!refresh.needs_scan(traces,cursor),"unchanged empty ring rescanned");
        traces[cursor++%4]={true,false,7,0};
        traces[cursor++%4]={true,false,8,0};
        require(refresh.needs_scan(traces,cursor),"insertions missed");
        traces[0].return_seen=true;
        require(refresh.needs_scan(traces,cursor),"older concurrent return missed");
        traces[1].arm=1;
        require(!refresh.needs_scan(traces,cursor),"irrelevant arm metadata rescanned");
        traces[1].return_seen=true;
        require(refresh.needs_scan(traces,cursor),"latest return missed");
        for(unsigned n=0;n<1000;++n)
            require(!refresh.needs_scan(traces,cursor),"completed ring rescanned");
        std::vector<unsigned> observed=completed(traces,cursor);
        std::uint32_t random=0x12345678;
        for(unsigned step=0;step<20000;++step) {
            random=random*1664525u+1013904223u;
            const auto index=(random>>8)%traces.size();
            switch(random%7) {
            case 0:case 1:traces[cursor++%traces.size()]={true,false,(random>>16)%23,0};break;
            case 2:case 3:if(traces[index].valid)traces[index].return_seen=true;break;
            case 4:++traces[index].arm;break;
            case 5:if(step%31==0){traces={};cursor=0;}break;
            default:break;
            }
            if(refresh.needs_scan(traces,cursor))observed=completed(traces,cursor);
            require(observed==completed(traces,cursor),"cached scan differs from original full scan");
        }
        std::cout<<"Diagnostic history refresh: insertion, wrap, reset, out-of-order returns and 20000 transitions passed\n";
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
