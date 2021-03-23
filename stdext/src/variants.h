/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 Minimalistic implementation of variant values
 */
#pragma once

#include <exception>
#include <string>
#include <cstdint>
#include "datetime.h"

namespace stdext::variants
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
            : exception(), m_message(message), m_error(error)
        {}
        variant_exception(const std::string& message, const variant_error error) noexcept
            : exception(), m_message(message), m_error(error)
        {}
        virtual const char* what() const noexcept override { return m_message.c_str(); }
        variant_error error() const noexcept { return m_error; }
    protected:
        std::string m_message;
        variant_error m_error;
    };

    enum class value_type
    {
        // Insert new operations after vt_unknown and before vt_object only
        vt_unknown,
        vt_bool,
        vt_datetime,
        vt_double,
        vt_int,
        vt_int64,
        vt_string,
        vt_wstring,
        vt_object
    };
    inline value_type first_value_type() { return value_type::vt_unknown; }
    inline value_type last_value_type() { return value_type::vt_object; }
    std::string to_string(const value_type vtype);
    inline value_type to_value_type(bool) { return value_type::vt_bool; }
    inline value_type to_value_type(stdext::datetime) { return value_type::vt_double; }
    inline value_type to_value_type(double) { return value_type::vt_double; }
    inline value_type to_value_type(int) { return value_type::vt_int; }
    inline value_type to_value_type(int64_t) { return value_type::vt_int64; }
    inline value_type to_value_type(float) { return value_type::vt_double; }
    inline value_type to_value_type(std::string) { return value_type::vt_string; }
    inline value_type to_value_type(std::wstring) { return value_type::vt_wstring; }

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
    inline operation first_operation() { return operation::unknown; }
    inline operation last_operation() { return operation::nop; }
    std::string to_string(const operation op);
    bool is_comparision(variants::operation op);
    bool is_logical_op(variants::operation op);


    class objref
    {
        friend class variant;
    public:
        objref()
        {
            add_ref();
        }
        virtual ~objref() {}
    protected:
        void add_ref() noexcept { m_refcount++; }
        virtual void release() noexcept
        {
            if (--m_refcount == 0)
                delete this;
        }
    protected:
        int m_refcount = 0;
    };


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
        variant(const int64_t val);
        variant& operator=(const int64_t source);
        variant(const stdext::datetime& val);
        variant& operator=(const stdext::datetime& source);
        variant(const double val);
        variant& operator=(const double source);
        variant(const std::string val);
        variant& operator=(const std::string source);
        variant(const char* val);
        variant& operator=(const char* source);
        variant(const std::wstring val);
        variant& operator=(const std::wstring source);
        variant(const wchar_t* val);
        variant& operator=(const wchar_t* source);
        variant(objref* const val);
        variant& operator=(objref* const source);
        virtual ~variant();
    public:
        // cast operators
        operator bool() const { return to_bool(); }
        operator stdext::datetime() const { return to_datetime(); }
        operator double() const { return to_double(); }
        operator int() const { return to_int(); }
        operator int64_t() const { return to_int64(); }
        operator std::string() const { return to_string(); }
        operator std::wstring() const { return to_wstring(); }
    public:
        // convertion functions
        bool to_bool() const noexcept(false);
        stdext::datetime& to_datetime() const noexcept(false);
        double to_double() const noexcept(false);
        int to_int() const noexcept(false);
        int64_t to_int64() const noexcept(false);
        std::string to_string() const noexcept(false);
        std::wstring to_wstring() const noexcept(false);
        template <class T>
        T* to_object() const { return static_cast<T*>(m_value); }
    public:
        // Operations
        // variant
        friend bool operator ==(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::cmp_eq); }
        friend bool operator !=(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::cmp_neq); }
        friend bool operator >(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::cmp_gt); }
        friend bool operator >=(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::cmp_ge); }
        friend bool operator <(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::cmp_lt); }
        friend bool operator <=(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::cmp_le); }
        bool equal(const variant& value) const noexcept(false) { return do_operation(*this, value, operation::cmp_eq); }
        friend variant operator +(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::add); }
        friend variant operator -(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::subtract); }
        friend variant operator *(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::multiply); }
        friend variant operator /(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::divide); }
        friend variant operator &&(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::b_and); }
        friend variant operator ||(const variant& v1, const variant& v2) { return do_operation(v1, v2, operation::b_or); }
        // bool
        friend bool operator ==(const variant& v1, bool v2) { return v1.to_bool() == v2; }
        friend bool operator !=(const variant& v1, bool v2) { return v1.to_bool() != v2; }
        friend bool operator ==(const bool v1, const variant& v2) { return v1 == v2.to_bool(); }
        friend bool operator !=(const bool v1, const variant& v2) { return v1 != v2.to_bool(); }
        bool equal(const bool value) const noexcept(false) { return to_bool() == value; }
        friend bool operator &&(const variant& v1, bool v2) { return v1.to_bool() && v2; }
        friend bool operator &&(const bool v1, const variant& v2) { return v1 && v2.to_bool(); }
        friend bool operator ||(const variant& v1, bool v2) { return v1.to_bool() || v2; }
        friend bool operator ||(const bool v1, const variant& v2) { return v1 || v2.to_bool(); }
        // int
        friend bool operator ==(const variant& v1, const int v2) { return v1.to_int64() == v2; }
        friend bool operator !=(const variant& v1, const int v2) { return v1.to_int64() != v2; }
        friend bool operator >(const variant& v1, const int v2) { return v1.to_int64() > v2; }
        friend bool operator >=(const variant& v1, const int v2) { return v1.to_int64() >= v2; }
        friend bool operator <(const variant& v1, const int v2) { return v1.to_int64() < v2; }
        friend bool operator <=(const variant& v1, const int v2) { return v1.to_int64() <= v2; }
        friend bool operator ==(const int v1, const variant& v2) { return v1 == v2.to_int64(); }
        friend bool operator !=(const int v1, const variant& v2) { return v1 != v2.to_int64(); }
        friend bool operator >(const int v1, const variant& v2) { return v1 > v2.to_int64(); }
        friend bool operator >=(const int v1, const variant& v2) { return v1 >= v2.to_int64(); }
        friend bool operator <(const int v1, const variant& v2) { return v1 < v2.to_int64(); }
        friend bool operator <=(const int v1, const variant& v2) { return v1 <= v2.to_int64(); }
        bool equal(const int value) const noexcept(false) { return to_int64() == value; }
        friend variant operator +(const variant& v1, const int v2) { return do_operation(v1, v2, operation::add); }
        friend variant operator -(const variant& v1, const int v2) { return do_operation(v1, v2, operation::subtract); }
        friend variant operator *(const variant& v1, const int v2) { return do_operation(v1, v2, operation::multiply); }
        friend variant operator /(const variant& v1, const int v2) { return do_operation(v1, v2, operation::divide); }
        friend variant operator +(const int v1, const variant& v2) { return do_operation(v1, v2, operation::add); }
        friend variant operator -(const int v1, const variant& v2) { return do_operation(v1, v2, operation::subtract); }
        friend variant operator *(const int v1, const variant& v2) { return do_operation(v1, v2, operation::multiply); }
        friend variant operator /(const int v1, const variant& v2) { return do_operation(v1, v2, operation::divide); }
        // int64
        friend bool operator ==(const variant& v1, const int64_t v2) { return v1.to_int64() == v2; }
        friend bool operator !=(const variant& v1, const int64_t v2) { return v1.to_int64() != v2; }
        friend bool operator >(const variant& v1, const int64_t v2) { return v1.to_int64() > v2; }
        friend bool operator >=(const variant& v1, const int64_t v2) { return v1.to_int64() >= v2; }
        friend bool operator <(const variant& v1, const int64_t v2) { return v1.to_int64() < v2; }
        friend bool operator <=(const variant& v1, const int64_t v2) { return v1.to_int64() <= v2; }
        friend bool operator ==(const int64_t v1, const variant& v2) { return v1 == v2.to_int64(); }
        friend bool operator !=(const int64_t v1, const variant& v2) { return v1 != v2.to_int64(); }
        friend bool operator >(const int64_t v1, const variant& v2) { return v1 > v2.to_int64(); }
        friend bool operator >=(const int64_t v1, const variant& v2) { return v1 >= v2.to_int64(); }
        friend bool operator <(const int64_t v1, const variant& v2) { return v1 < v2.to_int64(); }
        friend bool operator <=(const int64_t v1, const variant& v2) { return v1 <= v2.to_int64(); }
        bool equal(const int64_t value) const noexcept(false) { return to_int64() == value; }
        friend variant operator +(const variant v1, const int64_t v2) { return do_operation(v1, v2, operation::add); }
        friend variant operator -(const variant v1, const int64_t v2) { return do_operation(v1, v2, operation::subtract); }
        friend variant operator *(const variant v1, const int64_t v2) { return do_operation(v1, v2, operation::multiply); }
        friend variant operator /(const variant v1, const int64_t v2) { return do_operation(v1, v2, operation::divide); }
        friend variant operator +(const int64_t v1, const variant v2) { return do_operation(v1, v2, operation::add); }
        friend variant operator -(const int64_t v1, const variant v2) { return do_operation(v1, v2, operation::subtract); }
        friend variant operator *(const int64_t v1, const variant v2) { return do_operation(v1, v2, operation::multiply); }
        friend variant operator /(const int64_t v1, const variant v2) { return do_operation(v1, v2, operation::divide); }
        // double
        friend bool operator ==(const variant& v1, const double v2) { return v1.to_double() == v2; }
        friend bool operator !=(const variant& v1, const double v2) { return v1.to_double() != v2; }
        friend bool operator >(const variant& v1, const double v2) { return v1.to_double() > v2; }
        friend bool operator >=(const variant& v1, const double v2) { return v1.to_double() >= v2; }
        friend bool operator <(const variant& v1, const double v2) { return v1.to_double() < v2; }
        friend bool operator <=(const variant& v1, const double v2) { return v1.to_double() <= v2; }
        friend bool operator ==(const double v1, const variant& v2) { return v1 == v2.to_double(); }
        friend bool operator !=(const double v1, const variant& v2) { return v1 != v2.to_double(); }
        friend bool operator >(const double v1, const variant& v2) { return v1 > v2.to_double(); }
        friend bool operator >=(const double v1, const variant& v2) { return v1 >= v2.to_double(); }
        friend bool operator <(const double v1, const variant& v2) { return v1 < v2.to_double(); }
        friend bool operator <=(const double v1, const variant& v2) { return v1 <= v2.to_double(); }
        bool equal(const double value) const noexcept(false) { return to_double() == value; }
        friend double operator +(const variant v1, const double v2) { return v1.to_double() + v2; }
        friend double operator -(const variant v1, const double v2) { return v1.to_double() - v2; }
        friend double operator *(const variant v1, const double v2) { return v1.to_double() * v2; }
        friend double operator /(const variant v1, const double v2) { return v1.to_double() / v2; }
        friend double operator +(const double v1, const variant v2) { return v1 + v2.to_double(); }
        friend double operator -(const double v1, const variant v2) { return v1 - v2.to_double(); }
        friend double operator *(const double v1, const variant v2) { return v1 * v2.to_double(); }
        friend double operator /(const double v1, const variant v2) { return v1 / v2.to_double(); }
        // string
        friend bool operator ==(const variant& v1, const std::string& v2) { return v1.to_string() == v2; }
        friend bool operator !=(const variant& v1, const std::string& v2) { return v1.to_string() != v2; }
        friend bool operator >(const variant& v1, const std::string& v2) { return v1.to_string() > v2; }
        friend bool operator >=(const variant& v1, const std::string& v2) { return v1.to_string() >= v2; }
        friend bool operator <(const variant& v1, const std::string& v2) { return v1.to_string() < v2; }
        friend bool operator <=(const variant& v1, const std::string& v2) { return v1.to_string() <= v2; }
        friend bool operator ==(const std::string& v1, const variant& v2) { return v1 == v2.to_string(); }
        friend bool operator !=(const std::string& v1, const variant& v2) { return v1 != v2.to_string(); }
        friend bool operator >(const std::string& v1, const variant& v2) { return v1 > v2.to_string(); }
        friend bool operator >=(const std::string& v1, const variant& v2) { return v1 >= v2.to_string(); }
        friend bool operator <(const std::string& v1, const variant& v2) { return v1 < v2.to_string(); }
        friend bool operator <=(const std::string& v1, const variant& v2) { return v1 <= v2.to_string(); }
        bool equal(const std::string& value) const noexcept(false) { return to_string() == value; }
        friend std::string operator +(const variant& v1, const std::string& v2) { return v1.to_string() + v2; }
        friend std::string operator +(const std::string& v1, const variant& v2) { return v1 + v2.to_string(); }
        // char*
        friend bool operator ==(const variant& v1, const char* v2) { return v1.to_string() == v2; }
        friend bool operator !=(const variant& v1, const char* v2) { return v1.to_string() != v2; }
        friend bool operator >(const variant& v1, const char* v2) { return v1.to_string() > v2; }
        friend bool operator >=(const variant& v1, const char* v2) { return v1.to_string() >= v2; }
        friend bool operator <(const variant& v1, const char* v2) { return v1.to_string() < v2; }
        friend bool operator <=(const variant& v1, const char* v2) { return v1.to_string() <= v2; }
        friend bool operator ==(const char* v1, const variant& v2) { return v1 == v2.to_string(); }
        friend bool operator !=(const char* v1, const variant& v2) { return v1 != v2.to_string(); }
        friend bool operator >(const char* v1, const variant& v2) { return v1 > v2.to_string(); }
        friend bool operator >=(const char* v1, const variant& v2) { return v1 >= v2.to_string(); }
        friend bool operator <(const char* v1, const variant& v2) { return v1 < v2.to_string(); }
        friend bool operator <=(const char* v1, const variant& v2) { return v1 <= v2.to_string(); }
        bool equal(const char* value) const noexcept(false) { return to_string() == value; }
        friend std::string operator +(const variant& v1, const char* v2) { return v1.to_string() + v2; }
        friend std::string operator +(const char* v1, const variant& v2) { return v1 + v2.to_string(); }
        // wstring
        friend bool operator ==(const variant& v1, const std::wstring& v2) { return v1.to_wstring() == v2; }
        friend bool operator !=(const variant& v1, const std::wstring& v2) { return v1.to_wstring() != v2; }
        friend bool operator >(const variant& v1, const std::wstring& v2) { return v1.to_wstring() > v2; }
        friend bool operator >=(const variant& v1, const std::wstring& v2) { return v1.to_wstring() >= v2; }
        friend bool operator <(const variant& v1, const std::wstring& v2) { return v1.to_wstring() < v2; }
        friend bool operator <=(const variant& v1, const std::wstring& v2) { return v1.to_wstring() <= v2; }
        friend bool operator ==(const std::wstring& v1, const variant& v2) { return v1 == v2.to_wstring(); }
        friend bool operator !=(const std::wstring& v1, const variant& v2) { return v1 != v2.to_wstring(); }
        friend bool operator >(const std::wstring& v1, const variant& v2) { return v1 > v2.to_wstring(); }
        friend bool operator >=(const std::wstring& v1, const variant& v2) { return v1 >= v2.to_wstring(); }
        friend bool operator <(const std::wstring& v1, const variant& v2) { return v1 < v2.to_wstring(); }
        friend bool operator <=(const std::wstring& v1, const variant& v2) { return v1 <= v2.to_wstring(); }
        bool equal(const std::wstring& value) const noexcept(false) { return to_wstring() == value; }
        friend std::wstring operator +(const variant& v1, const std::wstring& v2) { return v1.to_wstring() + v2; }
        friend std::wstring operator +(const std::wstring& v1, const variant& v2) { return v1 + v2.to_wstring(); }
        // wchar_t*
        friend bool operator ==(const variant& v1, const wchar_t* v2) { return v1.to_wstring() == v2; }
        friend bool operator !=(const variant& v1, const wchar_t* v2) { return v1.to_wstring() != v2; }
        friend bool operator >(const variant& v1, const wchar_t* v2) { return v1.to_wstring() > v2; }
        friend bool operator >=(const variant& v1, const wchar_t* v2) { return v1.to_wstring() >= v2; }
        friend bool operator <(const variant& v1, const wchar_t* v2) { return v1.to_wstring() < v2; }
        friend bool operator <=(const variant& v1, const wchar_t* v2) { return v1.to_wstring() <= v2; }
        friend bool operator ==(const wchar_t* v1, const variant& v2) { return v1 == v2.to_wstring(); }
        friend bool operator !=(const wchar_t* v1, const variant& v2) { return v1 != v2.to_wstring(); }
        friend bool operator >(const wchar_t* v1, const variant& v2) { return v1 > v2.to_wstring(); }
        friend bool operator >=(const wchar_t* v1, const variant& v2) { return v1 >= v2.to_wstring(); }
        friend bool operator <(const wchar_t* v1, const variant& v2) { return v1 < v2.to_wstring(); }
        friend bool operator <=(const wchar_t* v1, const variant& v2) { return v1 <= v2.to_wstring(); }
        bool equal(const wchar_t* value) const noexcept(false) { return to_wstring() == value; }
        friend std::wstring operator +(const variant& v1, const wchar_t* v2) { return v1.to_wstring() + v2; }
        friend std::wstring operator +(const wchar_t* v1, const variant& v2) { return v1 + v2.to_wstring(); }
    public:
        void clear();
        template <class T> void clear();
        value_type vtype() const noexcept { return m_vtype; }
        inline bool is_bool() const noexcept { return is_vtype(value_type::vt_bool); }
        inline bool is_datetime() const noexcept { return is_vtype(value_type::vt_datetime); }
        inline bool is_float() const noexcept { return is_vtype(value_type::vt_double); }
        inline bool is_integer_numeric() const noexcept { return is_vtype({ value_type::vt_int, value_type::vt_int64 }); }
        inline bool is_null() const noexcept { return is_vtype(value_type::vt_unknown); }
        inline bool is_numeric() const noexcept { return is_vtype({ value_type::vt_int, value_type::vt_int64, value_type::vt_double }); }
        inline bool is_string() const noexcept { return is_vtype({ value_type::vt_string, value_type::vt_wstring }); }
    protected:
        inline bool is_vtype(const value_type vtype) const noexcept { return m_vtype == vtype; }
        bool is_vtype(const std::initializer_list<value_type> vtypes) const noexcept;
        static variant do_operation(const variant& v1, const variant& v2, const operation op) noexcept(false);
    protected:
        value_type m_vtype = value_type::vt_unknown;
        void* m_value = nullptr;
    };

    variant null();

    template<class T>
    void var_to_optional(const variants::variant& v, T& opt)
    {
        if (!v.is_null())
            opt = v;
        else
            opt.reset();
    }

    template<class T>
    variants::variant optional_to_var(const T& opt)
    {
        if (opt.has_value())
            return opt.value();
        else
            return variants::null();
    }

}



