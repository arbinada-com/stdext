#include "variants.h"
#include <limits>
#include "strutils.h"

using namespace std;
using namespace stdext;
using namespace variants;

std::string variants::to_string(const value_type vtype)
{
#define CASE_VTYPE_STR(name) case value_type::name: return #name
    switch (vtype)
    {
        CASE_VTYPE_STR(vt_bool);
        CASE_VTYPE_STR(vt_double);
        CASE_VTYPE_STR(vt_int);
        CASE_VTYPE_STR(vt_int64);
        CASE_VTYPE_STR(vt_object);
        CASE_VTYPE_STR(vt_string);
        CASE_VTYPE_STR(vt_wstring);
        CASE_VTYPE_STR(vt_unknown);
    default:
        return "unsupported";
    }
}

std::string variants::to_string(const operation op)
{
#define CASE_OP_STR(name) case operation::name: return #name
    switch (op)
    {
        CASE_OP_STR(unknown);
        CASE_OP_STR(add);
        CASE_OP_STR(divide);
        CASE_OP_STR(multiply);
        CASE_OP_STR(subtract);
        CASE_OP_STR(cmp_eq);
        CASE_OP_STR(cmp_neq);
        CASE_OP_STR(cmp_gt);
        CASE_OP_STR(cmp_ge);
        CASE_OP_STR(cmp_lt);
        CASE_OP_STR(cmp_le);
        CASE_OP_STR(b_and);
        CASE_OP_STR(b_or);
        CASE_OP_STR(b_not);
        CASE_OP_STR(nop);
    default:
        return "unsupported";
    }
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
    default:
        throw variant_exception(
            strutils::format("Unsupported type %s (%d)", to_string(source.m_vtype).c_str(), static_cast<int>(source.m_vtype)),
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

variant::~variant()
{
    clear();
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
        default:
            delete m_value;
            break;
        }
        m_value = nullptr;
    }
    m_vtype = value_type::vt_unknown;
}


/*
 * Comparison functions
 */
std::string err_msg_unsupported_comparision(const value_type vtype1, const value_type vtype2) noexcept
{
    return strutils::format(
        "Unsupported comparision betweent type %s (%d) and %s (%d)",
        to_string(vtype1).c_str(),
        static_cast<int>(vtype1),
        to_string(vtype2).c_str(),
        static_cast<int>(vtype2)
    );
}

/*
 * Arithmetic operations
 */
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
        op == operation::b_or ;

}

string err_msg_unsupported_operation(const operation op, const value_type vtype1, const value_type vtype2) noexcept
{
    return strutils::format(
        "Unsupported operation %s betweent type %s (%d) and %s (%d)",
        to_string(op).c_str(),
        to_string(vtype1).c_str(),
        static_cast<int>(vtype1),
        to_string(vtype2).c_str(),
        static_cast<int>(vtype2)
    );
}

template <class VType>
VType internal_do_binary_op_arithmetic(const VType& v1, const VType& v2, operation op)
{
    switch (op)
    {
    case operation::add:
        return v1 + v2;
    case operation::subtract:
        return v1 - v2;
    case operation::multiply:
        return v1 * v2;
    case operation::divide:
        return v1 / v2;
    default:
        throw variant_exception(
            strutils::format("Unsupported arithmetic operation '%s'", to_string(op).c_str()),
            variant_error::operation_failed_binary);
    }
}

template <class VType>
bool internal_do_binary_op_comparision(const VType& v1, const VType& v2, operation op)
{
    switch (op)
    {
    case operation::cmp_eq:
        return v1 == v2;
    case operation::cmp_ge:
        return v1 >= v2;
    case operation::cmp_gt:
        return v1 > v2;
    case operation::cmp_le:
        return v1 <= v2;
    case operation::cmp_lt:
        return v1 < v2;
    case operation::cmp_neq:
        return v1 != v2;
    default:
        throw variant_exception(
            strutils::format("Unsupported comparision operation '%s'", to_string(op).c_str()),
            variant_error::operation_failed_binary);
    }
}

bool internal_do_binary_op_boolean(const bool v1, const bool v2, operation op)
{
    switch (op)
    {
    case operation::b_and:
        return v1 && v2;
    case operation::b_or:
        return v1 || v2;
    default:
        throw variant_exception(
            strutils::format("Unsupported boolean operation '%s'", to_string(op).c_str()),
            variant_error::operation_failed_binary);
    }
}

bool variant::do_operation_cmp(const variant& v1, const variant& v2, const operation op) noexcept(false)
{
    if (v1.is_vtype(value_type::vt_bool) && v2.is_vtype(value_type::vt_bool))
    {
        return internal_do_binary_op_comparision<bool>(v1, v2, op);
    }
    else if (v1.is_numeric() && v2.is_numeric())
    {
        if (v1.is_vtype(value_type::vt_double) || v2.is_vtype(value_type::vt_double))
            return internal_do_binary_op_comparision<double>(v1, v2, op);
        else if (v1.is_vtype(value_type::vt_int64) || v2.is_vtype(value_type::vt_int64))
            return internal_do_binary_op_comparision<int64_t>(v1, v2, op);
        else
            return internal_do_binary_op_comparision<int>(v1, v2, op);
    }
    throw variant_exception(err_msg_unsupported_operation(op, v1.vtype(), v2.vtype()), variant_error::operation_failed_binary);
}

variant variant::do_operation(const variant& v1, const variant& v2, const operation op) noexcept(false)
{
    if (v1.is_vtype(value_type::vt_bool) && v2.is_vtype(value_type::vt_bool))
    {
        if (is_logical_op(op))
            return internal_do_binary_op_boolean(v1, v2, op);
    }
    else if (v1.is_numeric() && v2.is_numeric())
    {
        if (v1.is_vtype(value_type::vt_double) || v2.is_vtype(value_type::vt_double))
            return internal_do_binary_op_arithmetic<double>(v1, v2, op);
        else if (v1.is_vtype(value_type::vt_int64) || v2.is_vtype(value_type::vt_int64))
            return internal_do_binary_op_arithmetic<int64_t>(v1, v2, op);
        else
            return internal_do_binary_op_arithmetic<int>(v1, v2, op);
    }
    throw variant_exception(err_msg_unsupported_operation(op, v1.vtype(), v2.vtype()), variant_error::operation_failed_binary);
}

/*
 * Convertion functions
 */
string err_msg_convertion_failed(const value_type vtype, const char* to_type_name) noexcept
{
    return strutils::format(
        "Conversion failed from type %s (%d) to '%s'", 
        to_string(vtype).c_str(),
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

bool variant::to_bool() const
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
