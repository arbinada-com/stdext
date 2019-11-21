/*
 C++ standard library extensions
 (c) 2001-2019 Serguei Tarassov (see license.txt)

 A generic tree implementation
 */
#pragma once

#include <memory>

namespace stdext
{


    template <class T> class gtree_node;

    typedef enum class move_node_result
    {
        success,
        error_node_is_null,
        error_node_already_has_this_parent,
        error_node_is_parent,
        error_parent_is_node_child
    } move_node_result_t;

    /*
     Generic tree template class
     */
    template <class T, class Allocator = std::allocator<gtree_node<T> > >
    class gtree
    {
    public:
        typedef gtree_node<T> node_t;
    public:
        gtree()
        {
            m_root = m_alloc.allocate(1);
            m_alloc.construct(m_root, node_t(this));
        }

        virtual ~gtree()
        {
            clear();
            m_alloc.destroy(m_root);
            m_alloc.deallocate(m_root, 1);
        }

        void clear()
        {
            node_t* node = m_root->m_first_child;
            while (node != nullptr)
            {
                node_t* curr = node;
                node = node->m_next_sibling;
                delete_node(curr);
            }
            m_root->m_first_child = nullptr;
        }

        node_t* create_node(node_t* const parent, const T& data)
        {
            node_t* result = m_alloc.allocate(1);
            m_alloc.construct(result, node_t(this, data));
            append_node(result, parent);
            return result;
        }

        node_t* create_node(node_t* const parent)
        {
            T data;
            return create_node(parent, data);
        }

        node_t* create_node_first(node_t* const parent, const T& data)
        {
            node_t* result = create_node(parent, data);
            move_node_first(result);
            return result;
        }

        void delete_node(node_t* const node)
        {
            if (node == m_root || node == nullptr)
                return;
            node_t* parent = node->m_parent;
            node_t* child = node->m_first_child;
            while (child != nullptr)
            {
                node_t* curr = child;
                child = child->m_next_sibling;
                delete_node(curr);
            }
            if (node->m_prev_sibling != nullptr)
                node->m_prev_sibling->m_next_sibling = node->m_next_sibling;
            if (node->m_next_sibling != nullptr)
                node->m_next_sibling->m_prev_sibling = node->m_prev_sibling;
            if (parent == nullptr)
                parent = m_root;
            if (parent->m_first_child == node)
                parent->m_first_child = node->m_next_sibling;
            parent->m_child_count--;
            m_alloc.destroy(node);
            m_alloc.deallocate(node, 1);
        }

        const int child_count() const
        {
            return m_root->child_count(false);
        }

        const int child_count(bool recursive) const
        {
            return m_root->child_count(recursive);
        }

        inline node_t* first_child() const
        {
            return m_root->m_first_child;
        }

        bool is_child(const node_t* parent, const node_t* node, const bool recursive)
        {
            if (parent == nullptr || node == nullptr)
                return false;
            node_t* curr = parent->first_child();
            while (curr != nullptr)
            {
                if (curr == node || (recursive && is_child(curr, node, recursive)))
                    return true;
                curr = curr->next_sibling();
            }
            return false;
        }

        move_node_result_t move_node(node_t* const node, node_t* const parent)
        {
            if (node == nullptr)
                return move_node_result_t::error_node_is_null;
            else if (node->m_parent == parent)
                return move_node_result_t::error_node_already_has_this_parent;
            else if (node == parent)
                return move_node_result_t::error_node_is_parent;
            else if (is_child(node, parent, true))
                return move_node_result_t::error_parent_is_node_child;

            if (node->m_prev_sibling != nullptr)
                node->m_prev_sibling->m_next_sibling = node->m_next_sibling;
            if (node->m_next_sibling != nullptr)
                node->m_next_sibling->m_prev_sibling = node->m_prev_sibling;
            node_t* root = node->m_parent;
            if (root == nullptr)
                root = m_root;
            root->m_child_count--;
            if (root->m_first_child == node)
            {
                root->m_first_child = node->m_next_sibling;
                if (root->m_first_child != nullptr)
                    root->m_first_child->m_parent = root == m_root ? nullptr : root;
            }
            append_node(node, parent);
            return move_node_result_t::success;
        }

        void move_node_first(node_t* const node)
        {
            if (node == nullptr)
                return;
            while (node->prev_sibling() != nullptr)
                move_node_up(node);
        }

        void move_node_up(node_t* const node)
        {
            if (node == nullptr)
                return;
            node_t* prev = node->m_prev_sibling;
            if (prev != nullptr)
            {
                prev->m_next_sibling = node->m_next_sibling;
                node->m_prev_sibling = prev->m_prev_sibling;
                node->m_next_sibling = prev;
                prev->m_prev_sibling = node;
            }
            if (node->m_prev_sibling == nullptr && node->m_parent != nullptr)
                node->m_parent->m_first_child = node;
        }

    public:
        class iterator
        {
        public:
            iterator(const gtree* tree)
                : m_tree(tree), m_level(0)
            {
                m_current = tree->first_child();
                m_curr_root = m_current;
            }

            node_t* next()
            {
                if (m_current != nullptr)
                {
                    if (m_current->first_child() != nullptr)
                    {
                        m_current = m_current->first_child();
                        m_level++;
                    }
                    else
                    {
                        if (m_current->next_sibling() != nullptr)
                            m_current = m_current->next_sibling();
                        else
                        {
                            while (m_current->parent() != nullptr)
                            {
                                m_current = m_current->parent();
                                m_level--;
                                if (m_current->next_sibling() != nullptr)
                                {
                                    m_current = m_current->next_sibling();
                                    break;
                                }
                            }
                            if (m_current->parent() == nullptr)
                            {
                                m_current = m_curr_root->next_sibling();
                                m_curr_root = m_current;
                            }
                        }
                    }
                }
                return m_current;
            }

            inline void operator ++ ()
            {
                next();
            }

            inline node_t* current() const
            {
                return m_current;
            }

            inline int level() const
            {
                return m_level;
            }

            inline bool is_end()
            {
                return m_current == nullptr;
            }

        private:
            const gtree* m_tree;
            node_t*      m_current;
            node_t*      m_curr_root;
            int          m_level;
        };

    protected:
        void append_node(node_t* const node, node_t* const parent)
        {
            node->m_parent = parent;
            node->m_prev_sibling = nullptr;
            node->m_next_sibling = nullptr;
            if (parent == nullptr)
            {
                m_root->m_child_count++;
                if (m_root->m_first_child == nullptr)
                {
                    m_root->m_first_child = node;
                }
                else
                {
                    node_t* lastChild = m_root->last_child();
                    node->m_prev_sibling = lastChild;
                    if (lastChild != nullptr)
                        lastChild->m_next_sibling = node;
                }
            }
            else
            {
                parent->m_child_count++;
                if (parent->m_first_child == nullptr)
                {
                    parent->m_first_child = node;
                }
                else
                {
                    node_t* last = parent->m_first_child;
                    while (last->m_next_sibling != nullptr)
                        last = last->m_next_sibling;
                    last->m_next_sibling = node;
                    node->m_prev_sibling = last;
                }
            }
        }

    private:
        Allocator m_alloc;
        node_t*   m_root;
    };

    /*
     Generic tree node template class
     */
    template <class T>
    class gtree_node
    {
        friend class stdext::gtree<T>;
    public:
        typedef gtree_node<T>  node_t;
        typedef stdext::gtree<T> tree_t;
    public:
        gtree_node(tree_t* const tree)
            : m_tree(tree),
            m_parent(nullptr),
            m_prev_sibling(nullptr), m_next_sibling(nullptr),
            m_first_child(nullptr),
            m_child_count(0)
        { }

        gtree_node(tree_t* const tree, const T& data)
            : m_tree(tree),
            m_parent(nullptr),
            m_prev_sibling(nullptr), m_next_sibling(nullptr),
            m_first_child(nullptr),
            m_child_count(0),
            m_data(data)
        { }

        node_t* append_child(const T& data)
        {
            return m_tree->create_node(this, data);
        }

        node_t* append_first_child(const T& data)
        {
            return m_tree->create_node_first(this, data);
        }

        const int child_count()
        {
            return child_count(false);
        }

        const int child_count(bool recursive)
        {
            if (!recursive)
                return m_child_count;
            int result = m_child_count;
            node_t* child = m_first_child;
            while (child != nullptr)
            {
                result += child->child_count(recursive);
                child = child->m_next_sibling;
            }
            return result;
        }

        T& data()
        {
            return m_data;
        }

        const T& data() const
        {
            return m_data;
        }

        node_t* first_child() const
        {
            return m_first_child;
        }

        node_t* last_child() const
        {
            node_t* result = m_first_child;
            if (result != nullptr)
            {
                while (result->m_next_sibling != nullptr)
                    result = result->m_next_sibling;
            }
            return result;
        }

        node_t* parent() const
        {
            return m_parent;
        }

        node_t* prev_sibling() const
        {
            return m_prev_sibling;
        }

        node_t* next_sibling() const
        {
            return m_next_sibling;
        }

        tree_t* tree() const
        {
            return m_tree;
        }

        bool is_leaf() const
        {
            return m_first_child == nullptr;
        }

    protected:
        tree_t* m_tree = nullptr;
        int     m_child_count = 0;
        T       m_data;
        node_t* m_first_child = nullptr;
        node_t* m_parent = nullptr;
        node_t* m_prev_sibling = nullptr;
        node_t* m_next_sibling = nullptr;
    };

}

