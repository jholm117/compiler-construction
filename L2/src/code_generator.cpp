#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <climits>


#include <code_generator.h>
#include <L2.h>

using namespace std;

namespace L2{

  void replaceL2Variables(Function* f, Variable* v, regType r) {
    vector<Instruction*> newInstructions;
    for ( auto instruction : f->instructions ) {
      Instruction* i = new Instruction();
      for( auto arg : instruction->args ) {
        if( arg->equals(v) ){
          i->args.push_back(new Register(r));

        } else if( arg->item_type == MEMORY && ((Mem*)arg)->address->equals(v) ) {
          i->args.push_back(new Mem(new Register(r), ((Mem*)arg)->num));
        } else{
          i->args.push_back(arg);
        }
      }
      i->type = instruction->type;
      newInstructions.push_back(i);
    }
    f->instructions = newInstructions;
  }

  void generate_code(Program p){

    for( auto f : p.functions ) {
      vector<pair<L2_Item*, regType>> colored;
      
      bool needToSpill;
      int prevVarsToSpill = INT_MAX;
      do {
        colored.clear();
        needToSpill = false;
        // cout << "gen code" << f->name << endl;
        auto liveness = L2::computeLivenessAnalysis(p, f);
        // cout << "liveness" << endl;
        auto ig = L2::computeInterferenceGraph(f, liveness);
        // cout << "intereference" << endl;
        auto cg = L2::computeColoredGraph(ig);
        // cout << "colored graph" << endl;
        int newVarsToSpill = 0;

        int spillSuffix = 0;        
        for( auto kvpair : *cg ) {
          if( kvpair.second == regtype_error ) {
            SPILLER_spill(p, f, (Variable*)(kvpair.first), "S" + to_string(spillSuffix++));
            
            newVarsToSpill++;
            needToSpill = true;
          } else {
            // replaceL2Variables(f, (Variable*)kvpair.first, kvpair.second);
            colored.push_back(kvpair);
          }
        }
        if(newVarsToSpill >= prevVarsToSpill){
          for(auto kvpair : colored ) SPILLER_spill(p, f, (Variable*)(kvpair.first), "S" + to_string(spillSuffix++));
          // colored = vector<pair<L2_Item*, regType>> ();
        }

        prevVarsToSpill = newVarsToSpill;

        delete liveness;
        delete ig;
        delete cg;
      } while( needToSpill );
      for( auto kvpair : colored ){
        replaceL2Variables(f, (Variable*)kvpair.first, kvpair.second);
      }
    }

    std::ofstream outputFile;
    outputFile.open("prog.L1");
    outputFile << "(" << p.entryPointLabel << endl;
    for( auto f : p.functions ) {
      outputFile << f->toString();
    } 
    outputFile << endl << ")" << endl;
    outputFile.close();
    return;
  }
}