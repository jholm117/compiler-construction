#include <L2.h>
// #include <vector>
//#include <set>
//#include <unordered_map>

using namespace std;

namespace L2 { 
  void addItemToGraph(L2_Item* i, InterferenceGraph* g){
    g->emplace(i, OUR_SET());
  }

  void makeEdge(L2_Item* n1, L2_Item* n2, InterferenceGraph* g){
    if(!n1->equals(n2)){
      g->at(n1).insert(n2);
      g->at(n2).insert(n1);
    }
  }

  void connectTwoSets( OUR_SET s1, OUR_SET s2, InterferenceGraph* g){
    for(L2_Item* i1 : s1){
      for(L2_Item* i2 : s2){
        makeEdge(i1, i2, g);
      }
    }
  }

  void connectKillAndOut( OUR_SET kill, OUR_SET out, InterferenceGraph* g, Instruction* i ) {
    if(i->type != ASSIGN || i->args[2]->item_type != VAREXP ){
        connectTwoSets( kill, out, g );
    }
  }

  InterferenceGraph* initializeGraph(std::vector<OUR_SET> in_sets, std::vector<OUR_SET> kill_sets){
    InterferenceGraph* g = new InterferenceGraph();
    // Add all registers
    for(auto r : mapToRegisters(writeableRegisters)){
      addItemToGraph(r, g);
    }

    // Connect all registers
    for (auto n1 : (*g)){
      for (auto n2 : (*g)){
        makeEdge(n1.first, n2.first, g);
      }      
    }

    // Add variables to graph
    for(auto in : in_sets){
      for(auto item : in) {
        addItemToGraph(item, g);
      }
    }
    for(auto kill : kill_sets){
      for(auto item : kill){
        addItemToGraph(item, g);
      }
    }
    
    return g;
  }

  void addConstraints( Instruction* i, InterferenceGraph* g){
    if( i->type == SOP && i->args[2]->item_type == VAREXP ){
      for (auto r : mapToRegisters(nonRCXregisters)){
        makeEdge(i->args[2], r, g);
      }
    }
  }

  InterferenceGraph* computeInterferenceGraph( Function* f, DataFlowResult* dfr ){
    InterferenceGraph* g = initializeGraph(dfr->inSets, dfr->killSets);
    for( int i = 0; i < dfr->inSets.size(); ++i ){
      connectTwoSets(dfr->inSets[i], dfr->inSets[i], g);
      connectTwoSets(dfr->outSets[i], dfr->outSets[i], g);
      connectKillAndOut(dfr->killSets[i], dfr->outSets[i], g, f->instructions[i]);
      addConstraints(f->instructions[i], g);
    }
    
    return g;
  }
}