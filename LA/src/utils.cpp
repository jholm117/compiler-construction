#include <utils.h>

namespace utils{
    int64_t VARIABLE_COUNT = 0;
    const string BASE_VARIABLE_NAME = "%hola_";
    const string BASE_LABEL_NAME = ":kathier_";

    string new_var_name(){
        return BASE_VARIABLE_NAME + std::to_string(VARIABLE_COUNT++);
    }

    string new_label_name(){
        return BASE_LABEL_NAME + std::to_string(VARIABLE_COUNT++);
    }
}


