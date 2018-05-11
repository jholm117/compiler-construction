#pragma once

#include <string>
#include <vector>

static std::string i_line(std::string str){
    return "\t\t" + str + "\n";
}

static std::string f_line(std::string str){
    return "\t" + str + "\n";
}

static std::vector<std::string> argumentRegisters {
    "rdi",
    "rsi",
    "rdx",
    "rcx",
    "r8",
    "r9"
};