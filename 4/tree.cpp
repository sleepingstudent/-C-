#include "tree.h"

#include <cassert>

using NBinTree::TNode;

int main() {
    auto node = TNode<int>::CreateLeaf(1);

    assert(!node->GetLeft());
    assert(!node->GetRight());
    assert(node->GetParent() == nullptr);

    const auto node2 = node;
    assert(!node2->GetLeft());
    assert(!node2->GetRight());

    node->ReplaceLeftWithLeaf(5);

    assert(node->GetLeft()->GetValue() == 5);
    assert(node->GetLeft()->GetParent() == node);

    node->ReplaceRightWithLeaf(518);

    assert(node->GetLeft()->GetValue() == 5);
    assert(node->GetLeft()->GetParent() == node);
    assert(node->GetRight()->GetValue() == 518);
    assert(node->GetRight()->GetParent() == node);

    auto leaf = TNode<int>::CreateLeaf(4);
    auto node3 = TNode<int>::Fork(0, node.get(), leaf.get());

    assert(node3->GetValue() == 0);
    assert(node3->GetLeft()->GetValue() == 1);
    assert(node3->GetLeft()->GetLeft()->GetValue() == 5);
    assert(node3->GetLeft()->GetRight()->GetValue() == 518);
    assert(node3->GetRight()->GetValue() == 4);

    assert(node3->GetLeft()->GetParent() == node3);

    return 0;
}
