#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <string>
#include "trees.h"

using namespace std;
using namespace stdext;

namespace trees_test
{

class test_data
{
public:
    test_data()
        : m_int_value(0), m_str_value(L"")
    { }
    test_data(const int intval, const wstring& strval)
        : m_int_value(intval), m_str_value(strval)
    { }
    test_data(const test_data& src) = default;
    test_data(test_data&& src) = default;
    test_data& operator = (const test_data& src) = default;
    test_data& operator = (test_data&& src) = default;
    wstring name() { return this->m_str_value; }
    int     int_value() { return m_int_value; }
    void    int_value(const int value) { m_int_value = value; }
    wstring str_value() { return m_str_value; }
    void    str_value(const wstring& value) { m_str_value = value; }
private:
    int     m_int_value;
    wstring m_str_value;
};


#define FILL_GENERIC_TREE_1(tree) \
      n1   = tree.create_node(nullptr, test_data(1, L"Node 1")); \
      n11  = tree.create_node(n1, test_data(11, L"Node 11")); \
      n12  = tree.create_node(n1, test_data(12, L"Node 12")); \
      n121 = tree.create_node(n12, test_data(121, L"Node 121")); \
      n13  = tree.create_node(n1, test_data(13, L"Node 13"));
//
#define FILL_GENERIC_TREE_2(tree) \
      n1    = tree.create_node(nullptr, test_data(1, L"Node 1")); \
      n11   = n1->append_child(test_data(11, L"Node 11")); \
      n12   = n1->append_child(test_data(12, L"Node 12")); \
      n121  = n12->append_child(test_data(121, L"Node 121")); \
      n1211 = n121->append_child(test_data(1211, L"Node 1211")); \
      n2    = tree.create_node(nullptr, test_data(2, L"Node 2")); \
      n21   = n2->append_child(test_data(21, L"Node 21")); \
      n3    = tree.create_node(nullptr, test_data(3, L"Node 3"));
//

TEST(GenericTreeTest, TestGenericTree)
{
    /*
      Node1
      |-- Node11
      |-- Node12
          |--Node121
      |-- Node13
    */
    stdext::gtree<test_data> tree;
    stdext::gtree<test_data>::node_t *n1, *n11, *n12, *n13, *n121, *n1211, *n2, *n21, *n3;
    FILL_GENERIC_TREE_1(tree);

    ASSERT_TRUE(n1 == tree.first_child()) << L"gtree first child";
    ASSERT_EQ(1, tree.child_count()) << L"gtree children count";
    ASSERT_EQ(5, tree.child_count(true)) << L"gtree children count recursive";
    // 1
    ASSERT_TRUE(n1->parent() == nullptr) << L"Node 1 parent";
    ASSERT_FALSE(n1->is_leaf()) << L"Node 1 is_leaf";
    ASSERT_TRUE(n1->first_child() == n11) << L"Node 1 first child";
    ASSERT_TRUE(n1->prev_sibling() == nullptr) << L"Node 1 prev sibling";
    ASSERT_TRUE(n1->next_sibling() == nullptr) << L"Node 1 next sibling";
    ASSERT_EQ(3, n1->child_count()) << L"Node 1 children count";
    ASSERT_EQ(4, n1->child_count(true)) << L"Node 1 children count recursive";
    ASSERT_EQ(L"Node 1", n1->data().str_value()) << L"Node 1 str_value";
    ASSERT_EQ(1, n1->data().int_value()) << L"Node 1 int_value";
    // 11
    ASSERT_TRUE(n11->first_child() == nullptr) << L"Node 11 first child";
    ASSERT_TRUE(n11->prev_sibling() == nullptr) << L"Node 11 prev sibling";
    ASSERT_TRUE(n11->next_sibling() == n12) << L"Node 11 next sibling";
    ASSERT_TRUE(n11->parent() == n1) << L"Node 11 parent";
    ASSERT_TRUE(n11->is_leaf()) << L"Node 11 is_leaf";
    ASSERT_EQ(0, n11->child_count()) << L"Node 11 children count";
    ASSERT_EQ(0, n11->child_count(true)) << L"Node 11 children count recursive";
    ASSERT_EQ(L"Node 11", n11->data().str_value()) << L"Node 11 str_value";
    ASSERT_EQ(11, n11->data().int_value()) << L"Node 11 int_value";
    // 12
    ASSERT_TRUE(n12->first_child() == n121) << L"Node 12 first child";
    ASSERT_TRUE(n12->prev_sibling() == n11) << L"Node 12 prev sibling";
    ASSERT_TRUE(n12->next_sibling() == n13) << L"Node 12 next sibling";
    ASSERT_TRUE(n12->parent() == n1) << L"Node 12 parent";
    ASSERT_FALSE(n12->is_leaf()) << L"Node 12 is_leaf()";
    ASSERT_EQ(1, n12->child_count()) << L"Node 12 children count";
    ASSERT_EQ(1, n12->child_count(true)) << L"Node 12 children count recursive";
    ASSERT_EQ(L"Node 12", n12->data().str_value()) << L"Node 12 str_value";
    ASSERT_EQ(12, n12->data().int_value()) << L"Node 12 int_value";
    // 13
    ASSERT_TRUE(n13->first_child() == nullptr) << L"Node 13 first child";
    ASSERT_TRUE(n13->prev_sibling() == n12) << L"Node 13 next sibling";
    ASSERT_TRUE(n13->next_sibling() == nullptr) << L"Node 13 next sibling";
    ASSERT_TRUE(n13->parent() == n1) << L"Node 13 parent";
    ASSERT_TRUE(n13->is_leaf()) << L"Node 13 is_leaf()";
    ASSERT_EQ(0, n13->child_count()) << L"Node 13 children count";
    ASSERT_EQ(0, n13->child_count(true)) << L"Node 13 children count recursive";
    ASSERT_EQ(L"Node 13", n13->data().str_value()) << L"Node 13 str_value";
    ASSERT_EQ(13, n13->data().int_value()) << L"Node 13 int_value";
    // 121
    ASSERT_TRUE(nullptr == n121->first_child()) << L"Node 121 first child";
    ASSERT_TRUE(nullptr == n121->next_sibling()) << L"Node 121 next sibling";
    ASSERT_TRUE(nullptr == n121->prev_sibling()) << L"Node 121 prev sibling";
    ASSERT_TRUE(n121->parent() == n12) << L"Node 121 parent";
    ASSERT_TRUE(n121->is_leaf()) << L"Node 121 is_leaf()";
    ASSERT_EQ(0, n121->child_count()) << L"Node 121 children count";
    ASSERT_EQ(0, n121->child_count(true)) << L"Node 121 children count recursive";
    ASSERT_EQ(L"Node 121", n121->data().str_value()) << L"Node 121 str_value";
    ASSERT_EQ(121, n121->data().int_value()) << L"Node 121 int_value";

    tree.clear();
    ASSERT_TRUE(nullptr == tree.first_child()) << L"clear 1. gtree first child";
    ASSERT_EQ(0, tree.child_count()) << L"clear 1. gtree children count";
    ASSERT_EQ(0, tree.child_count(true)) << L"clear 1. gtree children count recursive";

    FILL_GENERIC_TREE_1(tree)
    ASSERT_TRUE(n1 == tree.first_child()) << L"Fill 2. gtree first child";
    ASSERT_EQ(1, tree.child_count()) << L"Fill 2. gtree children count";
    ASSERT_EQ(5, tree.child_count(true)) << L"Fill 2. gtree children count recursive";

    tree.delete_node(n11);
    ASSERT_TRUE(n12 == n1->first_child()) << L"Delete 1. Node 1 first child";
    ASSERT_TRUE(n12->prev_sibling() == nullptr) << L"Delete 1. Node 12 prev sibling";
    ASSERT_EQ(1, tree.child_count()) << L"Delete 1. gtree children count";
    ASSERT_EQ(4, tree.child_count(true)) << L"Delete 1. gtree children count recursive";
    tree.delete_node(n12);
    ASSERT_TRUE(n13 == n1->first_child()) << L"Delete 2. Node 1 first child";
    ASSERT_TRUE(n13->prev_sibling() == nullptr) << L"Delete 2. Node 12 prev sibling";
    ASSERT_EQ(1, tree.child_count()) << L"Delete 2. gtree children count";
    ASSERT_EQ(2, tree.child_count(true)) << L"Delete 2. gtree children count recursive";
    tree.delete_node(n13);
    ASSERT_TRUE(nullptr == n1->first_child()) << L"Delete 3. Node 1 first child";
    ASSERT_EQ(1, tree.child_count()) << L"Delete 3. gtree children count";
    ASSERT_EQ(1, tree.child_count(true)) << L"Delete 3. gtree children count recursive";
    tree.delete_node(n1);
    ASSERT_TRUE(nullptr == tree.first_child()) << L"Delete 4. gtree first child";
    ASSERT_EQ(0, tree.child_count()) << L"Delete 4. gtree children count";
    ASSERT_EQ(0, tree.child_count(true)) << L"Delete 4. gtree children count recursive";

    FILL_GENERIC_TREE_1(tree);
    ASSERT_TRUE(n1 == tree.first_child()) << L"Fill 3. gtree first child";
    ASSERT_EQ(1, tree.child_count()) << L"Fill 3. gtree children count";
    ASSERT_EQ(5, tree.child_count(true)) << L"Fill 3. gtree children count recursive";
    tree.delete_node(n1);
    ASSERT_TRUE(nullptr == tree.first_child()) << L"Delete 5. gtree first child";
    ASSERT_EQ(0, tree.child_count()) << L"Delete 5. gtree children count";
    ASSERT_EQ(0, tree.child_count(true)) << L"Delete 5. gtree children count recursive";

    tree.clear();
    FILL_GENERIC_TREE_1(tree);
    ASSERT_EQ(0, n13->child_count(true)) << L"Move 1 (before). Node 13 children count recursive";
    tree.move_node(n12, n13);
    ASSERT_EQ(1, tree.child_count()) << L"Move 1. gtree children count";
    ASSERT_EQ(5, tree.child_count(true)) << L"Move 1. gtree children count recursive";
    ASSERT_EQ(2, n13->child_count(true)) << L"Move 1. Node 13 children count recursive";

    tree.clear();
    FILL_GENERIC_TREE_1(tree);
    n2 = tree.create_node(nullptr, test_data(2, L"Node 2"));
    {
        stdext::gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << L"iterator 1. Node 1. Started";
        ASSERT_TRUE(n11 == it.next()) << L"iterator 1. Node 11";
        ++it;
        ASSERT_TRUE(n12 == it.current()) << L"iterator 1. Node 12";
        ASSERT_TRUE(n121 == it.next()) << L"iterator 1. Node 121";
        ASSERT_TRUE(n13 == it.next()) << L"iterator 1. Node 13";
        ASSERT_TRUE(n2 == it.next()) << L"iterator 1. Node 2";
        ASSERT_TRUE(nullptr == it.next()) << L"iterator 1. Finished";
    }

    tree.clear();
    FILL_GENERIC_TREE_2(tree);
    {
        stdext::gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << L"iterator 2. Node 1. Started";
        it.next();
        ASSERT_TRUE(n11 == it.current()) << L"iterator 2. Node 11";
        ++it;
        ASSERT_TRUE(n12 == it.current()) << L"iterator 2. Node 12";
        it.next();
        ASSERT_TRUE(n121 == it.current()) << L"iterator 2. Node 121";
        it.next();
        ASSERT_TRUE(n1211 == it.current()) << L"iterator 2. Node n1211";
        it.next();
        ASSERT_TRUE(n2 == it.current()) << L"iterator 2. Node 2";
        it.next();
        ASSERT_TRUE(n21 == it.current()) << L"iterator 2. Node 21";
        it.next();
        ASSERT_TRUE(n3 == it.current()) << L"iterator 2. Node 3";
        it.next();
        ASSERT_TRUE(nullptr == it.current()) << L"iterator 2. Finished";
    }

    tree.clear();
    {
        stdext::gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(nullptr == it.current()) << L"iterator 3. Started";
        ASSERT_TRUE(nullptr == it.next()) << L"iterator 3. Empty next";
        ASSERT_TRUE(nullptr == it.current()) << L"iterator 3. Empty current";
    }

    tree.clear();
    n1 = tree.create_node(nullptr, test_data(1, L"Node 1"));
    n2 = tree.create_node(nullptr, test_data(2, L"Node 2"));
    n12 = n1->append_child(test_data(12, L"Node 12"));
    n13 = n1->append_child(test_data(13, L"Node 13"));
    n11 = n1->append_first_child(test_data(11, L"Node 11"));
    {
        stdext::gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << L"iterator 4. Node 1. Started";
        ASSERT_TRUE(n11 == it.next()) << L"iterator 4. Node 11";
        ASSERT_TRUE(n12 == it.next()) << L"iterator 4. Node 12";
        ASSERT_TRUE(n13 == it.next()) << L"iterator 4. Node 13";
        ASSERT_TRUE(n2 == it.next()) << L"iterator 4. Node 2";
        ASSERT_TRUE(nullptr == it.next()) << L"iterator 4. Finished";
    }

    ASSERT_TRUE(move_node_result_t::error_node_is_null == tree.move_node(nullptr, n2)) << L"move_node error error_node_is_null";
    ASSERT_TRUE(move_node_result_t::error_node_already_has_this_parent == tree.move_node(n12, n1)) << L"move_node error error_node_already_has_this_parent";
    ASSERT_TRUE(move_node_result_t::error_node_is_parent ==  tree.move_node(n12, n12)) << L"move_node error error_node_is_parent";
    ASSERT_TRUE(move_node_result_t::error_parent_is_node_child == tree.move_node(n1, n12)) << L"move_node error error_parent_is_node_child";

    ASSERT_TRUE(move_node_result_t::success == tree.move_node(n12, n2)) << L"iterator 5. move_node 1";
    {
        stdext::gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << L"iterator 5. Node 1. Started";
        ASSERT_EQ(2, n1->child_count(true)) << L"iterator 5. Node 1 ChildrenCount";
        ASSERT_TRUE(n11 == it.next()) << L"iterator 5. Node 11";
        ASSERT_TRUE(n13 == it.next()) << L"iterator 5. Node 13";
        ASSERT_TRUE(n2 == it.next()) << L"iterator 5. Node 2";
        ASSERT_EQ(1, n2->child_count(true)) << L"iterator 5. Node 2 ChildrenCount";
        ASSERT_TRUE(n12 == it.next()) << L"iterator 5. Node 12";
        ASSERT_TRUE(nullptr == it.next()) << L"iterator 5. Finished";
    }
    ASSERT_TRUE(move_node_result_t::success == tree.move_node(n13, n2)) << L"iterator 6. move_node 1";
    tree.move_node_first(n13);
    {
        stdext::gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << L"iterator 6. Node 1. Started";
        ASSERT_EQ(1, n1->child_count(true)) << L"iterator 6. Node 1 ChildrenCount";
        ASSERT_TRUE(n11 == it.next()) << L"iterator 6. Node 11";
        ASSERT_TRUE(n2 == it.next()) << L"iterator 6. Node 2";
        ASSERT_EQ(2, n2->child_count(true)) << L"iterator 6. Node 2 ChildrenCount";
        ASSERT_TRUE(n13 == it.next()) << L"iterator 6. Node 13";
        ASSERT_TRUE(n12 == it.next()) << L"iterator 6. Node 12";
        ASSERT_TRUE(nullptr == it.next()) << L"iterator 6. Finished";
    }
    ASSERT_TRUE(move_node_result_t::success == tree.move_node(n13, n12)) << L"iterator 7. move_node 1";
    {
        stdext::gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << L"iterator 7. Node 1. Started";
        ASSERT_EQ(1, n1->child_count(true)) << L"iterator 7. Node 1 ChildrenCount";
        ASSERT_TRUE(n11 == it.next()) << L"iterator 7. Node 11";
        ASSERT_TRUE(n2 == it.next()) << L"iterator 7. Node 2";
        ASSERT_EQ(2, n2->child_count(true)) << L"iterator 7. Node 2 ChildrenCount";
        ASSERT_EQ(1, n12->child_count(true)) << L"iterator 7. Node 12 ChildrenCount";
        ASSERT_TRUE(n12 == it.next()) << L"iterator 7. Node 12";
        ASSERT_TRUE(n13 == it.next()) << L"iterator 7. Node 13";
        ASSERT_TRUE(nullptr == it.next()) << L"iterator 7. Finished";
    }
}

}
