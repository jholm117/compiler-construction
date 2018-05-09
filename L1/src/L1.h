#pragma once

#include <vector>
#include <iostream>
#include <algorithm>

namespace L1 {

  struct L1_item {
    std::string labelName;
  };

  class Instruction {
    public:
      bool isLabel = false;
      bool isRet = false;
      virtual std::string gen_asm(){
        return "you messed up";
      };
  };

  class Label : public Instruction {
    public:
      std::string name;
      Label(std::string n){
        name = "_" + n.substr(1) + ":";
        isLabel = true;
      }
      std::string gen_asm() {
        return name;
      };
  };

  class Operation : public Instruction {
    public:
      L1_item src;
      L1_item dest;
      explicit Operation(L1_item d, L1_item s){
        this->src = s;
        this->dest = d;
      }
      std::string gen_asm(){
        return this->src.labelName + ", " + this->dest.labelName;
      };      
  };

  class Dec : public Instruction {
    public:
      std::string dest;
      Dec(L1_item d){
        dest = d.labelName;
      }
      std::string gen_asm(){
        return "dec " + dest;
      };
  };

  class Inc : public Instruction {
    public:
      std::string dest;
      Inc(L1_item d){
        dest = d.labelName;
      }
      std::string gen_asm(){
        return "inc " + dest;
      };
  };

  class Assignment : public Operation {
    public:
      using Operation::Operation;
      std::string gen_asm(){
        return "movq "+ Operation::gen_asm();
      };
  };

  class AOP : public Operation {
    public:
      std::string math_i;
      AOP(L1_item d, L1_item s, std::string op) : Operation (d, s){
        math_i = chooseMath_i(op);
        if(math_i == "salq" || math_i == "sarq")
          if(s.labelName == "%rcx")
            this->src.labelName = "%cl";
      }
      std::string gen_asm(){
        return math_i + " " + Operation::gen_asm(); 
      };
    private:
      std::string chooseMath_i(std::string op){
        switch(op[0]){
          case '+':
            return "addq";
          case '-':
            return "subq";
          case '*':
            return "imulq";
          case '&':
            return "andq";
          case '<':
            return "salq";
          case '>':
            return "sarq";
          default:
            std::cout << op << std::endl;
            return "uh OH you messed up";
        }
      };
  };

  class LEA : public Instruction {
    public:
      std::string dest, reg1, reg2, num;
      LEA(L1_item d, L1_item r1, L1_item r2, L1_item e){
        dest = d.labelName;
        reg1 = r1.labelName;
        reg2 = r2.labelName;
        num = e.labelName;
      }
      std::string gen_asm(){
        return "lea (" + reg1 + ", " + reg2 + ", " + num + "), " + dest;
      }
  };

  class GOTO : public Instruction {
    public:
      std::string labbel;
      GOTO(L1_item l){
        labbel = l.labelName;
      }
      std::string gen_asm(){
        return "jmp " + labbel.substr(1);
      };
  };



  class CallRuntime : public Instruction {
    public:
      std::string i;
      CallRuntime(std::string str){
        std::replace(str.begin(), str.end(), '-','_');
        int64_t index = str.find_last_of(' ');
        i = str.substr(0,index);
      }
      std::string gen_asm(){
        return i;
      };
  };

  class Ret : public Instruction {
    public:
      Ret(){
        isRet = true;
      }
      std::string gen_asm(){
        return "retq";
      };
  };

  class CallFunction : public Instruction {
    public:
      std::string calleeLabel;
      int64_t offset;
      CallFunction(L1_item callee, L1_item args) {
        calleeLabel = fixedCalleeLabel(callee.labelName);
        offset = 8 + 8 * (std::max(std::stoi(args.labelName.substr(1)), 6) - 6);
      }
      std::string gen_asm(){
        std::string assembly;
        assembly = "subq $" + std::to_string(offset) + ", %rsp\n";
        assembly += "  jmp " + calleeLabel;
        return assembly;
      }  
    private:
      std::string fixedCalleeLabel(std::string callee){
        if(callee[0] == '%')
          return "*" + callee;
        return callee.substr(1);
      }
  };

  class Conditional {
    public:
      static std::string chooseSwappedCmp_i(std::string op){
        if(op == "<=")
          return "ge";
        if(op == "<")
          return "g";
        if(op == "=")
          return "e";
        return "you messed the heck up";
      };

      static std::string chooseCmp_i(std::string op) {
        if(op == "=")
          return "e";
        if(op == "<=")
          return "le";
        if(op == "<")
          return "l";
      };
  };

  class Cmp : public Operation {
    public:
      L1_item src2;
      std::string cmp_i;
      Cmp(L1_item d, L1_item s, L1_item s2, std::string op) : Operation (d, s) {
        src2 = s2;
        if(src.labelName[0] == '$'){
          swapSrc();
          this->cmp_i = Conditional::chooseSwappedCmp_i(op);
        }
        else {
          this->cmp_i = Conditional::chooseCmp_i(op);
        }
      }

      std::string gen_asm(){
        std::string assembly;
        assembly = "cmpq " + src2.labelName + ", " + src.labelName + "\n";
        assembly += "  set" + cmp_i + " " + regTo8(this->dest.labelName) + "\n";
        assembly += "  movzbq " + regTo8(this->dest.labelName) + ", " + this->dest.labelName;
        return assembly;
      };
    private:
      void swapSrc(){
        L1_item temp = src;
        src = src2;
        src2 = temp;
      };

      std::string regTo8(std::string reg){
        if(reg == "%r8") return "%r8b";
        if(reg == "%r9") return "%r9b";
        if(reg == "%r10") return "%r10b";
        if(reg == "%r11") return "%r11b";
        if(reg == "%r12") return "%r12b";
        if(reg == "%r13") return "%r13b";
        if(reg == "%r14") return "%r14b";
        if(reg == "%r15") return "%r15b";
        if(reg == "%rax") return "%al";
        if(reg == "%rbp") return "%bpl";
        if(reg == "%rbx") return "%bl";
        if(reg == "%rcx") return "%cl";
        if(reg == "%rdx") return "%dl";
        if(reg == "%rdi") return "%dil";
        if(reg == "%rsi") return "%sil";
        std::cout <<  "you're a heck up regTo8" << std::endl;
      };
  };

  class CmpNumbers : public Instruction {
    public:
      std::string dest, result;
      CmpNumbers(L1_item d, L1_item n1, L1_item op, L1_item n2){
        dest = d.labelName;
        result = calculateResult(std::stoi(n1.labelName),op.labelName,std::stoi(n2.labelName));
      }
      std::string gen_asm(){
        return "movq $" + result + ", " + dest;
      }
    private:
      std::string calculateResult(int num1, std::string op, int num2){
        bool answer = false;
        if(op == "=") answer = num1 == num2;
        else if(op == "<=") answer = num1 <= num2;
        else if(op == "<") answer = num1 < num2;
        else std::cout << "something went wrong in calculateResult" << std::endl;
        return answer ? "1" : "0";
      };
  };

  class CJump : public Instruction {
    public:
      L1_item comp1, comp2, trueLabel, falseLabel;
      std::string op;
      CJump(L1_item c1, L1_item c2, L1_item tLabel, L1_item fLabel, L1_item o) {
        comp1 = c1;
        comp2 = c2;
        trueLabel = tLabel;
        falseLabel = fLabel;
        if(comp1.labelName[0] == '$'){
          swapComp();
          op = Conditional::chooseSwappedCmp_i(o.labelName);
        }
        else 
          op = Conditional::chooseCmp_i(o.labelName);
      }
      std::string gen_asm() {
        std::string assembly;
        assembly = "cmpq " + comp2.labelName + ", " + comp1.labelName + "\n";
        assembly += "  j" + op + " " + trueLabel.labelName.substr(1) + "\n";
        assembly += "  jmp " + falseLabel.labelName.substr(1);
        return assembly;
      }

    private:
      void swapComp(){
        L1_item temp = comp1;
        comp1 = comp2;
        comp2 = temp;
      };
  };

  class CJumpNum : public Instruction {
    public:
      int64_t num1, num2;
      std::string trueLabel, falseLabel, op;
      CJumpNum(L1_item n1, L1_item n2, L1_item tLabel, L1_item fLabel, L1_item o) {
        num1 = std::stoi(n1.labelName);
        num2 = std::stoi(n2.labelName);
        trueLabel = tLabel.labelName.substr(1);
        falseLabel = fLabel.labelName.substr(1);
        op = o.labelName;
      }
      std::string gen_asm() {
        return "jmp " + (analyzeComp() ? trueLabel : falseLabel);
      }
    private:
      bool analyzeComp() {
        if(op == "<")
          return num1 < num2;
        if(op == "<=")
          return num1 <= num2;
        if(op == "=")
          return num1 == num2;
        std::cout << "you messed up again dummy" << std::endl;
      }
  };


  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<L1::Instruction *> instructions;
  };

  struct Program{
    std::string entryPointLabel;
    std::vector<L1::Function *> functions;
  };
} // L1
