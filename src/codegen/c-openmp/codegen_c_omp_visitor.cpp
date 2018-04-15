#include "codegen/c-openmp/codegen_c_omp_visitor.hpp"


using namespace symtab;
using namespace fmt::literals;
using SymbolType = std::shared_ptr<symtab::Symbol>;


/****************************************************************************************/
/*                      Routines must be overloaded in backend                          */
/****************************************************************************************/



void CodegenCOmpVisitor::print_channel_iteration_task_begin(BlockType type) {
    std::string vars;
    if (type == BlockType::Equation) {
        vars = "start, end, node_index, indexes, voltage, vec_rhs, vec_d, inst, thread, nt";
    } else {
        vars = "start, end, node_index, indexes, voltage, inst, thread, nt";
    }
    printer->add_line("#pragma omp task default(shared) firstprivate({})"_format(vars));
    printer->add_line("{");
    printer->increase_indent();
}


void CodegenCOmpVisitor::print_channel_iteration_task_end() {
    printer->decrease_indent();
    printer->add_line("}");
}


/*
 * Depending on the backend, print loop for tiling channel iterations
 */
void CodegenCOmpVisitor::print_channel_iteration_tiling_block_begin(BlockType type) {
    printer->add_line("const int TILE = 3;");
    printer->start_block("for (int block = 0; block < nodecount;) ");
    printer->add_line("int start = block;");
    printer->add_line("block = (block+TILE) < nodecount ? (block+TILE) : nodecount;");
    printer->add_line("int end = block;");
    print_channel_iteration_task_begin(type);
}


/**
 * End of tiled channel iteration block
 */
void CodegenCOmpVisitor::print_channel_iteration_tiling_block_end() {
    print_channel_iteration_task_end();
    printer->end_block();
    printer->add_newline();
}


/**
 * Depending programming model and compiler, we print compiler hint
 * for parallelization. For example:
 *
 *      #pragma ivdep
 *      for(int id=0; id<nodecount; id++) {
 *
 *      #pragma acc parallel loop
 *      for(int id=0; id<nodecount; id++) {
 *
 */
void CodegenCOmpVisitor::print_channel_iteration_block_parallel_hint() {
    printer->add_line("#pragma omp simd");
}


void CodegenCOmpVisitor::print_atomic_reduction_pragma() {
    printer->add_line("#pragma omp atomic update");
}


void CodegenCOmpVisitor::print_backend_includes() {
    printer->add_line("#include <omp.h>");
}


std::string CodegenCOmpVisitor::backend_name() {
    return "C-OpenMP (api-compatibility)";
}


bool CodegenCOmpVisitor::channel_task_dependency_enabled() {
    return true;
}


bool CodegenCOmpVisitor::block_require_shadow_update(BlockType type) {
    return !(!channel_task_dependency_enabled() || type == BlockType::Initial);
}