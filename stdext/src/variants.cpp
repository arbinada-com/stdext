/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "variants.h"
#include <cmath>
#include <limits>
#include "strutils.h"

using namespace std;

namespace stdext
{
namespace variants
{

#define VARIANTS_CASE_VTYPE_STR(name) case value_type::name: return #name
#define VARIANTS_CASE_OP_STR(name) case operation::name: return #name

std::string to_string(const value_type vtype)
{
    switch (vtype)
    {
        VARIANTS_CASE_VTYPE_STR(vt_bool);
        VARIANTS_CASE_VTYPE_STR(vt_double);
        VARIANTS_CASE_VTYPE_STR(vt_int);
        VARIANTS_CASE_VTYPE_STR(vt_int64);
        VARIANTS_CASE_VTYPE_STR(vt_object);
        VARIANTS_CASE_VTYPE_STR(vt_string);
        VARIANTS_CASE_VTYPE_STR(vt_wstring);
        VARIANTS_CASE_VTYPE_STR(vt_unknown);
    default:
        return "unsupported";
    }
}

std::string to_string(const operation op)
{
    switch (op)
    {
        VARIANTS_CASE_OP_STR(unknown);
        VARIANTS_CASE_OP_STR(add);
        VARIANTS_CASE_OP_STR(divide);
        VARIANTS_CASE_OP_STR(multiply);
        VARIANTS_CASE_OP_STR(subtract);
        VARIANTS_CASE_OP_STR(cmp_eq);
        VARIANTS_CASE_OP_STR(cmp_neq);
        VARIANTS_CASE_OP_STR(cmp_gt);
        VARIANTS_CASE_OP_STR(cmp_ge);
        VARIANTS_CASE_OP_STR(cmp_lt);
        VARIANTS_CASE_OP_STR(cmp_le);
        VARIANTS_CASE_OP_STR(b_and);
        VARIANTS_CASE_OP_STR(b_or);
        VARIANTS_CASE_OP_STR(b_not);
        VARIANTS_CASE_OP_STR(nop);
    default:
        return "unsupported";
    }
}

bool is_comparision(variants::operation op)
{
    return
        op == operation::cmp_eq ||
        op == operation::cmp_ge ||
        op == operation::cmp_gt ||
        op == operation::cmp_le ||
        op == operation::cmp_lt ||
        op == operation::cmp_neq;
}

bool is_logical_op(variants::operation op)
{
    return
        op == operation::b_and ||
        op == operation::b_not ||
        op == operation::b_or;
}


/*
 * Initializations
 */
variant::variant()
    : m_vtype(value_type::vt_unknown), m_value(nullptr)
{ }

variant::variant(const variant& source) { *this = source; }
variant& variant::operator=(const variants::variant& source)
{
    if (this == &source)
        return *this;
    clear();
    switch (source.m_vtype)
    {
    case value_type::vt_bool:
        *this = source.to_bool();
        break;
    case value_type::vt_int:
        *this = source.to_int();
        break;
    case value_type::vt_int64:
        *this = source.to_int64();
        break;
    case value_type::vt_double:
        *this = source.to_double();
        break;
    case value_type::vt_string:
        *this = source.to_string();
        break;
    case value_type::vt_wstring:
        *this = source.to_wstring();
        break;
    case value_type::vt_object:
        m_vtype = source.m_vtype;
        m_value = source.m_value;
        static_cast<objref*>(m_value)->add_ref();
        break;
    default:
        throw variant_exception(
            str::format("Unsupported type %s (%d)", variants::to_string(source.m_vtype).c_str(), static_cast<int>(source.m_vtype)),
            variant_error::conversion_failed);
    }
    return *this;
}

variant::variant(variant&& source) { *this = std::move(source); }
variant& variant::operator=(variant&& source)
{
    if (this == &source)
        return *this;
    clear();
    m_vtype = source.m_vtype;
    m_value = source.m_value;
    source.m_value = nullptr;
    return *this;
}

variant::variant(const bool val) { *this = val; }
variant& variant::operator=(const bool source)
{
    m_vtype = value_type::vt_bool;
    m_value = new bool(source);
    return *this;
}

variant::variant(const int val) { *this = val; }
variant& variant::operator=(const int source)
{
    m_vtype = value_type::vt_int;
    m_value = new int(source);
    return *this;
}

variant::variant(const int64_t val) { *this = val; }
variant& variant::operator=(const int64_t source)
{
    m_vtype = value_type::vt_int64;
    m_value = new int64_t(source);
    return *this;
}

variant::variant(const double val) { *this = val; }
variant& variant::operator=(const double source)
{
    m_vtype = value_type::vt_double;
    m_value = new double(source);
    return *this;
}

variant::variant(const std::string val) { *this = val; }
variant& variant::operator=(const std::string source)
{
    m_vtype = value_type::vt_string;
    m_value = new std::string(source);
    return *this;
}

variant::variant(const char* val)
    : variant(std::string(val))
{ }
variant& variant::operator=(const char* source)
{
    m_vtype = value_type::vt_string;
    m_value = new std::string(source);
    return *this;
}

variant::variant(const std::wstring val) { *this = val; }
variant& variant::operator=(const std::wstring source)
{
    m_vtype = value_type::vt_wstring;
    m_value = new std::wstring(source);
    return *this;
}

variant::variant(const wchar_t* val)
    : variant(std::wstring(val))
{ }
variant& variant::operator=(const wchar_t* source)
{
    m_vtype = value_type::vt_wstring;
    m_value = new std::wstring(source);
    return *this;
}

variant::variant(objref* const val) { *this = val; }
variant& variant::operator=(objref* const source)
{
    m_vtype = value_type::vt_object;
    m_value = static_cast<void*>(source);
    return *this;
}

variant::~variant()
{
    clear();
}

template <class T>
void variant::clear()
{
    if (m_vtype == value_type::vt_object)
        static_cast<objref*>(m_value)->release();
    else
        delete static_cast<T*>(m_value);
    m_value = nullptr;
    m_vtype = value_type::vt_unknown;
}

void variant::clear()
{
    if (m_value != nullptr)
    {
        switch (m_vtype)
        {
        case value_type::vt_bool:
            delete (bool*)m_value;
            break;
        case value_type::vt_int:
            delete (int*)m_value;
            break;
        case value_type::vt_int64:
            delete (int64_t*)m_value;
            break;
        case value_type::vt_double:
            delete (double*)m_value;
            break;
        case value_type::vt_string:
            delete (std::string*)m_value;
            break;
        case value_type::vt_wstring:
            delete (std::wstring*)m_value;
            break;
        case value_type::vt_object:
            static_cast<objref*>(m_value)->release();
            break;
        default:
            break;
        }
        m_value = nullptr;
    }
    m_vtype = value_type::vt_unknown;
}


std::string err_msg_unsupported_comparision(const operation op, const value_type vtype1, const value_type vtype2) noexcept
{
    return str::format(
        "Unsupported comparision %s between type %s (%d) and %s (%d)",
        variants::to_string(op).c_str(),
        variants::to_string(vtype1).c_str(),
        static_cast<int>(vtype1),
        variants::to_string(vtype2).c_str(),
        static_cast<int>(vtype2)
    );
}

string err_msg_unsupported_operation(const operation op, const value_type vtype1, const value_type vtype2) noexcept
{
    return str::format(
        "Unsupported operation %s between type %s (%d) and %s (%d)",
        variants::to_string(op).c_str(),
        variants::to_string(vtype1).c_str(),
        static_cast<int>(vtype1),
        variants::to_string(vtype2).c_str(),
        static_cast<int>(vtype2)
    );
}

value_type deduce_compatible_type(const value_type t1, const value_type t2) noexcept
{
    switch (t1)
    {
    case value_type::vt_bool:
        if (t2 == value_type::vt_bool)
            return t2;
        break;
    case value_type::vt_int:
        if (t2 == value_type::vt_int || t2 == value_type::vt_int64 || t2 == value_type::vt_double)
            return t2;
        break;
    case value_type::vt_int64:
        switch (t2)
        {
        case value_type::vt_int:
        case value_type::vt_int64:
            return value_type::vt_int64;
        case value_type::vt_double:
            return value_type::vt_double;
        default:
            return value_type::vt_unknown;
        }
        break;
    case value_type::vt_double:
        if (t2 == value_type::vt_int || t2 == value_type::vt_int64 || t2 == value_type::vt_double)
            return value_type::vt_double;
        break;
    case value_type::vt_string:
        if (t2 == value_type::vt_string)
            return t2;
        break;
    case value_type::vt_wstring:
        if (t2 == value_type::vt_wstring)
            return t2;
        break;
    default:
        return value_type::vt_unknown;
    }
    return value_type::vt_unknown;
}

value_type deduce_compatible_type(const variant& v1, const variant& v2) noexcept
{
    return deduce_compatible_type(v1.vtype(), v2.vtype());
}

variant variant::do_operation(const variant& v1, const variant& v2, const operation op) noexcept(false)
{
    switch (op)
    {
    // Math
    case operation::add:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_int: return v1.to_int() + v2.to_int();
        case value_type::vt_int64: return v1.to_int64() + v2.to_int64();
        case value_type::vt_double: return v1.to_double() + v2.to_double();
        case value_type::vt_string: return v1.to_string() + v2.to_string();
        case value_type::vt_wstring: return v1.to_string() + v2.to_string();
        default: break;
        }
        break;
    case operation::divide:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_int: return v1.to_int() / v2.to_int();
        case value_type::vt_int64: return v1.to_int64() / v2.to_int64();
        case value_type::vt_double: return v1.to_double() / v2.to_double();
        default: break;
        }
        break;
    case operation::multiply:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_int: return v1.to_int() * v2.to_int();
        case value_type::vt_int64: return v1.to_int64() * v2.to_int64();
        case value_type::vt_double: return v1.to_double() * v2.to_double();
        default: break;
        }
        break;
    case operation::subtract:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_int: return v1.to_int() - v2.to_int();
        case value_type::vt_int64: return v1.to_int64() - v2.to_int64();
        case value_type::vt_double: return v1.to_double() - v2.to_double();
        default: break;
        }
        break;
     // Logical
    case operation::b_and:
        if (deduce_compatible_type(v1, v2) == value_type::vt_bool)
            return v1.to_bool() && v2.to_bool();
        break;
    case operation::b_or:
        if (deduce_compatible_type(v1, v2) == value_type::vt_bool)
            return v1.to_bool() || v2.to_bool();
        break;
    // Comparisions
    case operation::cmp_eq:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_bool: return v1.to_bool() == v2.to_bool();
        case value_type::vt_int: return v1.to_int() == v2.to_int();
        case value_type::vt_int64: return v1.to_int64() == v2.to_int64();
        case value_type::vt_double: return v1.to_double() == v2.to_double();
        case value_type::vt_string: return v1.to_string() == v2.to_string();
        case value_type::vt_wstring: return v1.to_wstring() == v2.to_wstring();
        default: break;
        }
        break;
    case operation::cmp_neq:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_bool: return v1.to_bool() != v2.to_bool();
        case value_type::vt_int: return v1.to_int() != v2.to_int();
        case value_type::vt_int64: return v1.to_int64() != v2.to_int64();
        case value_type::vt_double: return v1.to_double() != v2.to_double();
        case value_type::vt_string: return v1.to_string() != v2.to_string();
        case value_type::vt_wstring: return v1.to_wstring() != v2.to_wstring();
        default: break;
        }
        break;
    case operation::cmp_gt:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_bool: return v1.to_bool() > v2.to_bool();
        case value_type::vt_int: return v1.to_int() > v2.to_int();
        case value_type::vt_int64: return v1.to_int64() > v2.to_int64();
        case value_type::vt_double: return v1.to_double() > v2.to_double();
        case value_type::vt_string: return v1.to_string() > v2.to_string();
        case value_type::vt_wstring: return v1.to_wstring() > v2.to_wstring();
        default: break;
        }
        break;
    case operation::cmp_ge:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_bool: return v1.to_bool() >= v2.to_bool();
        case value_type::vt_int: return v1.to_int() >= v2.to_int();
        case value_type::vt_int64: return v1.to_int64() >= v2.to_int64();
        case value_type::vt_double: return v1.to_double() >= v2.to_double();
        case value_type::vt_string: return v1.to_string() >= v2.to_string();
        case value_type::vt_wstring: return v1.to_wstring() >= v2.to_wstring();
        default: break;
        }
        break;
    case operation::cmp_lt:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_bool: return v1.to_bool() < v2.to_bool();
        case value_type::vt_int: return v1.to_int() < v2.to_int();
        case value_type::vt_int64: return v1.to_int64() < v2.to_int64();
        case value_type::vt_double: return v1.to_double() < v2.to_double();
        case value_type::vt_string: return v1.to_string() < v2.to_string();
        case value_type::vt_wstring: return v1.to_wstring() < v2.to_wstring();
        default: break;
        }
        break;
    case operation::cmp_le:
        switch (deduce_compatible_type(v1, v2))
        {
        case value_type::vt_bool: return v1.to_bool() <= v2.to_bool();
        case value_type::vt_int: return v1.to_int() <= v2.to_int();
        case value_type::vt_int64: return v1.to_int64() <= v2.to_int64();
        case value_type::vt_double: return v1.to_double() <= v2.to_double();
        case value_type::vt_string: return v1.to_string() <= v2.to_string();
        case value_type::vt_wstring: return v1.to_wstring() <= v2.to_wstring();
        default: break;
        }
        break;
    default:
        break;
    }
    if (is_comparision(op))
        throw variant_exception(err_msg_unsupported_comparision(op, v1.vtype(), v2.vtype()), variant_error::operation_failed_binary);
    throw variant_exception(err_msg_unsupported_operation(op, v1.vtype(), v2.vtype()), variant_error::operation_failed_binary);
}

/*
 * Convertion functions
 */
string err_msg_convertion_failed(const value_type vtype, const char* to_type_name) noexcept
{
    return str::format(
        "Conversion failed from type %s (%d) to '%s'",
        variants::to_string(vtype).c_str(),
        static_cast<int>(vtype),
        to_type_name
    );
}

inline bool variant::is_vtype(const value_type vtype) const noexcept
{
    return m_vtype == vtype;
}

bool variant::is_vtype(const std::initializer_list<value_type> vtypes) const noexcept
{
    for (value_type vtype : vtypes)
    {
        if (is_vtype(vtype))
            return true;
    }
    return false;
}

bool variant::to_bool() const noexcept(false)
{
    if (!is_vtype(value_type::vt_bool))
        throw variant_exception(err_msg_convertion_failed(m_vtype, "bool"), variant_error::conversion_failed);
    return *((bool*)m_value);
}

int variant::to_int() const noexcept(false)
{
    switch (m_vtype)
    {
    case value_type::vt_int:
        return static_cast<int>(*((int*)m_value));
    case value_type::vt_int64:
    {
        int64_t n = *((int64_t*)m_value);
        if (n > std::numeric_limits<int>::max() || n < std::numeric_limits<int>::min())
            throw variant_exception(err_msg_convertion_failed(m_vtype, "int"), variant_error::conversion_failed_number_out_of_limits);
        return static_cast<int>(n);
    }
    case value_type::vt_double:
    {
        double ipart;
        modf(*((double*)m_value), &ipart);
        if (ipart > std::numeric_limits<int>::max() || ipart < std::numeric_limits<int>::min())
            throw variant_exception(err_msg_convertion_failed(m_vtype, "int"), variant_error::conversion_failed_number_out_of_limits);
        return static_cast<int>(ipart);
    }
    default:
        throw variant_exception(err_msg_convertion_failed(m_vtype, "int"), variant_error::conversion_failed);
    }
}

int64_t variant::to_int64() const noexcept(false)
{
    switch (m_vtype)
    {
    case value_type::vt_int:
        return static_cast<int64_t>(*((int*)m_value));
    case value_type::vt_int64:
        return *((int64_t*)m_value);
    case value_type::vt_double:
    {
        double ipart;
        modf(*((double*)m_value), &ipart);
        if (ipart > (double)std::numeric_limits<int64_t>::max() || ipart < (double)std::numeric_limits<int64_t>::min())
            throw variant_exception(err_msg_convertion_failed(m_vtype, "int"), variant_error::conversion_failed_number_out_of_limits);
        return static_cast<int64_t>(ipart);
    }
    default:
        throw variant_exception(err_msg_convertion_failed(m_vtype, "int64"), variant_error::conversion_failed);
    }
}

double variant::to_double() const noexcept(false)
{
    switch (m_vtype)
    {
    case value_type::vt_int:
        return static_cast<double>(*((int*)m_value));
    case value_type::vt_int64:
        return static_cast<double>(*((int64_t*)m_value));
    case value_type::vt_double:
        return *((double*)m_value);
    default:
        throw variant_exception(err_msg_convertion_failed(m_vtype, "double"), variant_error::conversion_failed);
    }
}

std::string variant::to_string() const noexcept(false)
{
    switch (m_vtype)
    {
    case value_type::vt_bool:
        return std::to_string(to_bool());
    case value_type::vt_double:
        return std::to_string(to_double());
    case value_type::vt_int:
        return std::to_string(to_int());
    case value_type::vt_int64:
        return std::to_string(to_int64());
    case value_type::vt_string:
        return *((std::string*)m_value);
    case value_type::vt_wstring:
        return str::to_string(*((std::wstring*)m_value));
    default:
        throw variant_exception(err_msg_convertion_failed(m_vtype, "std::string"), variant_error::conversion_failed);
    }
}

std::wstring variant::to_wstring() const noexcept(false)
{
    switch (m_vtype)
    {
    case value_type::vt_bool:
        return std::to_wstring(to_bool());
    case value_type::vt_double:
        return std::to_wstring(to_double());
    case value_type::vt_int:
        return std::to_wstring(to_int());
    case value_type::vt_int64:
        return std::to_wstring(to_int64());
    case value_type::vt_string:
        return str::to_wstring(*((std::string*)m_value));
    case value_type::vt_wstring:
        return *((std::wstring*)m_value);
    default:
        throw variant_exception(err_msg_convertion_failed(m_vtype, "std::wstring"), variant_error::conversion_failed);
    }
}

}
}
