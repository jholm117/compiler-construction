#pragma once

#include <LA.h>

namespace LA {
    void encode_decode(Program & p);
    void check_array_access(Program & p);
    void generate_basic_blocks(Program & p);
}