// NOTE: this assumes neuronapi.h is on your CPLUS_INCLUDE_PATH
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include "neuronapi.h"
#include "./test_common.h"

using std::cout;
using std::endl;
using std::ofstream;

constexpr std::array<double, 6> EXPECTED_V{
#ifndef CORENEURON_ENABLED
    -0x1.04p+6,
    -0x1.085a63d029bc3p+6,
    -0x1.112a5e95eb67cp+6,
    -0x1.1795abaec26c1p+6,
    -0x1.0422351f3f9dcp+6,
    -0x1.03e5317ac368cp+6,
#else
    -0x1.04p+6,
    -0x1.085a703d657a7p+6,
    -0x1.112d0039e9c38p+6,
    -0x1.17974aa201b7bp+6,
    -0x1.041fdf57a182bp+6,
    -0x1.03e58fad20b92p+6,
#endif
};

int main(void) {
    static std::array<const char*, 4> argv = {"netcon", "-nogui", "-nopython", nullptr};
    nrn_init(3, argv.data());

    // load the stdrun library
    nrn_function_call_s("load_file", "stdrun.hoc");

    // topology
    auto soma = nrn_section_new("soma");

    // ion channels
    nrn_mechanism_insert(soma, nrn_symbol("hh"));

    // NetStim
    auto ns = nrn_object_new(nrn_symbol("NetStim"), 0);
    nrn_property_set(ns, "start", 5);
    nrn_property_set(ns, "noise", 1);
    nrn_property_set(ns, "interval", 5);
    nrn_property_set(ns, "number", 10);

    nrn_double_push(1);
    nrn_double_push(2);
    nrn_double_push(3);
    nrn_method_call(ns, nrn_method_symbol(ns, "noiseFromRandom123"), 3);

    // syn = h.ExpSyn(soma(0.5))
    nrn_double_push(0.5);
    auto syn = nrn_object_new(nrn_symbol("ExpSyn"), 1);
    nrn_property_set(syn, "tau", 3);  // 3 ms timeconstant
    nrn_property_set(syn, "e", 0);    // 0 mV reversal potential (excitatory synapse)

    // nc = h.NetCon(ns, syn)
    nrn_object_push(ns);
    nrn_object_push(syn);
    auto nc = nrn_object_new(nrn_symbol("NetCon"), 2);
    nrn_property_array_set(nc, "weight", 0, 0.5);
    nrn_property_set(nc, "delay", 0);

    auto vec = nrn_object_new(nrn_symbol("Vector"), 0);

    // nc.record(vec)
    nrn_object_push(vec);
    nrn_method_call(nc, nrn_method_symbol(nc, "record"), 1);
    // TODO: record probably put something on the stack that should be removed

    // setup recording
    Object* v = nrn_object_new(nrn_symbol("Vector"), 0);
    nrn_rangevar_push(nrn_symbol("v"), soma, 0.5);
    nrn_double_push(20);
    nrn_method_call(v, nrn_method_symbol(v, "record"), 2);
    nrn_object_unref(nrn_object_pop());  // record returns the vector

    nrn_function_call_d("finitialize", -65);

    nrn_function_call_d("continuerun", 100.5);

    if (!approximate(EXPECTED_V, v)) {
        return 1;
    }
}
