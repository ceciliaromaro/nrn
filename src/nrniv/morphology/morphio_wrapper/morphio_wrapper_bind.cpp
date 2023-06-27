#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "morphio_wrapper.hpp"

namespace py = pybind11;

PYBIND11_MODULE(morphio_api, m) {
    py::class_<neuron::morphology::MorphIOWrapper>(m, "MorphIOWrapper")
        .def(py::init<const std::string&>())
        .def("morph_as_hoc", &neuron::morphology::MorphIOWrapper::morph_as_hoc)
        .def_property_readonly("sec_idx2names", &neuron::morphology::MorphIOWrapper::sec_idx2names)
        .def_property_readonly("sec_typeid_distrib", &neuron::morphology::MorphIOWrapper::sec_typeid_distrib);
}