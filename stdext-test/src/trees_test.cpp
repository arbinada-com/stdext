#include <gtest/gtest.h>
#include <string>
#include "trees.h"

using namespace std;

namespace stdext {
namespace trees_test {

using namespace std;

class test_data
{
public:
    test_data()
        : m_int_value(0), m_str_value("")
    {}
    test_data(const int intval, const string& strval)
        : m_int_value(intval), m_str_value(strval)
    {}
    test_data(const test_data& src) = default;
    test_data(test_data&& src) = default;
    test_data& operator = (const test_data& src) = default;
    test_data& operator = (test_data&& src) = default;
public:
    bool operator ==(const test_data& rhs) const
    {
        return m_str_value == rhs.m_str_value && m_int_value == rhs.m_int_value;
    }
    string name() { return this->m_str_value; }
    int    int_value() { return m_int_value; }
    void   int_value(const int value) { m_int_value = value; }
    string str_value() { return m_str_value; }
    void   str_value(const string& value) { m_str_value = value; }
private:
    int    m_int_value;
    string m_str_value;
};


#define FILL_GENERIC_TREE_1(tree) \
    n1   = tree.create_node(nullptr, test_data(1, "Node 1")); \
    n11  = tree.create_node(n1, test_data(11, "Node 11")); \
    n12  = tree.create_node(n1, test_data(12, "Node 12")); \
    n121 = tree.create_node(n12, test_data(121, "Node 121")); \
    n13  = tree.create_node(n1, test_data(13, "Node 13"));
//
#define FILL_GENERIC_TREE_2(tree) \
    n1    = tree.create_node(nullptr, test_data(1, "Node 1")); \
    n11   = n1->append_child(test_data(11, "Node 11")); \
    n12   = n1->append_child(test_data(12, "Node 12")); \
    n121  = n12->append_child(test_data(121, "Node 121")); \
    n1211 = n121->append_child(test_data(1211, "Node 1211")); \
    n2    = tree.create_node(nullptr, test_data(2, "Node 2")); \
    n21   = n2->append_child(test_data(21, "Node 21")); \
    n3    = tree.create_node(nullptr, test_data(3, "Node 3"));
//

TEST(Trees, TestGTree)
{
    /*
      Node1
      |-- Node11
      |-- Node12
          |--Node121
      |-- Node13
    */
    gtree<test_data> tree;
    EXPECT_EQ(tree.child_count(), 0u);
    tree.clear();
    EXPECT_EQ(tree.child_count(), 0u);

    gtree<test_data>::node_t *n1, *n11, *n12, *n13, *n121;
    FILL_GENERIC_TREE_1(tree);

    ASSERT_TRUE(n1 == tree.first_child()) << "gtree first child";
    ASSERT_EQ(1u, tree.child_count()) << "gtree children count";
    ASSERT_EQ(5u, tree.child_count(true)) << "gtree children count recursive";
    // 1
    ASSERT_TRUE(n1->parent() == nullptr) << "Node 1 parent";
    ASSERT_FALSE(n1->is_leaf()) << "Node 1 is_leaf";
    ASSERT_TRUE(n1->first_child() == n11) << "Node 1 first child";
    ASSERT_TRUE(n1->prev_sibling() == nullptr) << "Node 1 prev sibling";
    ASSERT_TRUE(n1->next_sibling() == nullptr) << "Node 1 next sibling";
    ASSERT_EQ(3u, n1->child_count()) << "Node 1 children count";
    ASSERT_EQ(4u, n1->child_count(true)) << "Node 1 children count recursive";
    ASSERT_EQ("Node 1", n1->data().str_value()) << "Node 1 str_value";
    ASSERT_EQ(1, n1->data().int_value()) << "Node 1 int_value";
    // 11
    ASSERT_TRUE(n11->first_child() == nullptr) << "Node 11 first child";
    ASSERT_TRUE(n11->prev_sibling() == nullptr) << "Node 11 prev sibling";
    ASSERT_TRUE(n11->next_sibling() == n12) << "Node 11 next sibling";
    ASSERT_TRUE(n11->parent() == n1) << "Node 11 parent";
    ASSERT_TRUE(n11->is_leaf()) << "Node 11 is_leaf";
    ASSERT_EQ(0u, n11->child_count()) << "Node 11 children count";
    ASSERT_EQ(0u, n11->child_count(true)) << "Node 11 children count recursive";
    ASSERT_EQ("Node 11", n11->data().str_value()) << "Node 11 str_value";
    ASSERT_EQ(11, n11->data().int_value()) << "Node 11 int_value";
    // 12
    ASSERT_TRUE(n12->first_child() == n121) << "Node 12 first child";
    ASSERT_TRUE(n12->prev_sibling() == n11) << "Node 12 prev sibling";
    ASSERT_TRUE(n12->next_sibling() == n13) << "Node 12 next sibling";
    ASSERT_TRUE(n12->parent() == n1) << "Node 12 parent";
    ASSERT_FALSE(n12->is_leaf()) << "Node 12 is_leaf()";
    ASSERT_EQ(1u, n12->child_count()) << "Node 12 children count";
    ASSERT_EQ(1u, n12->child_count(true)) << "Node 12 children count recursive";
    ASSERT_EQ("Node 12", n12->data().str_value()) << "Node 12 str_value";
    ASSERT_EQ(12, n12->data().int_value()) << "Node 12 int_value";
    // 13
    ASSERT_TRUE(n13->first_child() == nullptr) << "Node 13 first child";
    ASSERT_TRUE(n13->prev_sibling() == n12) << "Node 13 next sibling";
    ASSERT_TRUE(n13->next_sibling() == nullptr) << "Node 13 next sibling";
    ASSERT_TRUE(n13->parent() == n1) << "Node 13 parent";
    ASSERT_TRUE(n13->is_leaf()) << "Node 13 is_leaf()";
    ASSERT_EQ(0u, n13->child_count()) << "Node 13 children count";
    ASSERT_EQ(0u, n13->child_count(true)) << "Node 13 children count recursive";
    ASSERT_EQ("Node 13", n13->data().str_value()) << "Node 13 str_value";
    ASSERT_EQ(13, n13->data().int_value()) << "Node 13 int_value";
    // 121
    ASSERT_TRUE(nullptr == n121->first_child()) << "Node 121 first child";
    ASSERT_TRUE(nullptr == n121->next_sibling()) << "Node 121 next sibling";
    ASSERT_TRUE(nullptr == n121->prev_sibling()) << "Node 121 prev sibling";
    ASSERT_TRUE(n121->parent() == n12) << "Node 121 parent";
    ASSERT_TRUE(n121->is_leaf()) << "Node 121 is_leaf()";
    ASSERT_EQ(0u, n121->child_count()) << "Node 121 children count";
    ASSERT_EQ(0u, n121->child_count(true)) << "Node 121 children count recursive";
    ASSERT_EQ("Node 121", n121->data().str_value()) << "Node 121 str_value";
    ASSERT_EQ(121, n121->data().int_value()) << "Node 121 int_value";

    tree.clear();
    ASSERT_TRUE(nullptr == tree.first_child()) << "clear 1. gtree first child";
    ASSERT_EQ(0u, tree.child_count()) << "clear 1. gtree children count";
    ASSERT_EQ(0u, tree.child_count(true)) << "clear 1. gtree children count recursive";

    FILL_GENERIC_TREE_1(tree)
            ASSERT_TRUE(n1 == tree.first_child()) << "Fill 2. gtree first child";
    ASSERT_EQ(1u, tree.child_count()) << "Fill 2. gtree children count";
    ASSERT_EQ(5u, tree.child_count(true)) << "Fill 2. gtree children count recursive";

    tree.delete_node(n11);
    ASSERT_TRUE(n12 == n1->first_child()) << "Delete 1. Node 1 first child";
    ASSERT_TRUE(n12->prev_sibling() == nullptr) << "Delete 1. Node 12 prev sibling";
    ASSERT_EQ(1u, tree.child_count()) << "Delete 1. gtree children count";
    ASSERT_EQ(4u, tree.child_count(true)) << "Delete 1. gtree children count recursive";
    tree.delete_node(n12);
    ASSERT_TRUE(n13 == n1->first_child()) << "Delete 2. Node 1 first child";
    ASSERT_TRUE(n13->prev_sibling() == nullptr) << "Delete 2. Node 12 prev sibling";
    ASSERT_EQ(1u, tree.child_count()) << "Delete 2. gtree children count";
    ASSERT_EQ(2u, tree.child_count(true)) << "Delete 2. gtree children count recursive";
    tree.delete_node(n13);
    ASSERT_TRUE(nullptr == n1->first_child()) << "Delete 3. Node 1 first child";
    ASSERT_EQ(1u, tree.child_count()) << "Delete 3. gtree children count";
    ASSERT_EQ(1u, tree.child_count(true)) << "Delete 3. gtree children count recursive";
    tree.delete_node(n1);
    ASSERT_TRUE(nullptr == tree.first_child()) << "Delete 4. gtree first child";
    ASSERT_EQ(0u, tree.child_count()) << "Delete 4. gtree children count";
    ASSERT_EQ(0u, tree.child_count(true)) << "Delete 4. gtree children count recursive";

    FILL_GENERIC_TREE_1(tree);
    ASSERT_TRUE(n1 == tree.first_child()) << "Fill 3. gtree first child";
    ASSERT_EQ(1u, tree.child_count()) << "Fill 3. gtree children count";
    ASSERT_EQ(5u, tree.child_count(true)) << "Fill 3. gtree children count recursive";
    tree.delete_node(n1);
    ASSERT_TRUE(nullptr == tree.first_child()) << "Delete 5. gtree first child";
    ASSERT_EQ(0u, tree.child_count()) << "Delete 5. gtree children count";
    ASSERT_EQ(0u, tree.child_count(true)) << "Delete 5. gtree children count recursive";

    tree.clear();
    FILL_GENERIC_TREE_1(tree);
    ASSERT_EQ(0u, n13->child_count(true)) << "Move 1 (before). Node 13 children count recursive";
    tree.move_node(n12, n13);
    ASSERT_EQ(1u, tree.child_count()) << "Move 1. gtree children count";
    ASSERT_EQ(5u, tree.child_count(true)) << "Move 1. gtree children count recursive";
    ASSERT_EQ(2u, n13->child_count(true)) << "Move 1. Node 13 children count recursive";
}


TEST(Trees, TestGTree_Move)
{
    gtree<test_data> tree;
    gtree<test_data>::node_t *n1, *n11, *n12, *n13, *n121;
    FILL_GENERIC_TREE_1(tree);
    gtree<test_data>::size_type size = tree.child_count(true);
    EXPECT_TRUE(size > 0);
    gtree<test_data> tree2(std::move(tree));
    gtree<test_data>::iterator it(tree2);
    ASSERT_FALSE(it.is_end());
    EXPECT_EQ(size, tree2.child_count(true));
    EXPECT_TRUE(n1 == *it); ++it;
    EXPECT_TRUE(n11 == *it); ++it;
    EXPECT_TRUE(n12 == *it); ++it;
    EXPECT_TRUE(n121 == *it); ++it;
    EXPECT_TRUE(n13 == *it); ++it;
    EXPECT_TRUE(it.is_end());
    ++it;
    EXPECT_TRUE(it.is_end());
}

TEST(Trees, TestGTree_DataMove)
{
    gtree<test_data> tree;
    gtree<test_data>::node_t *n1, *n2;
    n1 = tree.create_node(nullptr, test_data(1, "Node 1"));
    test_data d2(2, "Node 2");
    tree.create_node(nullptr, std::move(d2));
    n2 = n1->next_sibling();
    ASSERT_FALSE(nullptr == n2);
    ASSERT_EQ(n2->data().name(), "Node 2");
    ASSERT_EQ(n2->data().int_value(), 2);
}

TEST(Trees, TestGTree_Iterator_TopDown_Empty)
{
    gtree<test_data> tree;
    gtree<test_data>::iterator it(&tree);
    EXPECT_TRUE(it.is_end());
    EXPECT_TRUE(*it == nullptr);
    EXPECT_TRUE(it.next() == nullptr);
    EXPECT_TRUE(*it == nullptr);
}

TEST(Trees, TestGTree_Iterator_TopDown_Flat)
{
    gtree<test_data> tree;
    gtree<test_data>::node_t *n1, *n2;
    n1 = tree.create_node(nullptr, test_data(1, "Node 1"));
    n2 = tree.create_node(nullptr, test_data(2, "Node 2"));
    gtree<test_data>::iterator it(&tree);
    EXPECT_FALSE(it.is_end());
    EXPECT_TRUE(*it == n1);
    EXPECT_FALSE(it.next() == nullptr);
    EXPECT_TRUE(*it == n2);
    ++it;
    EXPECT_TRUE(it.is_end());
}

TEST(Trees, TestGTree_Iterator_TopDown)
{
    gtree<test_data> tree;
    gtree<test_data>::node_t *n1, *n11, *n12, *n13, *n121, *n1211, *n2, *n21, *n3;
    FILL_GENERIC_TREE_1(tree);
    n2 = tree.create_node(nullptr, test_data(2, "Node 2"));
    {
        gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << "iterator 1. Node 1. Started";
        ASSERT_TRUE(n11 == it.next()) << "iterator 1. Node 11";
        ++it;
        ASSERT_TRUE(n12 == it.current()) << "iterator 1. Node 12";
        ASSERT_TRUE(n121 == it.next()) << "iterator 1. Node 121";
        ASSERT_TRUE(n13 == it.next()) << "iterator 1. Node 13";
        ASSERT_TRUE(n2 == it.next()) << "iterator 1. Node 2";
        ASSERT_TRUE(nullptr == it.next()) << "iterator 1. Finished";
    }

    tree.clear();
    FILL_GENERIC_TREE_2(tree);
    {
        gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << "iterator 2. Node 1. Started";
        it.next();
        ASSERT_TRUE(n11 == it.current()) << "iterator 2. Node 11";
        ++it;
        ASSERT_TRUE(n12 == it.current()) << "iterator 2. Node 12";
        it.next();
        ASSERT_TRUE(n121 == it.current()) << "iterator 2. Node 121";
        it.next();
        ASSERT_TRUE(n1211 == it.current()) << "iterator 2. Node n1211";
        it.next();
        ASSERT_TRUE(n2 == it.current()) << "iterator 2. Node 2";
        it.next();
        ASSERT_TRUE(n21 == it.current()) << "iterator 2. Node 21";
        it.next();
        ASSERT_TRUE(n3 == it.current()) << "iterator 2. Node 3";
        it.next();
        ASSERT_TRUE(nullptr == it.current()) << "iterator 2. Finished";
    }

    tree.clear();
    {
        gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(nullptr == it.current()) << "iterator 3. Started";
        ASSERT_TRUE(nullptr == it.next()) << "iterator 3. Empty next";
        ASSERT_TRUE(nullptr == it.current()) << "iterator 3. Empty current";
    }

    tree.clear();
    n1 = tree.create_node(nullptr, test_data(1, "Node 1"));
    n2 = tree.create_node(nullptr, test_data(2, "Node 2"));
    n12 = n1->append_child(test_data(12, "Node 12"));
    n13 = n1->append_child(test_data(13, "Node 13"));
    n11 = n1->append_first_child(test_data(11, "Node 11"));
    {
        gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << "iterator 4. Node 1. Started";
        ASSERT_TRUE(n11 == it.next()) << "iterator 4. Node 11";
        ASSERT_TRUE(n12 == it.next()) << "iterator 4. Node 12";
        ASSERT_TRUE(n13 == it.next()) << "iterator 4. Node 13";
        ASSERT_TRUE(n2 == it.next()) << "iterator 4. Node 2";
        ASSERT_TRUE(nullptr == it.next()) << "iterator 4. Finished";
    }

    ASSERT_TRUE(move_node_result_t::error_node_is_null == tree.move_node(nullptr, n2)) << "move_node error error_node_is_null";
    ASSERT_TRUE(move_node_result_t::error_node_already_has_this_parent == tree.move_node(n12, n1)) << "move_node error error_node_already_has_this_parent";
    ASSERT_TRUE(move_node_result_t::error_node_is_parent ==  tree.move_node(n12, n12)) << "move_node error error_node_is_parent";
    ASSERT_TRUE(move_node_result_t::error_parent_is_node_child == tree.move_node(n1, n12)) << "move_node error error_parent_is_node_child";

    ASSERT_TRUE(move_node_result_t::success == tree.move_node(n12, n2)) << "iterator 5. move_node 1";
    {
        gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << "iterator 5. Node 1. Started 1";
        ASSERT_TRUE(n1 == *it) << "iterator 5. Node 1. Started 2";
        ASSERT_TRUE(n1->data() == it->data()) << "iterator 5. Node 1. Started 3";
        ASSERT_EQ(2u, n1->child_count(true)) << "iterator 5. Node 1 ChildrenCount";
        ASSERT_TRUE(n11 == it.next()) << "iterator 5. Node 11";
        ASSERT_TRUE(n13 == it.next()) << "iterator 5. Node 13";
        ASSERT_TRUE(n2 == it.next()) << "iterator 5. Node 2";
        ASSERT_EQ(1u, n2->child_count(true)) << "iterator 5. Node 2 ChildrenCount";
        ASSERT_TRUE(n12 == it.next()) << "iterator 5. Node 12";
        ASSERT_TRUE(nullptr == it.next()) << "iterator 5. Finished";
    }
    ASSERT_TRUE(move_node_result_t::success == tree.move_node(n13, n2)) << "iterator 6. move_node 1";
    tree.move_node_first(n13);
    {
        gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << "iterator 6. Node 1. Started";
        ASSERT_EQ(1u, n1->child_count(true)) << "iterator 6. Node 1 ChildrenCount";
        ASSERT_TRUE(n11 == it.next()) << "iterator 6. Node 11";
        ASSERT_TRUE(n2 == it.next()) << "iterator 6. Node 2";
        ASSERT_EQ(2u, n2->child_count(true)) << "iterator 6. Node 2 ChildrenCount";
        ASSERT_TRUE(n13 == it.next()) << "iterator 6. Node 13";
        ASSERT_TRUE(n12 == it.next()) << "iterator 6. Node 12";
        ASSERT_TRUE(nullptr == it.next()) << "iterator 6. Finished";
    }
    ASSERT_TRUE(move_node_result_t::success == tree.move_node(n13, n12)) << "iterator 7. move_node 1";
    {
        gtree<test_data>::iterator it(&tree);
        ASSERT_TRUE(n1 == it.current()) << "iterator 7. Node 1. Started";
        ASSERT_EQ(1u, n1->child_count(true)) << "iterator 7. Node 1 ChildrenCount";
        ASSERT_TRUE(n11 == it.next()) << "iterator 7. Node 11";
        ASSERT_TRUE(n2 == it.next()) << "iterator 7. Node 2";
        ASSERT_EQ(2u, n2->child_count(true)) << "iterator 7. Node 2 ChildrenCount";
        ASSERT_EQ(1u, n12->child_count(true)) << "iterator 7. Node 12 ChildrenCount";
        ASSERT_TRUE(n12 == it.next()) << "iterator 7. Node 12";
        ASSERT_TRUE(n13 == it.next()) << "iterator 7. Node 13";
        ASSERT_TRUE(nullptr == it.next()) << "iterator 7. Finished";
    }
}


TEST(Trees, TestGTree_Iterator_BottomUp_Empty)
{
    gtree<test_data> tree;
    gtree<test_data>::iterator_bottom_up it(&tree);
    EXPECT_TRUE(it.is_end());
    EXPECT_TRUE(*it == nullptr);
    EXPECT_TRUE(it.next() == nullptr);
    EXPECT_TRUE(*it == nullptr);
    ++it;
    EXPECT_TRUE(*it == nullptr);
    it++;
    EXPECT_TRUE(*it == nullptr);
}

TEST(Trees, TestGTree_Iterator_BottomUp_Flat)
{
    gtree<test_data> tree;
    gtree<test_data>::node_t *n1, *n2;
    n1 = tree.create_node(nullptr, test_data(1, "Node 1"));
    n2 = tree.create_node(nullptr, test_data(2, "Node 2"));
    gtree<test_data>::iterator_bottom_up it(&tree);
    EXPECT_FALSE(it.is_end());
    EXPECT_TRUE(*it == n1);
    EXPECT_FALSE(it.next() == nullptr);
    EXPECT_TRUE(*it == n2);
    ++it;
    EXPECT_TRUE(it.is_end());
}

TEST(Trees, TestGTree_Iterator_BottomUp)
{
    gtree<test_data> tree;
    gtree<test_data>::node_t *n1, *n11, *n12, *n13, *n121, *n1211, *n2, *n21, *n3;
    FILL_GENERIC_TREE_1(tree);
    /*
      Node1
      |-- Node11
      |-- Node12
          |--Node121
      |-- Node13
    */
    n2 = tree.create_node(nullptr, test_data(2, "Node 2"));
    {
        gtree<test_data>::iterator_bottom_up it(&tree);
        EXPECT_FALSE(it.is_end());
        EXPECT_TRUE(n11 == it.current()) << it->data().name();
        ASSERT_TRUE(n11 == *it) << it->data().name();
        ASSERT_TRUE(n121 == it.next()) << it->data().name();
        ++it;
        ASSERT_TRUE(n12 == it.current()) << it->data().name();
        ASSERT_TRUE(n13 == it.next()) << it->data().name();
        ASSERT_TRUE(n1 == it.next()) << it->data().name();
        ASSERT_TRUE(n2 == it.next()) << it->data().name();
        ASSERT_TRUE(nullptr == it.next());
        EXPECT_TRUE(it.is_end());
    }

    tree.clear();
    FILL_GENERIC_TREE_2(tree);
    /*
      Node1
      |-- Node11
      |-- Node12
        |--Node121
          |--Node1211
      Node2
      |-- Node21
      Node3
    */
    {
        gtree<test_data>::iterator_bottom_up it(&tree);
        ASSERT_TRUE(n11 == it.current()) << it->data().name();
        it.next();
        ASSERT_TRUE(n1211 == it.current()) << it->data().name();
        ++it;
        ASSERT_TRUE(n121 == it.current()) << it->data().name();
        it.next();
        ASSERT_TRUE(n12 == it.current()) << it->data().name();
        it.next();
        ASSERT_TRUE(n1 == it.current()) << it->data().name();
        it.next();
        ASSERT_TRUE(n21 == it.current()) << it->data().name();
        it.next();
        ASSERT_TRUE(n2 == it.current()) << it->data().name();
        it.next();
        ASSERT_TRUE(n3 == it.current()) << it->data().name();
        it.next();
        ASSERT_TRUE(nullptr == it.current());
    }
}

}
}
