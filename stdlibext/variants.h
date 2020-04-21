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
            // Insert new operations after vt_unknown and before vt_object only
            vt_unknown,
            vt_bool,
            vt_double,
            vt_int,
            vt_long,
            vt_string,
            vt_wstring,
            vt_object
        };
        const value_type first_value_type = value_type::vt_unknown;
        const value_type last_value_type = value_type::vt_object;
        std::string to_string(const value_type vtype);
        inline const value_type to_value_type(bool) { return value_type::vt_bool; }
        inline const value_type to_value_type(int) { return value_type::vt_int; }
        inline const value_type to_value_type(long) { return value_type::vt_long; }
        inline const value_type to_value_type(float) { return value_type::vt_double; }
        inline const value_type to_value_type(double) { return value_type::vt_double; }
        inline const value_type to_value_type(std::string) { return value_type::vt_string; }
        inline const value_type to_value_type(std::wstring) { return value_type::vt_wstring; }

        enum class operation
        {
            // Insert new operations after UNKNOWN and before NOP only
            unknown,
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
            nop
        };
        const operation first_operation = operation::unknown;
        const operation last_operation = operation::nop;
        std::string to_string(const operation op);


#define DECLARE_OPERATOR(type1, type2, rettype, opname, optype) \
    friend rettype operator opname(const type1& v1, const type2& v2) { return do_operation(v1, v2, optype); }
#define DECLARE_OPERATOR_CMP(type1, type2, rettype, opname, optype) \
    friend rettype operator opname(const type1& v1, const type2& v2) { return do_operation_cmp(v1, v2, optype); }
#define DECLARE_EQUAL_FUNC(type) \
    bool equal(const type& value) const noexcept(false) { return do_operation_cmp(*this, value, operation::cmp_eq); }

#define DECLARE_OPERATORS_COMPARISION(type1, type2) \
    DECLARE_OPERATOR_CMP(type1, type2, bool, ==, operation::cmp_eq) \
    DECLARE_OPERATOR_CMP(type1, type2, bool, !=, operation::cmp_neq) \
    DECLARE_OPERATOR_CMP(type1, type2, bool, >, operation::cmp_gt) \
    DECLARE_OPERATOR_CMP(type1, type2, bool, >=, operation::cmp_ge) \
    DECLARE_OPERATOR_CMP(type1, type2, bool, <, operation::cmp_lt) \
    DECLARE_OPERATOR_CMP(type1, type2, bool, <=, operation::cmp_le)
#define DECLARE_OPERATORS_COMPARISION_FULL(type) \
    DECLARE_OPERATORS_COMPARISION(variant, type) \
    DECLARE_OPERATORS_COMPARISION(type, variant) \
    DECLARE_EQUAL_FUNC(type)

#define DECLARE_OPERATORS_ARITHMETIC(type1, type2) \
    DECLARE_OPERATOR(type1, type2, variant, +, operation::add) \
    DECLARE_OPERATOR(type1, type2, variant, -, operation::subtract) \
    DECLARE_OPERATOR(type1, type2, variant, *, operation::multiply) \
    DECLARE_OPERATOR(type1, type2, variant, /, operation::divide)
#define DECLARE_OPERATORS_ARITHMETIC_FULL(type) \
    DECLARE_OPERATORS_ARITHMETIC(variant, type)\
    DECLARE_OPERATORS_ARITHMETIC(type, variant)

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
            DECLARE_OPERATORS_COMPARISION(variant, variant)
            DECLARE_EQUAL_FUNC(variant)
            DECLARE_OPERATORS_COMPARISION_FULL(bool)
            DECLARE_OPERATORS_COMPARISION_FULL(int)
            DECLARE_OPERATORS_COMPARISION_FULL(long)
            // Arithmetic operations
            DECLARE_OPERATORS_ARITHMETIC(variant, variant)
            DECLARE_OPERATORS_ARITHMETIC_FULL(int)
            DECLARE_OPERATORS_ARITHMETIC_FULL(long)

        public:
            void clear();
            value_type vtype() const noexcept { return m_vtype; }
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


