#include <my_utils.h>
#include <string>
#include <iostream>   

std::string chooseRegisterName(regType reg){
    switch(reg){
        case rdi: return "rdi";
        case rsi: return "rsi";
        case rcx: return "rcx";
        case rdx: return "rdx";
        case rbx: return "rbx";
        case rbp: return "rbp";
        case rax: return "rax";
        case rsp: return "rsp";
        case r8: return "r8";
        case r9: return "r9";
        case r10: return "r10";
        case r11: return "r11";
        case r12: return "r12";
        case r13: return "r13";
        case r14: return "r14";
        case r15: return "r15";
        default: return "invalid regType";
    }
}

regType chooseRegisterType(std::string str){
  if("rdi" == str) return rdi;
  if("rsi" == str) return rsi;
  if("rcx" == str) return rcx;
  if("rdx" == str) return rdx;
  if("rbx" == str) return rbx;
  if("rbp" == str) return rbp;
  if("rax" == str) return rax;
  if("rsp" == str) return rsp;
  if("r8" == str) return r8;
  if("r9" == str) return r9;
  if("r10" == str) return r10;
  if("r11" == str) return r11;
  if("r12" == str) return r12;
  if("r13" == str) return r13;
  if("r14" == str) return r14;
  if("r15" == str) return r15;

  std::cout << "invalid register name" << std::endl; 
  return regtype_error;
}

operatorType chooseOperatorType(std::string str){
    if(str == "++") return INC;
    if(str == "--") return DEC;
    if(str == "<") return LESS;
    if(str == "<=") return LESS_EQUAL;
    if(str == "=") return EQUAL;
    if(str == "<-") return ARROW;
    if(str == "+=") return PLUS;
    if(str == "-=") return MINUS;
    if(str == "*=") return TIMES;
    if(str == "&=") return AND;
    if(str == "<<=") return SHIFT_LEFT;
    if(str == ">>=") return SHIFT_RIGHT;
    if(str == "@") return AT;
    return OPERATOR_ERROR;
}

std::string opTypeToString(operatorType t){
    switch(t){
        case INC: return "++";
        case DEC: return "--";
        case LESS: return "<";
        case LESS_EQUAL: return "<=";
        case EQUAL: return "=";
        case ARROW: return "<-";
        case PLUS: return "+=";
        case MINUS: return "-=";
        case TIMES: return "*=";
        case AND: return "&=";
        case SHIFT_LEFT: return "<<=";
        case SHIFT_RIGHT: return ">>=";
        case AT: return "@";
        default: return "operatorType ERROR";
    }
}

int chooseNumArgs(instructionType t){
    switch (t){
      case ASSIGN:
      case AOP: 
      case SOP: return 3;
      case CALL_LOCAL:
      case CALL_RUNTIME:
      case INC_DEC: return 2;
      case CJUMP:
      case LEA: 
      case ASSIGN_CMP: return 5;
      case LABEL:
      case GOTO: return 1;
      case RETURN: return 0;
      default: 
        std::cout << "instructionType error" << std::endl;
        return -1;
    }
  }

std::string chooseInstructionPrefix (instructionType t){
    switch(t) {
        case CJUMP: return "cjump ";
        case CALL_RUNTIME: 
        case CALL_LOCAL: return "call ";
        case GOTO: return "goto ";
        case RETURN: return "return ";
        case I_TYPE_ERROR: return "I_TYPE_ERROR";
        default: return "";
    }
}