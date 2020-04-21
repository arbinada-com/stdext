/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 Minimalistic implementation of variant values 
 */
#pragma once

#include <exception>
#include <string>

namespace stdext
{
    namespace variants
    {

        enum class variant_error
        {
            none = 0,
            conversion_failed = 100,
            conversion_failed_number_out_of_limits = 110,
            comparison_failed = 200,
            operation_failed = 300,
            operation_failed_binary = 310,
            operation_failed_unary = 320,
        };

        class variant_exception : public std::exception
        {
        public:
            variant_exception(const char* message, const variant_error error) noexcept
                : exception(message), 
                m_error(error)
            {}
            variant_exception(const std::string message, const variant_error error) noexcept
                : exception(message.c_str()),
                m_error(error)
            {}
            variant_error error() const noexcept { return m_error; }
        protected:
            variant_error m_error;
        };

        enum class value_type
        {
            vt_unknown,
            vt_bool,
            vt_double,
            vt_int,
            vt_long,
            vt_object,
            vt_string,
            vt_wstring
        };
        std::string to_string(const value_type vtype);

        enum class operation
        {
            add,
            subtract,
            multiply,
            divide,
            cmp_eq,
            cmp_neq,
            cmp_gt,
            cmp_ge,
            cmp_lt,
            cmp_le,
            b_and,
            b_or,
            b_not,
        };
        std::string to_string(const operation op);

#define DECLARE_COMPARISION_OPERATORS_LH(type) \
    friend bool operator==(const variant& v1, const type v2) { return do_operation_cmp(v1, v2, operation::cmp_eq); } \
    friend bool operator!=(const variant& v1, const type v2) { return do_operation_cmp(v1, v2, operation::cmp_neq); } \
    friend bool operator>(const variant& v1, const type v2) { return do_operation_cmp(v1, v2, operation::cmp_gt); } \
    friend bool operator>=(const variant& v1, const type v2) { return do_operation_cmp(v1, v2, operation::cmp_ge); } \
    friend bool operator<(const variant& v1, const type v2) { return do_operation_cmp(v1, v2, operation::cmp_lt); } \
    friend bool operator<=(const variant& v1, const type v2) { return do_operation_cmp(v1, v2, operation::cmp_le); } \
    bool equal(const type& value) const noexcept(false) { return do_operation_cmp(*this, value, operation::cmp_eq); } \
// end
#define DECLARE_COMPARISION_OPERATORS_RH(type) \
    friend bool operator==(const type& v1, const variant& v2) { return do_operation_cmp(v1, v2, operation::cmp_eq); } \
    friend bool operator!=(const type& v1, const variant& v2) { return do_operation_cmp(v1, v2, operation::cmp_neq); } \
    friend bool operator>(const type& v1, const variant& v2) { return do_operation_cmp(v1, v2, operation::cmp_gt); } \
    friend bool operator>=(const type& v1, const variant& v2) { return do_operation_cmp(v1, v2, operation::cmp_ge); } \
    friend bool operator<(const type& v1, const variant& v2) { return do_operation_cmp(v1, v2, operation::cmp_lt); } \
    friend bool operator<=(const type& v1, const variant& v2) { return do_operation_cmp(v1, v2, operation::cmp_le); } \
// end
#define DECLARE_COMPARISION_OPERATORS(type) \
    DECLARE_COMPARISION_OPERATORS_LH(type)\
    DECLARE_COMPARISION_OPERATORS_RH(type)

#define DECLARE_ARITHMETIC_OPERATORS_LH(type) \
    friend variant operator+(const variant& v1, const type& v2) { return do_operation(v1, v2, operation::add); } \
    friend variant operator-(const variant& v1, const type& v2) { return do_operation(v1, v2, operation::subtract); } \
// end
#define DECLARE_ARITHMETIC_OPERATORS_RH(type) \
    friend variant operator+(const type& v1, const variant& v2) { return do_operation(v1, v2, operation::add); } \
    friend variant operator-(const type& v1, const variant& v2) { return do_operation(v1, v2, operation::subtract); } \
// end
#define DECLARE_ARITHMETIC_OPERATORS(type) \
    DECLARE_ARITHMETIC_OPERATORS_LH(type)\
    DECLARE_ARITHMETIC_OPERATORS_RH(type)

        class variant
        {
        public:
            variant();
            variant(const variant& source);
            variant& operator=(const variant& source);
            variant(variant&& source);
            variant& operator=(variant&& source);
            variant(const bool val);
            variant& operator=(const bool source);
            variant(const int val);
            variant& operator=(const int source);
            variant(const long val);
            variant& operator=(const long source);
            variant(const double val);
            variant& operator=(const double source);
            virtual ~variant() noexcept;
        public: 
            // cast operators
            operator bool() const { return to_bool(); }
            operator int() const { return to_int(); }
            operator long() const { return to_long(); }
            operator double() const { return to_double(); }
        public:
            // convertion functions
            bool to_bool() const noexcept(false);
            int to_int() const noexcept(false);
            long to_long() const noexcept(false);
            double to_double() const noexcept(false);
        public:
            // Comparison operations
            DECLARE_COMPARISION_OPERATORS_LH(variant)
            DECLARE_COMPARISION_OPERATORS(bool)
            DECLARE_COMPARISION_OPERATORS(int)
            DECLARE_COMPARISION_OPERATORS(long)
            // Arithmetic operations
            DECLARE_ARITHMETIC_OPERATORS_LH(variant)
            DECLARE_ARITHMETIC_OPERATORS(int)
            DECLARE_ARITHMETIC_OPERATORS(long)

        public:
            void clear();
            value_type vtype() const { return m_vtype; }
            inline bool is_integer_numeric() const noexcept { return is_vtype({ value_type::vt_int, value_type::vt_long }); }
            inline bool is_numeric() const noexcept { return is_vtype({ value_type::vt_int, value_type::vt_long, value_type::vt_double }); }
            inline bool is_string() const noexcept { return is_vtype({ value_type::vt_string, value_type::vt_wstring }); }
        protected:
            inline bool is_vtype(const value_type vtype) const noexcept;
            bool is_vtype(const std::initializer_list<value_type> vtypes) const noexcept;
            static bool do_operation_cmp(const variant& v1, const variant& v2, const operation op) noexcept(false);
            static variant do_operation(const variant& v1, const variant& v2, const operation op) noexcept(false);
        protected:
            void* m_value = nullptr;
            value_type m_vtype = value_type::vt_unknown;
        };

    }
}


