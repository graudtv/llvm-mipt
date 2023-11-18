#pragma once

#include "Instr.h"
#include <vector>

namespace qrisc {

struct SimulationOptions {
  size_t MemorySize = 4096;
  uintptr_t StackAddr = 2048;
};

void simulate(const std::vector<Instr> &Instrs,
              const SimulationOptions &SimOpts);

} // namespace qrisc
