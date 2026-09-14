#include "hg/controller.hpp"
#include <iostream>
#define CHECK(x) do {if(!(x)){std::cerr<<"Failed line "<<__LINE__<<'\n';return 1;}}while(0)
int main(){
    hg::Controller pad;
    auto r=pad.exchange({1,0x43,0,1,0},0xbff7);
    CHECK(r==std::vector<std::uint8_t>({0xff,0x41,0x5a,0xf7,0xbf}) && pad.configuration);
    auto query=[&](std::uint8_t cmd,std::uint8_t param=0){return pad.exchange({1,cmd,0,param,0,0,0,0,0},0xffff);};
    r=query(0x41);CHECK(r[3]==0 && r[8]==0);
    r=query(0x45);CHECK(r==std::vector<std::uint8_t>({0xff,0xf3,0x5a,3,2,0,2,1,0}));
    r=query(0x46,0);CHECK(r[5]==1 && r[6]==2 && r[8]==10);
    r=query(0x46,1);CHECK(r[6]==1 && r[7]==1 && r[8]==20);
    CHECK(query(0x4c,0)[6]==4 && query(0x4c,1)[6]==7);
    pad.exchange({1,0x44,0,1,3,0,0,0,0},0xffff);CHECK(pad.analog && pad.analog_locked);
    CHECK(query(0x45)[5]==1);
    r=query(0x41);CHECK(r[3]==0xff && r[4]==0xff && r[5]==3 && r[8]==0x5a);
    r=pad.exchange({1,0x4d,0,0,1,0xff,0xff,0xff,0xff},0xffff);CHECK(r[3]==0xff && r[8]==0xff);
    query(0x43,0);CHECK(!pad.configuration);
    pad.axes={0x10,0x20,0x30,0x40};r=pad.exchange({1,0x42,0,1,0x77,0,0,0,0},0xfffe);
    CHECK(r==std::vector<std::uint8_t>({0xff,0x73,0x5a,0xfe,0xff,0x10,0x20,0x30,0x40}));
    CHECK(pad.small_motor==1 && pad.large_motor==0x77);
    bool rejected=false;try{query(0x45);}catch(const std::runtime_error&){rejected=true;}CHECK(rejected);
    pad.exchange({1,0x43,0,1,0,0,0,0,0},0xffff);
    r=pad.exchange({1,0x4f,0,0xff,0xff,3,0,0,0},0xffff);
    CHECK(r==std::vector<std::uint8_t>({0xff,0xf3,0x5a,0,0,0,0,0,0x5a}));
    CHECK(pad.pressure && pad.analog);
    for(unsigned button=0;button<12;++button) {
        r=pad.exchange({1,0x40,0,std::uint8_t(button),2,0,0,0,0},0xffff);
        CHECK(r==std::vector<std::uint8_t>({0xff,0xf3,0x5a,0,0,2,0,0,0x5a}));
    }
    CHECK(pad.initialized_pressure_sensors==0xfff && pad.pressure);
    rejected=false;try{pad.exchange({1,0x40,0,12,2,0,0,0,0},0xffff);}catch(const std::runtime_error&){rejected=true;}CHECK(rejected);
    query(0x43,0);
    r=pad.exchange({1,0x42,0,0,0},0xfffe);
    CHECK(r==std::vector<std::uint8_t>({0xff,0x79,0x5a,0xfe,0xff}));
    std::vector<std::uint8_t> poll(21,0);poll[0]=1;poll[1]=0x42;
    pad.pressures[0]=0x35;pad.pressures[11]=0xab;
    r=pad.exchange(poll,0xffff);
    CHECK(r.size()==21 && r[1]==0x79 && r[9]==0x35 && r[20]==0xab);
    poll[1]=0x43;poll[3]=1;pad.exchange(poll,0xffff);
    query(0x44,1);CHECK(!pad.pressure);
    rejected=false;try{query(0x4f);}catch(const std::runtime_error&){rejected=true;}CHECK(rejected);
    return 0;
}
