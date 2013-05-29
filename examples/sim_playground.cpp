// Author:  Mario S. Könz <mskoenz@gmx.net>
// Date:    06.05.2013 12:07:06 EDT
// File:    sim_playground.cpp

#include <iostream>
#include <sim_class.hpp>
#include <bash_parameter2_msk.hpp>

using namespace std;
using namespace perimeter;

void test_loop_count(sim_class & sim) {
    sim.grid().init_loops();
    auto start_loops = sim.grid().n_loops();
    sim.grid().print_all({}, addon::parameter["-f"]);
    
    sim.grid().set_shift_mode(qmc::ket_preswap);
    sim.grid().init_loops();
    auto preswap_loops = sim.grid().n_loops();
    sim.grid().print_all({}, addon::parameter["-f"]);
    
    sim.grid().set_shift_mode(qmc::ket_swap);
    sim.grid().init_loops();
    auto swap_loops = sim.grid().n_loops();
    sim.grid().print_all({}, addon::parameter["-f"]);
    
    DEBUG_VAR(start_loops)
    DEBUG_VAR(preswap_loops)
    DEBUG_VAR(swap_loops)
}

void test_sim(sim_class & sim) {
    sim.grid().print_all({}, addon::parameter["-f"]);
    sim.run();
    //~ while(1) {
        //~ char a;
        //~ std::cin >> a;
        //~ sim.update();
        //~ sim.grid().print_all({0,1}, addon::parameter["-f"]);
    //~ }
    
    sim.grid().set_shift_mode(qmc::ket_preswap);
    sim.grid().print_all({0,1}, addon::parameter["-f"]);
    sim.present_data();
    
}

void test_spin_copy(sim_class & sim) {
    sim.grid().set_shift_mode(qmc::ket_preswap);
    sim.grid().init_loops();
    sim.grid().copy_to_ket();
    sim.grid().print_all({0,1,2}, addon::parameter["-f"]);
}

void run_dual_sim() {
    
    addon::timer_class<addon::normal> timer(addon::parameter["-L"]*2+2);
    
    
    for(uint i = 0; i <= addon::parameter["-L"]; ++i) {
        timer.progress(i);
        DEBUG_MSG("pos")
        DEBUG_VAR(i)
        addon::parameter.set("g", i);
        sim_class sim(addon::parameter.get(), "../examples/swap_16x16p.txt");
        sim.run();
    }
    
    for(uint i = 0; i <= addon::parameter["-L"]; ++i) {
        timer.progress(addon::parameter["-L"] + 1 + i);
        DEBUG_MSG("neg")
        DEBUG_VAR(i)
        addon::parameter.set("g", i);
        sim_class sim(addon::parameter.get(), "../examples/swap_16x16m.txt");
        sim.run();
    }
}

void run_single_sim() {
    addon::timer_class<addon::normal> timer(addon::parameter["-L"]  +1);
    
    for(uint i = 0; i <= addon::parameter["-L"]; ++i) {
        timer.progress(i);
        DEBUG_MSG("pos")
        DEBUG_VAR(i)
        addon::parameter.set("g", i);
        sim_class sim(addon::parameter.get(), "../examples/swap_16x16.txt");
        sim.run();
    }
}
int main(int argc, char* argv[])
{
    std::string test = "";
    //~ std::string test = "../../";
    addon::parameter.set("init0", 0);
    addon::parameter.set("init1", 0);
    addon::parameter.set("f", 1);
    addon::parameter.set("g", 0);
    
    addon::parameter.set("term", 1000);
    addon::parameter.set("sim", 100000);
    
    addon::parameter.set("H", 16);
    addon::parameter.set("L", 16);
    
    addon::parameter.read(argc, argv);
    
    sim_class sim(addon::parameter.get(), "../examples/swap_16x16.txt");
    
    //~ test_sim(sim);
    //~ test_spin_copy(sim);
    //~ run_dual_sim();
    run_single_sim();
    
    return 0;
}
