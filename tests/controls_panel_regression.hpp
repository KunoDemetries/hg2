#pragma once
#include "hg/controls_panel.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

inline int controls_panel_regression() {
    using namespace hg::host_controls;
    const auto require=[](bool value,const char* message) {
        if(!value)throw std::runtime_error(message);
    };
    PanelState ui;
    require(!ui.update(false,false)&&!ui.open,"help should start closed");
    require(!ui.update(true,false)&&ui.open,"F1 should open help");
    for(unsigned n=0;n<100;++n)require(!ui.update(true,false)&&ui.open,"held F1 retriggered");
    require(!ui.update(false,false)&&ui.open,"key release closed help");
    require(!ui.update(false,true)&&!ui.open,"Escape should close only help");
    for(unsigned n=0;n<100;++n)require(!ui.update(false,true)&&!ui.open,"held Escape quit after closing help");
    require(!ui.update(false,false),"Escape release requested quit");
    require(ui.update(false,true),"separate Escape press should request host close");
    ui={};
    require(!ui.update(true,false)&&ui.open,"second open failed");
    require(!ui.update(false,false)&&ui.open,"focus/key release changed visibility");
    require(!ui.update(true,false)&&!ui.open,"F1 should toggle help off");
    ui={};ui.open=true;
    require(!ui.update(true,true)&&!ui.open,"Escape should win over simultaneous F1");

    std::uint64_t comparisons=0;
    for(unsigned bits=0;bits<65536;++bits) {
        hg::KeyboardKeys keys{};
        for(unsigned n=0;n<16;++n)keys[n]=(bits&(1u<<n))!=0;
        for(unsigned n=16;n<hg::keyboard_key_count;++n)keys[n]=(bits&(1u<<(n-16)))!=0;
        const auto original=keys;
        for(bool focused:{false,true})for(bool tab:{false,true}) {
            auto expected=hg::keyboard_input(keys,focused);
            if(tab&&focused)expected.buttons&=std::uint16_t(0xfffe);
            require(keyboard_with_select_alias(keys,tab,focused)==expected,"Tab Select alias changed other input");
            require(keys==original,"Select alias mutated caller key state");
            ++comparisons;
        }
    }
    require(bindings[7].control=="L1"&&bindings[7].keys=="Q","L1 reference mismatch");
    require(bindings[8].control=="L2"&&bindings[8].keys=="1 (TOP ROW)","L2 reference mismatch");
    require(bindings[9].control=="R1"&&bindings[9].keys=="E","R1 reference mismatch");
    require(bindings[10].control=="R2"&&bindings[10].keys=="3 (TOP ROW)","R2 reference mismatch");
    require(bindings[14].control=="SELECT"&&bindings[14].keys=="TAB / BACKSPACE","Select reference mismatch");
    const auto image=make_panel();
    require(image.pixels.size()==PanelImage::width*PanelImage::height,"panel image size mismatch");
    unsigned bright_pixels=0;
    for(auto pixel:image.pixels) {
        require((pixel>>24)==255,"panel should be opaque");
        bright_pixels+=(pixel&255)>100;
    }
    require(bright_pixels>10000,"panel text missing");
    bool fault=false;
    try{(void)glyph('?');}catch(const std::invalid_argument&){fault=true;}
    require(fault,"unknown UI glyph should fault");
    auto bounds_image=image;
    fault=false;
    try{bounds_image.fill(PanelImage::width,0,1,1,0);}catch(const std::out_of_range&){fault=true;}
    require(fault,"out-of-bounds UI fill should fault");
    fault=false;
    try{bounds_image.text(0,0,"A",0,0);}catch(const std::out_of_range&){fault=true;}
    require(fault,"zero-scale UI text should fault");
    fault=false;
    try{bounds_image.text(PanelImage::width-1,0,"A",2,0);}catch(const std::out_of_range&){fault=true;}
    require(fault,"out-of-bounds UI text should fault");
    require(bounds_image.pixels==image.pixels,"rejected UI operation mutated image");

    // Redistributable synthetic host-UI preview, never an original game/frame capture.
    const auto directory=std::filesystem::temp_directory_path()/"haunting-toc-probe";
    std::filesystem::create_directories(directory);
    const auto path=directory/"controls-panel-preview.ppm";
    std::ofstream output(path,std::ios::binary);
    require(bool(output),"cannot create controls reference preview");
    output<<"P6\n"<<PanelImage::width<<' '<<PanelImage::height<<"\n255\n";
    for(auto pixel:image.pixels) {
        const char rgb[]{char(pixel),char(pixel>>8),char(pixel>>16)};
        output.write(rgb,3);
    }
    output.close();require(bool(output),"cannot write controls reference preview");
    std::cout<<"Controls panel: hotkey/held-Escape tests, "<<comparisons
             <<" Select alias/focus/input comparisons and checked bitmap passed; preview="<<path<<'\n';
    return 0;
}
