#pragma once

#include <LB.h>

using namespace std;

namespace LB{
    void flatten_scopes(Program & p);

    void translate_control_structures(Program & p);
}