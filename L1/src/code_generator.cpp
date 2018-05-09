#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

#define print_i(i) outputFile << "  " << i << "\n"
#define print_l(l) outputFile << l << "\n"

using namespace std;
std::ofstream outputFile;

namespace L1{
    

  std::string convertLabel(std::string name){
    return "_"+name.substr(1) + ":";
  }

  void printLabel(std::string name){
    print_l(convertLabel(name));
  }

  void allocateStack(int64_t locals){
    if(locals>0){
      print_i("subq $" + to_string(locals*8) + ", %rsp");
    }
  }

  void translateInstructions(vector<Instruction*> instructions, string shrink_stack_i){
    for(vector<Instruction*>::iterator it = instructions.begin(); it != instructions.end(); ++it){
      string str = (*it)->gen_asm();
      if((*it)->isRet) print_i(shrink_stack_i);
      (*it)->isLabel ? print_l(str) : print_i(str);
    }
  }

  string deallocateStack(int args, int64_t locals){
    int64_t num = (locals + std::max(6,args) - 6) *8;
    return "addq $" + to_string(num) + ", %rsp";
  }

  void generateL1Fun(Function* f){
    printLabel(f->name);
    allocateStack(f->locals);
    translateInstructions(f->instructions,deallocateStack(f->arguments,f->locals));
    ;
  }
  

  void translate(Program p){
    for(std::vector<Function*>::iterator it = p.functions.begin(); it != p.functions.end(); ++it){
      generateL1Fun(*it);
    }
  }

  void generate_code(Program p){

    /* 
     * Open the output file.
     */ 

    std::string entry = "_" + p.entryPointLabel.substr(1);
    outputFile.open("prog.S");
    
    /* 
     * Generate target code
     */ 
    print_l(".text");
    print_i(".globl go");
    print_l("go:");
    print_i("pushq %rbx");
    print_i("pushq %rbp");
    print_i("pushq %r12");
    print_i("pushq %r13");     
    print_i("pushq %r14");
    print_i("pushq %r15");
    print_i("call " + entry);
    print_i("popq %r15");
    print_i("popq %r14");
    print_i("popq %r13");
    print_i("popq %r12");
    print_i("popq %rbp");
    print_i("popq %rbx");
    print_i("retq");
    translate(p);

    /* 
     * Close the output file.
     */ 
    outputFile.close();
   
    return ;
  }
}
