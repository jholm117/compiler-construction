#include <L3.h>

namespace L3 {

  struct Node {
    L3_Item* value;
    vector<Node*> children;
    Node() {};
    Node(L3_Item* v) {
      value = v
    }
    bool equals(Node* n) {
      return this.value->toString() == n->value->toString();
    }
  }
}