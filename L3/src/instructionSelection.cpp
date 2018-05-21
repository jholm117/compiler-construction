#include <instructionSelection.h>
#include <algorithm>

using namespace std;

namespace L3 {

  template< typename TO, typename FROM, class UnaryOperator  >
  vector<TO> map(vector<FROM> & from_vec, vector<TO> & to_vec, UnaryOperator & op){
    to_vec.resize(from_vec.size());
    std::transform(from_vec.begin(), from_vec.end(), to_vec.begin(), op );
    return to_vec;
  }

  Node* mapToNode(L3_Item* a){
    return new Node(a);
  }
  
  Node* createInstructionNode(Contextual_I* i) {
    vector<Node*> itemNodes;  
    for( auto arg : i->args ) {
      itemNodes.push_back(new Node(arg));
    }
    vector<Node*> nodes;
    nodes = map(i->args, nodes, mapToNode); 
    
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
    else if(dynamic_cast<Assign_Op_I*>(i) || dynamic_cast<Assign_Cmp_I*>(i) ) {
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

  bool isVariable(Node * n){
    return dynamic_cast<Variable*>(n->value);
  }

  bool hasChildren(int number, Node * n){
    return n->children.size() == number;
  }

  bool isOperator(Node * n){
    return dynamic_cast<Operator*>(n->value);
  }

  bool isSOAP(Node * n){
    return dynamic_cast<SOAP*>(n->value);
  }
  
  bool isSource(Node * n){
    return !isOperator(n);
  }

  bool isCMP(Node * n){
    return dynamic_cast<CMP*>(n->value);
  }

  bool isLoad(Node * n){
    return isOperator(n) && ((Operator*)n->value)->op == LOAD;
  }

  bool isStore(Node * n){
    return isOperator(n) && ((Operator*)n->value)->op == STORE; 
  }

  bool hasOneChild(Node * n){
    return hasChildren(1, n);
  }

  bool hasTwoChildren(Node * n){
    return hasChildren(2, n);
  }

  Node* firstChild(Node * n){
    return n->children.front();
  }

  Node* secondChild(Node * n){
    return n->children[1];
  }

  Node* firstGrandChild(Node * n){
    return firstChild(firstChild(n));
  }

  Node* secondGrandChild(Node * n){
    return secondChild(firstChild(n));
  }

  struct L2_Assign : L2_Tile {
    string src; 
    string dest;
    L2_Assign(Node * t){
        dest = t->value->toString();
        src = firstChild(t)->value->toString();
    }
    static bool match(L3::Node * t){
      return isVariable(t) && hasChildren(1, t) && isSource(firstChild(t));
    }

    string toString() override {
      return i_line( dest + " <- " + src );
    }
  };

  L3_OperatorType toL2Op(CMP * o){
    switch(o->op){
      case GREATER_THAN: return LESS_THAN;
      case GREATER_THAN_EQ: return LESS_THAN_EQ;
      default: return o->op;
    }
  }

  struct L2_CMP_Assign : L2_Tile {
    string dest;
    string src1;
    string op;
    string src2;
    L2_CMP_Assign(Node * n){
      dest = n->value->toString();
      auto opNode = firstChild(n);
      auto opper = (CMP*)(opNode->value);
      auto opType = toL2Op(opper);
      if(opType == opper->op){
        src1 = firstChild(opNode)->value->toString();
        src2 = secondChild(opNode)->value->toString();
      } else {
        opper->op = opType;
        src2 = firstChild(opNode)->value->toString();
        src1 = secondChild(opNode)->value->toString();
      }
        this->op = opper->toString();

    }
    static bool match (Node * n){
      return isVariable(n) && hasChildren(1, n) && isCMP(firstChild(n)) && hasChildren(2, firstChild(n)) && 
        isSource(firstGrandChild(n)) && isSource(secondChild(firstChild(n))) ;
    }
    string toString() override {
      return i_line(dest + " <- " + src1 + " " + this->op + " " + src2);
    }
  };

  struct L2_Memory_Load : L2_Tile {
    string dst;
    string addr;
    L2_Memory_Load(Node * n){
      dst = n->value->toString();
      addr = firstChild(firstChild(n))->value->toString();
    }
    static bool match(Node * n){
      return isVariable(n) && hasChildren(1,n) && isLoad(firstChild(n)) 
      && hasChildren(1, firstChild(n)) && isVariable(firstChild(firstChild(n)));
    }
    string toString() override {
      return i_line(dst + " <- mem " + addr + " 0");
    }
  };

  struct L2_Memory_Store : L2_Tile {
    string addr;
    string src;
    L2_Memory_Store(Node * n){
      addr = firstChild(n)->value->toString();
      src = secondChild(n)->value->toString();
    }
    static bool match(Node * n){
      return isStore(n) && hasChildren(2, n) && isVariable(firstChild(n)) && isSource(secondChild(n));
    }
    string toString() override {
      return i_line("mem " + addr + " 0 <- " + src);
    }
  };

  struct L2_SOAP_EQUALS : L2_Tile {
    string dst;
    string op;
    string src;
    L2_SOAP_EQUALS(Node * n){
      dst = n->value->toString();
      op = firstChild(n)->value->toString();
      src = firstGrandChild(n)->equals(n) ? secondGrandChild(n)->value->toString() : firstGrandChild(n)->value->toString();
    }
    static bool match(Node * n){
      return isVariable(n) && hasChildren(1, n) && isSOAP(firstChild(n)) && hasChildren(2, firstChild(n))
      && ((firstGrandChild(n)->equals(n) && isSource(secondGrandChild(n))) || 
      (isSource(firstGrandChild(n)) && secondGrandChild(n)->equals(n)));
    }
    string toString() override {
      return i_line(dst + " " + op + " " + src);
    }
  };

  struct L2_SOAP_TwoStep : L2_Tile {
    string dest;
    string src1;
    string op;
    string src2;
    L2_SOAP_TwoStep(Node * n){
      dest = n->value->toString();
      op = firstChild(n)->value->toString();
      src1 = firstGrandChild(n)->value->toString();
      src2 = secondGrandChild(n)->value->toString();
    }
    static bool match(Node * n){
      return isVariable(n) && hasOneChild(n) && isSOAP(firstChild(n)) && hasTwoChildren(firstChild(n)) && isSource(firstGrandChild(n)) && isSource(secondGrandChild(n));
    }
    string toString() override {
      auto s = i_line(dest + " <- " + src1);
      s += i_line( dest + " " + op + " " + src2); 
      return s;
    }
  };

  L2_Tile* mapToTile(Node * n){
    if( L2_CMP_Assign::match(n) ) return new L2_CMP_Assign(n);
    if ( L2_Memory_Load::match(n) ) return new L2_Memory_Load(n);
    if( L2_Memory_Store::match(n) ) return new L2_Memory_Store(n);
    if( L2_SOAP_EQUALS::match(n) ) return new L2_SOAP_EQUALS(n);
    if( L2_SOAP_TwoStep::match(n) ) return new L2_SOAP_TwoStep(n);
    if( L2_Assign::match(n) ) return new L2_Assign(n);
    cout << "NULL" << endl;
    return nullptr;
  }

  std::string transformInstruction(Contextual_I* instruction){
    auto node = createInstructionNode(instruction);
    auto tile = mapToTile(node);
    string s;
    s += tile->toString();
    return s;

  }
}