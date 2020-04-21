#include "variants.h"
#include <limits>
#include "strutils.h"

using namespace std;
using namespace stdext;
using namespace variants;

std::string variants::to_string(const value_type vtype)
{
    switch (vtype)
    {
    case value_type::vt_bool: return "vt_bool";
    case value_type::vt_double: return "vt_double";
    case value_type::vt_int: return "vt_int";
    case value_type::vt_long: return "vt_long";
    case value_type::vt_object: return "vt_object";
    case value_type::vt_string: return "vt_string";
    case value_type::vt_wstring: return "vt_wstring";
    case value_type::vt_unknown: return "vt_unknown";
    default:
        return "unsupported";
    }
}

std::string variants::to_string(const operation op)
{
    switch (op)
    {
    case operation::add: return "add";
    case operation::divide: return "divide";
    case operation::multiply: return "multiply";
    case operation::subtract: return "subtract";
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
    case value_type::vt_long:
        *this = source.to_long();
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

variant::variant(const long val) { *this = val; }
variant& variant::operator=(const long source)
{
    m_vtype = value_type::vt_long;
    m_value = new long(source);
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
        case value_type::vt_long:
            delete (long*)m_value;
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
        else if (v1.is_vtype(value_type::vt_long) || v2.is_vtype(value_type::vt_long))
            return internal_do_binary_op_comparision<long>(v1, v2, op);
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
        else if (v1.is_vtype(value_type::vt_long) || v2.is_vtype(value_type::vt_long))
            return internal_do_binary_op_arithmetic<long>(v1, v2, op);
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
    case value_type::vt_long:
    {
        long n = *((long*)m_value);
        if (n > std::numeric_limits<int>::max() || n < std::numeric_limits<int>::min())
            throw variant_exception(err_msg_convertion_failed(m_vtype, "int"), variant_error::conversion_failed_number_out_of_limits);
        return static_cast<int>(n);
    }
    default:
        throw variant_exception(err_msg_convertion_failed(m_vtype, "int"), variant_error::conversion_failed);
    }
}

long variant::to_long() const noexcept(false)
{
    switch (m_vtype)
    { 
    case value_type::vt_int:
        return static_cast<long>(*((int*)m_value));
    case value_type::vt_long:
        return *((long*)m_value);
    default:
        throw variant_exception(err_msg_convertion_failed(m_vtype, "long"), variant_error::conversion_failed);
    }
}

double variant::to_double() const noexcept(false)
{
    switch (m_vtype)
    {
    case value_type::vt_int:
        return static_cast<double>(*((int*)m_value));
    case value_type::vt_long:
        return static_cast<double>(*((long*)m_value));
    case value_type::vt_double:
        return *((double*)m_value);
    default:
        throw variant_exception(err_msg_convertion_failed(m_vtype, "double"), variant_error::conversion_failed);
    }
}
