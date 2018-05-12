#include <L3.h>
#include <instructionSelection.h>

using namespace std;

namespace L3 {
  
  Node* createInstructionNode(Instruction* i) {
    vector<Node*> itemNodes;  
    for( auto arg : i->arguments ) {
      itemNodes.push_back(new Node(arg));
    }
    // Has an assign as the root, assign[0] is what is assigned to, assign[1] is the value
    // if( dynamic_cast<Assign_I*>(i) ) {
    //   itemNodes[1]->children.push_back(itemNodes[0]);
    //   itemNodes[1]->children.push_back(itemNodes[2]);
    //   return itemNodes[1];
    // }
    // else if(dynamic_cast<Assign_Op_I*>(i)) {
    //   itemNodes[1]->children.push_back(itemNodes[0]);
    //   itemNodes[1]->children.push_back(itemNodes[3]);
    //   itemNodes[3]->children.push_back(itemNodes[2]);
    //   itemNodes[3]->children.push_back(itemNodes[4]);
    //   return itemNodes[1];
    // }
    // else if(dynamic_cast<Load_I*>(i)) {
    //   itemNodes[2]->children.push_back(itemNodes[3]);
    //   itemNodes[1]->children.push_back(itemNodes[0])
    //   itemNodes[1]->children.push_back(itemNodes[2])
    //   return itemNodes[1];
    // }
    // else if(dynamic_cast<Store_I*>(i)) {
    //   itemNodes[0]->children.push_back(itemNodes[1]);
    //   itemNodes[2]->children.push_back(itemNodes[0]);
    //   itemNodes[2]->children.push_back(itemNodes[3]);
    //   return itemNodes[2];
    // }
    

    // Rules:
    // All operations are children of the variable they are assigned to
    // An operation holds the 'result' of its operands
    // load and store are operations
    // Assign ("<-") is only used for direct assigns (var1 <- var2)
    // The root of a node is the variable being assigned to

    if( dynamic_cast<Assign_I*>(i) ) {
      itemNodes[0]->children.push_back(itemNodes[1]);
      itemNodes[1]->children.push_back(itemNodes[2]);
      return itemNodes[0];
    }
    else if(dynamic_cast<Assign_Op_I*>(i)) {
      itemNodes[0]->children.push_back(itemNodes[3]);
      itemNodes[3]->children.push_back(itemNodes[2]);
      itemNodes[3]->children.push_back(itemNodes[4]);
      return itemNodes[0];
    }
    else if(dynamic_cast<Load_I*>(i)) {
      itemNodes[0]->children.push_back(itemNodes[1]);
      itemNodes[2]->children.push_back(itemNodes[3]);
      return itemNodes[1];
    }
    else if(dynamic_cast<Store_I*>(i)) {
      itemNodes[0]->children.push_back(itemNodes[1]);
      itemNodes[2]->children.push_back(itemNodes[0]);
      itemNodes[2]->children.push_back(itemNodes[3]);
      return itemNodes[0];
    }
    return new Node();
  }

  vector<Node*> createNodesFromContext(vector<Instruction*> instructions) {
    vector<Node*> nodeList;
    for ( auto i : instructions ) {
      nodeList.push_back(createInstructionNode(i));
    }
    return nodeList;
  }

  vector<Node*> getNodeLeaves(Node* n) {
    if ( n->children.size() == 0 )
      return n;
    
    vector<Node*> leaves;
    for ( auto child : n->children ) {
      leaves.push_back(getNodeLeaves(child));
    }
    return leaves;
  }

  // input: a root node
  // output: the node that represents the value of the root
  Node* getNodeValue(Node* n) {
    auto store = new Operator();
    store->op = LOAD;
    auto assign = new Operator();
    assign->op = ARROW;
    if ( n->value->equals(load) ) {
      return n->children[1];
    }
    if ( n->children[0]->value->equals(assign) ) {
      return n->children[0]->children[0];
    }
    return n->children[0];
  }

  vector<Node*> mergeNodesInContext(vector<Node*> n) {
    // Algorithm:
    // Iterate through list of nodes, starting at first node
    // For each node: 
    //   Go through all previous nodes
    //   If any leaf of the current node is the root of a previous node, assign the leaf
    //   as the value (usually first child) of the previous node
    for ( int i = 0; i < n.size(); i++ ) {
      vector<Node*> leaves = getNodeLeaves(n[i]);
      for ( auto leaf : leaves ) {
        for ( int j = i - 1; j > 0; j-- ) {
          if ( n[j]->equals(leaf)) {
            leaf = getValue(n[j]);
          }
        }
      }  
    }
  }
}