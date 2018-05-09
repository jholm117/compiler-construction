#include <L2.h>
#include <vector>
#include <string>

using namespace std;

namespace L2 {
  Instruction* createReadInstruction(string name, int offset) {
    Instruction* i = new Instruction();
    i->args = vector<L2_Item*>();
    i->args.push_back(new Variable(name));
    i->args.push_back(new Operator(ARROW));
    i->args.push_back(new Mem(new Register(rsp), new Number(offset)));
    i->type = ASSIGN;
    return i;
  }

  Instruction* createWriteInstruction(string name, int offset) {
    Instruction* i = new Instruction();
    i->args = vector<L2_Item*>();
    i->args.push_back(new Mem(new Register(rsp), new Number(offset)));
    i->args.push_back(new Operator(ARROW));
    i->args.push_back(new Variable(name));
    i->type = ASSIGN;
    return i;
  }

  Instruction* createSpilledInstruction(Instruction* instruction, Variable* v, string name, int offset) {
    Instruction* i = new Instruction();
    i->args = vector<L2_Item*>();
    for( auto item : instruction->args ) {
      if( item->equals(v) ){
        i->args.push_back(new Variable(name));

      } else if (item->item_type == MEMORY && ((Mem*)item)->address->equals(v)){
        ((Mem*)item)->address = new Variable(name);
        i->args.push_back(item);
      } else {
        i->args.push_back(item);
      }
    }
    i->type = instruction->type;
    return i;
  }

  void SPILLER_spill(Program p, Function* f, Variable* v, string s) {
    vector<Instruction*> newInstructions;
    int counter = 0;
    int offset = f->locals * 8;
    
    for ( auto i : f->instructions ) {
      string name = s + to_string(counter);
      OUR_SET genSet = ComputeGen(i);
      OUR_SET killSet = ComputeKill(i);
      bool containsVar = false; 
      if(genSet.find(v) != genSet.end()) {
        newInstructions.push_back(createReadInstruction(name, offset));
        containsVar = true;
      }

      newInstructions.push_back(createSpilledInstruction(i, v, name, offset));

      if(killSet.find(v) != killSet.end()) {
        newInstructions.push_back(createWriteInstruction(name, offset));
        containsVar = true;
      }
      if(containsVar) ++counter; 

    }
    if (counter > 0) f->locals++;
    f->instructions = newInstructions;
  }

}