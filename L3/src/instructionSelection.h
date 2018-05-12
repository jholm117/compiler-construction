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
}