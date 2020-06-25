/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */
#pragma once
#include <vector>
#include <algorithm>

namespace stdext
{

    template<class Type, class Alloc = std::allocator<Type>>
    class ptr_vector
    {
    public:
        typedef std::vector<Type*, Alloc> vector_t;
        typedef typename vector_t::const_iterator const_iterator;
        typedef typename vector_t::iterator iterator;
        typedef typename vector_t::size_type size_type;

        ptr_vector()
        { }

        ptr_vector(ptr_vector&) = delete;
        ptr_vector& operator =(const ptr_vector&) = delete;
        ptr_vector(ptr_vector&& source) { *this = std::move(source); }
        ptr_vector& operator =(ptr_vector&& source)
        {
            m_vector = std::move(source.m_vector);
            return *this;
        }

        ~ptr_vector()
        {
            clear();
        }

        iterator begin(void) { return m_vector.begin(); }
        const_iterator begin(void) const { return m_vector.begin(); }

        iterator end(void) { return m_vector.end(); }
        const_iterator end(void) const { return m_vector.end(); }

        void push_back(Type* const value)
        {
            m_vector.push_back(value);
        }

        const Type* back() const { return m_vector.back(); }
        Type* back() { return m_vector.back(); }

        const Type* front() const { return m_vector.front(); }
        Type* front() { return m_vector.front(); }

        ptr_vector::size_type size() const
        {
            return m_vector.size();
        }

        vector_t *operator->() { return &m_vector; }
        const vector_t *operator->() const { return &m_vector; }

        vector_t& operator * () { return m_vector; }
        const vector_t& operator * () const { return m_vector; }

        Type* &operator[](const size_type index) { return m_vector[index]; }
        Type* const &operator[](const size_type index) const { return m_vector[index]; }

        bool operator == (const ptr_vector& rhs) const
        {
            if (m_vector.size() != rhs.m_vector.size())
                return false;
            typename vector_t::const_iterator lhs_iter = m_vector.begin();
            typename vector_t::const_iterator lhs_end = m_vector.end();
            typename vector_t::const_iterator rhs_iter = rhs.m_vector.begin();
            while (lhs_iter != lhs_end)
            {
                if (!(**lhs_iter == **rhs_iter))
                    return false;
                lhs_iter++;
                rhs_iter++;
            }
            return true;
        }

        Type* at(const size_type index) const
        {
            return m_vector.at(index);
        }

        void clear()
        {
            if (!m_vector.empty())
            {
                for (Type* t : m_vector)
                    delete t;
            }
            m_vector.clear();
        }

        bool empty() const noexcept { return m_vector.empty(); }

        iterator erase(iterator position)
        {
            if (position == end())
                return end();
            Type* t = *position;
            delete t;
            return m_vector.erase(position);
        }

        Type* extract(iterator position)
        {
            if (position == end())
                return nullptr;
            Type* t = *position;
            *position = nullptr;
            m_vector.erase(position);
            return t;
        }
        
        iterator find(const Type* value) 
        { 
            return std::find(m_vector.begin(), m_vector.end(), value); 
        }
        const_iterator find(const Type* value) const 
        {
            return std::find(m_vector.begin(), m_vector.end(), value); 
        }
    protected:
        vector_t m_vector;
    };
}
