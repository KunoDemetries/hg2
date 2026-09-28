#include "hg/native_host.hpp"
#include <exception>
#include <iostream>

int main(int argc,char** argv) {
    try{return hg::run_native_game(argc,argv);}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
