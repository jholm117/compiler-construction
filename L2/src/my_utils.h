#include <string>
#include <vector>

enum regType{ 
  rdi, 
  rsi, 
  rcx, 
  rdx, 
  rbx, 
  rbp, 
  rax, 
  rsp, 
  r8, 
  r9, 
  r10, 
  r11, 
  r12, 
  r13, 
  r14, 
  r15, 
  regtype_error
 };

enum instructionType{ 
  AOP,
  SOP, 
  LEA,
  INC_DEC, 
  ASSIGN, 
  ASSIGN_CMP, 
  CJUMP, 
  CALL_RUNTIME, 
  CALL_LOCAL, 
  GOTO, 
  LABEL, 
  RETURN, 
  I_TYPE_ERROR
  };

enum operatorType { 
  INC, 
  DEC, 
  LESS, 
  LESS_EQUAL, 
  EQUAL, 
  ARROW, 
  PLUS, 
  MINUS, 
  TIMES, 
  AND, 
  SHIFT_LEFT, 
  SHIFT_RIGHT, 
  AT, 
  OPERATOR_ERROR
  };

enum itemType {
  CONSTANT,
  VAREXP,
  MEMORY,
  STACKARG
};

static std::vector<regType> calleeSaved {r12, r13, r14, r15, rbx, rbp};
static std::vector<regType> callerSaved {r8, r9, r10, r11, rax, rcx, rdi, rdx, rsi};
static std::vector<regType> arguments {rdi, rsi, rdx, rcx, r8, r9};
static std::vector<regType> writeableRegisters {
  rdi, 
  rsi, 
  rcx, 
  rdx, 
  rbx, 
  rbp, 
  rax, 
  r8, 
  r9, 
  r10, 
  r11, 
  r12, 
  r13, 
  r14, 
  r15, 
};
static std::vector<regType> nonRCXregisters {
  rdi, 
  rsi, 
  rdx, 
  rbx, 
  rbp, 
  rax, 
  r8, 
  r9, 
  r10, 
  r11, 
  r12, 
  r13, 
  r14, 
  r15, 
};

std::string chooseRegisterName(regType reg);
regType chooseRegisterType(std::string str);
operatorType chooseOperatorType(std::string str);
std::string opTypeToString(operatorType t);
int chooseNumArgs(instructionType t);
std::string chooseInstructionPrefix (instructionType t);