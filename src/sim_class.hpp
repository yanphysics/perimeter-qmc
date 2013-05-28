// Author:  Mario S. Könz <mskoenz@gmx.net>
// Date:    18.04.2013 12:48:33 EDT
// File:    sim_class.hpp

#ifndef __SIM_CLASS_HEADER
#define __SIM_CLASS_HEADER

#include <grid_class.hpp>
#include <shift_region_class.hpp>

#include <random2_msk.hpp>
#include <timer2_msk.hpp>
#include <accum_double.hpp>

#include <map>
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <cmath>

//perimeter is documented in grid_class.hpp
namespace perimeter
{
    class sim_class {
        typedef typename grid_class::index_type index_type;
        typedef typename grid_class::site_type site_type;
    public:
        sim_class(std::map<std::string, double> param, std::string shift_file):   H_(param["-H"])
                                                        , L_(param["-L"])
                                                        , param_(param)
                                                        , grid_(H_, L_, {uint(param["-init0"]), uint(param["-init1"]), uint(param["-init2"]), uint(param["-init3"]), uint(param["-init4"])}) 
                                                        , rngS_() 
                                                        , rngH_(H_) 
                                                        , rngL_(L_) 
                                                        , p(param_["-p"]){
            std::cout << "Parameter" << std::endl;
            for(auto in = param_.begin(); in != param_.end(); ++in)
                std::cout << in->first << " = " << in->second << std::endl;
            
            shift_region_class sr_(shift_file);
            sr_.set_grow({qmc::right});
            sr_.grow(param_["-g"]);
            sr_.print(2);
            grid_.set_shift_region(sr_);
        }
        
        bool two_bond_update(index_type i, index_type j, state_type state) {
            site_type & target = grid_(i, j);
            
            state_type bra = state;
            if(bra >= qmc::n_bra) //it's a ket
                bra = qmc::invert_state - state; //now it's a bra
            
            bond_type const b = grid_.two_bond_update_site(target, state, bra);
            if(b != qmc::none) {
                
                bond_type & dir = target.bond[state];
                grid_.two_bond_flip(&target, target.neighbor[dir], b, state);
                return true;
            }
            return false;
        }
        
        void spin_update() {
            grid_.init_loops();
            state_type bra = 0;
            std::for_each(grid_.begin(), grid_.end(), 
                [&](site_type & s) {
                    if(s.check[bra] == false)
                    {
                        if(rngS_() > .5) {
                            grid_.follow_loop_tpl(&s, bra, 
                                [&](site_type * next) {
                                    next->check[bra] = true;
                                }
                            );
                        }
                        else {
                            grid_.follow_loop_tpl(&s, bra, 
                                [&](site_type * next) {
                                    next->check[bra] = true;
                                    next->spin[bra] = qmc::invert_spin - next->spin[bra];
                                    next->spin[qmc::invert_state - bra] = qmc::invert_spin - next->spin[qmc::invert_state - bra];
                                }
                            );
                        }
                    }
                }
            );
            grid_.clear_check();
        }
        
        void update() {
            for(state_type state = qmc::start_state; state < qmc::n_states; ++state)
                for(uint i = 0; i < H_ * L_; ++i)
                    two_bond_update(rngH_(), rngL_(), state);
            
            //~ spin_update();
        }
        
        void measure() {
            //~ data["swap_loops"] << grid_.n_swap_loops();
            //~ data["swap_overlap"] << pow(2.0, int(grid_.n_swap_loops()) - int(grid_.n_loops({qmc::bra, qmc::bra2})));
            data["loops"] << grid_.n_loops();
            //~ data["overlap"] << pow(2.0, int(grid_.n_loops()) - 2*H_*L_* .5 );
        }
        
        void run() {
            addon::timer_class<addon::data> timer(param_["-term"] + param_["-sim"]);
            timer.set_names("S_seed", "H_seed", "L_seed", "grow", "S2");
            timer.set_comment("test");
            
            for(uint i = 0; i < param_["-term"]; ++i) {
                update();
                timer.progress(i);
            }
            for(uint i = 0; i < param_["-sim"]; ++i) {
                update();
                measure();
                timer.progress(param_["-term"] + i);
            }
            timer.print(rngS_.seed(), rngH_.seed(), rngL_.seed(), param_["-g"], -std::log(data["swap_overlap"].mean()));
            timer.write(rngS_.seed(), rngH_.seed(), rngL_.seed(), param_["-g"], -std::log(data["swap_overlap"].mean()));
        }
        
        void present_data() {
            std::for_each(data.begin(), data.end(), 
                [&](std::pair<std::string const, accumulator_double> & p) {
                    std::cout << p.first << ": " << p.second << std::endl;
                }
            );
            std::cout << "S2 = " << -std::log(data["swap_overlap"].mean()) << std::endl;
        }
        
        grid_class & grid() {
            return grid_;
        }
    private:
        const uint H_;
        const uint L_;
        std::map<std::string, double> param_;
        grid_class grid_;
        addon::random_class<double, addon::mersenne> rngS_;
        addon::random_class<int, addon::mersenne> rngH_;
        addon::random_class<int, addon::mersenne> rngL_;
        std::map<std::string, accumulator_double> data;
        double const p;
    };
}
#endif //__SIM_CLASS_HEADER
