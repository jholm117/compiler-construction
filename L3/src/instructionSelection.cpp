#include <L3.h>
#include <instructionSelection.h>
#include <algorithm>

using namespace std;

namespace L3 {
  
  Node* createInstructionNode(Contextual_I* i) {
    vector<Node*> itemNodes;  
    for( auto arg : i->args ) {
      itemNodes.push_back(new Node(arg));
    }
    
    // Rules:
    // All operations are children of the variable they are assigned to
    // An operation holds the 'result' of its operands (its children)
    // load and store are operations
    // Assign ("<-") is an operation, and is only used for direct assigns (var1 <- var2)
    // The root of a node is the variable being assigned to

    if( dynamic_cast<Assign_I*>(i) ) {
      // var var
      itemNodes[0]->children.push_back(itemNodes[1]);
      itemNodes[0]->isRoot = true;
      return itemNodes[0];
    }
    else if(dynamic_cast<Assign_Op_I*>(i)) {
      // var var op var
      itemNodes[0]->children.push_back(itemNodes[2]);
      itemNodes[2]->children.push_back(itemNodes[1]);
      itemNodes[2]->children.push_back(itemNodes[3]);
      itemNodes[0]->isRoot = true;
      return itemNodes[0];
    }
    else if(dynamic_cast<Load_I*>(i)) {
      // var var
      auto loadNode = new Node(new Operator(LOAD));
      itemNodes[0]->children.push_back(loadNode);
      loadNode->children.push_back(itemNodes[1]);
      itemNodes[0]->isRoot = true;
      return itemNodes[0];
    }
    else if(dynamic_cast<Store_I*>(i)) {
      // var var
      auto storeNode = new Node(new Operator(STORE));
      storeNode->children.push_back(itemNodes[0]);
      storeNode->children.push_back(itemNodes[1]);
      storeNode->isRoot = true;
      return storeNode;
    }
    return new Node();
  }

  vector<Node*> createNodesFromContext(vector<Contextual_I*> & instructions) {
    vector<Node*> nodeList;
    nodeList.resize(instructions.size());
    std::transform(instructions.begin(), instructions.end(), nodeList.begin(), createInstructionNode);

    return nodeList;
  }

  vector<Node*> getNodeLeaves(Node* n) {
    vector<Node*> leaves;
    if ( n->children.empty() ) {
      leaves.push_back(n);
      return leaves;
    }
  
    for ( auto child : n->children ) {
      auto childLeaves = getNodeLeaves(child);
      leaves.insert(leaves.end(), childLeaves.begin(), childLeaves.end());
    }
    return leaves;
  }

  void mergeNodesInContext(vector<Node*>& v) {
    // Algorithm:
    // Iterate through list of nodes, starting at first node
    // For each node: 
    //   Go through all previous nodes
    //   If any leaf of the current node is the root of a previous node, assign the leaf
    //   as the value (usually first child) of the previous node
    for ( int i = 0; i < v.size(); i++ ) {
      vector<Node*> leaves = getNodeLeaves(v[i]);
      for ( auto leaf : leaves ) {
        for ( int j = i - 1; j > 0; j-- ) {
          if ( leaf->equals(v[j])) {
            leaf = v[j];
            v[j]->isRoot = false;
            break;
          }
        }
      }
    }
  }

  // TiledTree tileTree(Node* n){

  // }

  std::string transformContext(vector<Contextual_I*> & context){
    auto nodes = createNodesFromContext(context);
    mergeNodesInContext(nodes);
    // filter out non roots
    // map to tiled trees
    // foreach generate code

  }
}