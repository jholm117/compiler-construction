#include <L3.h>

namespace L3 {

  struct Node {
    L3_Item* value;
    vector<Node*> children;
    bool isRoot;
    Node() {
      isRoot = false;
    };
    Node(L3_Item* v) {
      value = v;
      isRoot = false;
    }
    bool equals(Node* n) {
      return this->value->equals( n->value );
    }
  };

  std::string transformContext(vector<Contextual_I*> & v);
  std::string transformInstruction(Contextual_I* i);
  struct L2_Tile {
    virtual string toString() = 0;
  };
}