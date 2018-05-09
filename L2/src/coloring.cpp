#include <L2.h>
#include <vector>
#include <algorithm>

using namespace std;

namespace L2 {

  void printIGraph(InterferenceGraph* i) {
    for (auto p : *i){
        std::string n1 = p.first->toString();
        cout << n1 << " ";
        for (auto n2 : p.second){
          cout << n2->toString() << " ";
        }
        cout << std::endl;
      }
  }

  bool shouldReplaceChoice(int currentChoiceConnections, int newChoiceConnections) {
    if (currentChoiceConnections == -1) return true;
    if (currentChoiceConnections >= 15) {
      if (newChoiceConnections >= 15) {
        return newChoiceConnections > currentChoiceConnections;
      }
      return true;
    }
    if( newChoiceConnections >= 15) {
      return false;
    }
    return newChoiceConnections > currentChoiceConnections;

    // return currentChoiceConnections >= 15 ? (newChoiceConnections >= 15 ? newChoiceConnections > currentChoiceConnections : true) : (newChoiceConnections >= 15 ? false : newChoiceConnections > currentChoiceConnections);
  }

  L2_Item* chooseNodeToRemove(InterferenceGraph* i) {
    L2_Item* currentChoice;
    int currentChoiceConnections = -1;
    for ( auto kvpair : *i ) {
      if( !dynamic_cast<Variable*>(kvpair.first) ) continue;

      int itemConnections = kvpair.second.size();
      if ( shouldReplaceChoice(currentChoiceConnections, itemConnections) ) {
        currentChoice = kvpair.first;
        currentChoiceConnections = itemConnections;
      }
    }
    return currentChoice;
  }

  // Don't remove connections from items that have already been 'removed' from the graph
  // removeConnections 'removes' a node from an IG by removing all edges to it from nodes that are still on the graph
  void removeNode(L2_Item* item, InterferenceGraph* i) {
    for( auto kvpair : *i ) {
      if ( kvpair.second.erase(item) ) {
        (*i)[kvpair.first] = kvpair.second;
      }
    }
  }

  // addNode 'puts a node back in the graph', by completing the connection between its edges and other nodes
  void addNode(pair<L2_Item*, OUR_SET> item, InterferenceGraph* i) {
    i->insert(item);
    // for( auto missingEdgeItem : i->at(item) ) {
    //   i->at(missingEdgeItem).insert(item);
    // }
  }

  vector<pair<L2_Item*, OUR_SET>> createNodeStack(InterferenceGraph* i) {
    vector<pair<L2_Item*, OUR_SET>> orderedStack;

    while( i->size() > 15 ) {
      L2_Item* item = chooseNodeToRemove(i);
      orderedStack.push_back(pair<L2_Item*, OUR_SET>(item, i->at(item)));
      i->erase(item);
      removeNode(item, i);
    }

    return orderedStack;
  }

  regType chooseColor(OUR_SET edges, ColoredGraph* c, InterferenceGraph* i) {
    set<regType> availableColors;


    for( auto reg : writeableRegisters ) {
      availableColors.insert(reg);
    }

    for( auto edge : edges ) {
      if( dynamic_cast<Register*>(edge) ) {
        availableColors.erase(((Register*)edge)->name);
      }
      if( dynamic_cast<Variable*>(edge) ) {
        availableColors.erase(c->at(edge));
      }
    }

    for( auto reg : callerSaved ) {
      if( availableColors.find(reg) != availableColors.end() ) return reg;
    }

    for( auto reg : calleeSaved ) {
      if( availableColors.find(reg) != availableColors.end() ) return reg;
    }

    // uncolorable
    return regtype_error;

  }


  ColoredGraph* colorGraph(vector<pair<L2_Item*, OUR_SET>> stack, InterferenceGraph* i) {
    ColoredGraph* cgraph = new ColoredGraph();
    while( stack.size() > 0 ) {
      pair<L2_Item*, OUR_SET> currentItem = stack.back();
      stack.pop_back();
      regType color = chooseColor(currentItem.second, cgraph, i);
      cgraph->emplace(currentItem.first, color);
      addNode(currentItem, i);
    }
    
    return cgraph;
  }

  ColoredGraph* computeColoredGraph(InterferenceGraph* i) {
    vector<pair<L2_Item*, OUR_SET>> stack = createNodeStack(i);
    return colorGraph(stack, i);
  }
}