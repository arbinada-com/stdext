/*
 Implementations are based on following sources:
 1. "Practical astronomy with your calculator", Peter Duffet-Smith, 3rd edition, 
    Cambridge University Press, 1988
 2. Julian date (Wikipedia)
 */


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <cstdlib>
#include <cmath>
#include <cwchar>
#include "datetime.h"
#include "strutils.h"

using namespace std;
using namespace stdext;

#if defined(__BCPLUSPLUS__) && defined(__clang__)
double trunc2(const double n)
{
    double ipart;
    double fpart = modf(n, &ipart);
    return ipart;
}
#define trunc(n) trunc2(n)
#endif

/**
 * datetime_exception class
 */
datetime_exception::datetime_exception(const datetime_exception::kind kind)
    : datetime_exception(kind, "")
{ }

datetime_exception::datetime_exception(const datetime_exception::kind kind, const char* value)
    : m_kind(kind)
{
    m_message = strutils::format(msg_by_kind(kind), value);
}

datetime_exception::datetime_exception(const datetime_exception::kind kind, const int value)
    : m_kind(kind)
{ }

const char* datetime_exception::what() const throw()
{
    return m_message.c_str();
}

std::string datetime_exception::msg_by_kind(const datetime_exception::kind kind)
{
    switch(kind)
    {
        case kind::none:
            return "No error";
        case kind::invalid_datetime_string:
            return "Invalid date/time initial string \"%s\"";
        case kind::invalid_datetime_value:
            return "Invalid date/time unit value \"%s\"";
        case kind::invalid_date_part:
            return "Invalid date part. Code = %d";
        case kind::invalid_time_unit:
            return "Invalid time unit. Code = %d";
        case kind::invalid_date_part_for_time_format:
            return "Invalid date part when object has the TIME format. Code = %d";
        case kind::unknown_calendar:
            return "Unknown calendar. Code: %d";
        case kind::not_implemented:
            return "Not implemented";
        default:
            return "Unsupported type";
    }
}



/**
 * datetime class
 */
datetime::datetime()
    : m_jd(0.0), m_cal(calendar_t::julian)
{ }


datetime::datetime(const datepart_t year, const datepart_t month, const datepart_t day,
                   const datepart_t hour, const datepart_t minute, const datepart_t second, 
                   const datepart_t millisecond,
                   const calendar_t cal)
{
    init(year, month, day, hour, minute, second, millisecond, cal);
    if (!is_valid())
    {
        string s = strutils::format("Year: %d, month: %d, day: %d, hour: %d, minute: %d, second: %d",
            year, month, day, hour, minute, second);
        throw datetime_exception(datetime_exception::kind::invalid_datetime_string, s.c_str());
    }
}

datetime::datetime(const datepart_t year, const datepart_t month, const datepart_t day,
                   const calendar_t cal)
    : datetime(year, month, day, 
        (year == -4712 && month == 1 && day == 1 && (cal == calendar_t::date_default || cal == calendar_t::julian) ? 12 : 0), 
        0, 0, 0, cal)
{ }

datetime::datetime(const struct tm &t)
    : datetime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, 0, calendar_t::gregorian)
{ }

datetime::datetime(const char* str)
{
	parse(str);
}

datetime::datetime(const std::wstring& str)
{
    parse(strutils::to_string(str).c_str());
}

datetime::datetime(const wchar_t* str)
    : datetime(wstring(str))
{ }


datetime::datetime(const std::string& str)
{
	parse(str.c_str());
}

datetime::datetime(const datetime& dt)
    : m_jd(dt.m_jd),
      m_cal(dt.m_cal)
{ }

datetime& datetime::operator =(const datetime& dt)
{
    m_jd = dt.m_jd;
    m_cal = dt.m_cal;
    return *this;
}

datetime::datetime(datetime&& dt) noexcept
    : m_jd(std::move(dt.m_jd)),
      m_cal(std::move(dt.m_cal))
{ }

datetime& datetime::operator =(datetime&& dt) noexcept
{
    m_jd = std::move(dt.m_jd);
    m_cal = std::move(dt.m_cal);
    return *this;
}

void datetime::init(const datepart_t year, const datepart_t month, const datepart_t day,
                    const datepart_t hour, const datepart_t minute, const datepart_t second,
                    const datepart_t millisecond,
                    const calendar_t cal)
{
    m_cal = cal;
    if (m_cal == calendar_t::date_default)
        m_cal = (year > 1582 || (year == 1582 && month > 10) || (year == 1582 && month == 10 && day >= 15))
                ? calendar_t::gregorian : calendar_t::julian;
    m_jd = calendar_to_jd(m_cal, data_t(year, month, day, hour, minute, second, millisecond));
}


datetime::jd_t datetime::calendar_to_jd(const calendar_t cal, const data_t& dt)
{
    datepart_t y = dt.year;
    datepart_t m = dt.month;
    if (m == 1 || m == 2)
    {
        y--;
        m += 12;
    }
    long b = 0;
    if (cal == calendar_t::gregorian)
    {
        short a = div(y, 100).quot;
        b = 2 - a + div(a, 4).quot;
    }
    long c = (long)trunc(365.25 * y);
    if (y < 0)
        c = (long)trunc(365.25 * y - 0.75);
    long d = (long)trunc(30.6001 * (m + 1));
    double hh = hms_to_hh(dt);
    jd_t jd = b + c + d + dt.day + hh + 1720994.5;
    return jd;
}


datetime::data_t datetime::jd_to_calendar(const calendar_t cal, const jd_t jd)
{
    
    double fpart, ipart;
    fpart = modf(jd + 0.5, &ipart);
    double b = ipart;
    if (cal == calendar_t::gregorian) // Also can check (b > 2299160) when consider calendar is Gregorian for all dates since 15 Oct 1582 at 00:00
    {
        double a = trunc((ipart - 1867216.25) / 36524.25);
        b = ipart + 1 + a - trunc(a / 4.0);
    }
    double c = b + 1524;
    double d = trunc((c - 122.1) / 365.25);
    double e = trunc(365.25 * d);
    double g = trunc((c - e) / 30.6001);
    double day = c - e + fpart - trunc(30.6001 * g);
    double hh, dd;
    hh = modf(day, &dd);
    data_t dt = hh_to_hms(hh);
    dt.day = (datepart_t)dd;
    dt.month = (datepart_t)trunc(g < 13.5 ? g - 1 : g - 13);
    dt.year = (datepart_t)trunc(dt.month > 2.5 ? d - 4716 : d - 4715);
    return dt;
}

datetime::data_t datetime::hh_to_hms(const double hh)
{
    data_t dt;
    double ipart;
    double fpart = modf(hh * 24, &ipart);
    dt.hour = (datepart_t)ipart;
    fpart = modf(fpart * 60, &ipart);
    dt.min = (datepart_t)ipart;
    fpart = modf(fpart * 60, &ipart);
    dt.sec = (datepart_t)ipart;
    fpart = modf(round(fpart * 1000), &ipart);
    dt.msec = (datepart_t)ipart;
    if (dt.msec > 999)
    {
        dt.msec -= 1000;
        if (++dt.sec > 59)
        {
            dt.sec -= 60;
            if (++dt.min > 59)
            {
                dt.min -= 60;
                if (++dt.hour > 23)
                {
                    dt.hour -= 24;
                    dt.day = 1;
                }
            }
        }
    }
    return dt;
}

double datetime::hms_to_hh(const data_t time)
{
    double ss = time.sec + (time.msec / 1000.0);
    double hh = ((((ss / 60.0) + time.min) / 60.0) + time.hour) / 24.0;
    return hh;
}


datetime datetime::now()
{
    time_t cur_time;
    time(&cur_time);
    struct tm t;
#if defined(__BCPLUSPLUS__)
    localtime_s(&cur_time, &t);
#else
    localtime_s(&t, &cur_time);
#endif
    return datetime(t);
}

void datetime::parse(const char* str)
{
    datepart_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, millisec = 0;
    int npos = EOF;
	if (strchr(str, '-') != NULL)
	{
        // try to read date and time
        if (strchr(str, 'T') != NULL)
            npos = sscanf_s(str, "%4hd-%2hu-%2huT%2hu:%2hu:%2hu.%3hu", &year, &month, &day, &hour, &minute, &second, &millisec);
        else
			npos = sscanf_s(str, "%4hd-%2hu-%2hu %2hu:%2hu:%2hu.%3hu", &year, &month, &day, &hour, &minute, &second, &millisec);
        if (npos < 3 || npos == EOF || npos == 4)
            throw datetime_exception(datetime_exception::kind::invalid_datetime_string, str);
    }
	else
	{
		// try to read time only
		npos = sscanf_s(str, "%2hu:%2hu:%2hu.%3hu", &hour, &minute, &second, &millisec);
		if (npos < 2 || npos == EOF)
            throw datetime_exception(datetime_exception::kind::invalid_datetime_string, str);
        datetime dt = datetime::now();
        year = dt.year();
        month = dt.month();
        day = dt.day();
    }
    init(year, month, day, hour, minute, second, millisec, calendar_t::date_default);
	if (!is_valid())
        throw datetime_exception(datetime_exception::kind::invalid_datetime_value, str);
}


string datetime::to_str()
{
    char buf[64];
	sprintf_s(buf, sizeof(buf) - 1, "%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu",
            year(), month(), day(), hour(), minute(), second(), millisecond());
	return string(buf);
}

datepart_t datetime::year() const 
{ 
    return jd_to_calendar(m_cal, m_jd).year;
}

datepart_t datetime::month() const 
{
    return jd_to_calendar(m_cal, m_jd).month;
}

datepart_t datetime::quarter() const 
{ 
    return datetime::get_quarter_of_month(month()); 
}

datepart_t datetime::day() const 
{ 
    return jd_to_calendar(m_cal, m_jd).day;
}

datepart_t datetime::hour() const 
{ 
    return jd_to_calendar(m_cal, m_jd).hour;
}

datepart_t datetime::minute() const 
{ 
    return jd_to_calendar(m_cal, m_jd).min;
}

datepart_t datetime::second() const 
{ 
    return jd_to_calendar(m_cal, m_jd).sec;
}

datepart_t datetime::millisecond() const 
{
    return jd_to_calendar(m_cal, m_jd).msec;
}


datepart_t datetime::get_quarter_of_month(datepart_t month)
{
    return div(month - 1, 3).quot + 1;
}


bool datetime::is_valid(jd_t jd)
{
    return jd >= 0;
}

bool datetime::is_valid()
{
    return is_valid(m_jd);
}


datepart_t datetime::day_of_week(const jd_t& jd)
{
    double ipart;
    double fpart = modf(trunc(jd + 1.5) / 7.0, &ipart);
    datepart_t dow = (datepart_t)round(fpart * 7);
    if (dow == 0)
        return 7;
    return dow;
}

datepart_t datetime::day_of_week() const
{
    return datetime::day_of_week(m_jd);
}

datepart_t datetime::day_of_year() const
{
    datepart_t a = month();
    datepart_t b = is_leap_year() ? 62 : 63;
    if (a > 2)
        a = (datepart_t)trunc((a + 1) * 30.6) - b;
    else
        a = div((a - 1) * b, 2).quot;
    return a + day();
}


long datetime::diff(const datetime_unit unit, const datetime& dt_then) const
{
    switch (unit)
    {
    case dtunit_t::years:
    case dtunit_t::months:
    {
        data_t d1 = jd_to_calendar(m_cal, m_jd);
        data_t d2 = jd_to_calendar(dt_then.calendar(), dt_then.jd());
        long d = d2.year - d1.year;
        if (unit == dtunit_t::years)
        {
            if (d1.year < 0 && d2.year > 0)
                d--;
            else if (d1.year > 0 && d2.year < 0)
                d++;
            return d;
        }
        return (d2.year - d1.year) * 12 + (d2.month - d1.month);
    }
    case dtunit_t::days:
        return (long)(trunc(dt_then.jd() + 0.5) - trunc(jd() + 0.5));
    case dtunit_t::hours:
    case dtunit_t::minutes:
    case dtunit_t::seconds:
    {
        double dd;
        double hh = modf(dt_then.jd() - jd(), &dd);
        data_t hms = hh_to_hms(hh);
        long result = (long)(trunc(dd * HOURS_IN_DAY)) + hms.hour;
        if (unit == dtunit_t::hours)
            return result;
        result = result * MINUTES_IN_HOUR + hms.min;
        if (unit == dtunit_t::minutes)
            return result;
        return result * SECONDS_IN_MINUTE + hms.sec;
    }
    default:
        throw datetime_exception(datetime_exception::kind::not_implemented);
    }
}


datetime& datetime::inc(const dtunit_t unit, const int offset)
{
    jd_t jd = 0.0;
    switch (unit)
    {
    case dtunit_t::years:
    case dtunit_t::months:
    {
        data_t dt = jd_to_calendar(m_cal, m_jd);
        if (unit == dtunit_t::years)
            dt.year += offset;
        else // unit == dtunit_t::months
        {
            int month = dt.month + offset;
            if (month > 0)
            {
                if (month > 12)
                {
                    dt.year += div(month, MONTHS_IN_YEAR).quot;
                    dt.month = div(month, MONTHS_IN_YEAR).rem;
                }
                else
                    dt.month = month;
            }
            else if (month == 0)
            {
                dt.year--;
                dt.month = MONTHS_IN_YEAR;
            }
            else if (month < 0)
            {
                dt.year -= div(abs(month), MONTHS_IN_YEAR).quot + 1;
                dt.month = MONTHS_IN_YEAR - div(abs(month), MONTHS_IN_YEAR).rem;
            }
        }
        if (dt.day > days_in_month(dt.month, dt.year))
            dt.day = days_in_month(dt.month, dt.year);
        jd = calendar_to_jd(m_cal, dt);
        break;
    }
    case dtunit_t::days:
        jd = m_jd + offset;
        break;
    case dtunit_t::hours:
    case dtunit_t::minutes:
    case dtunit_t::seconds:
    {
        data_t dt;
        if (unit == dtunit_t::hours)
        {
            div_t days = div(abs(offset), HOURS_IN_DAY);
            jd = m_jd + days.quot;
            dt.hour = days.rem;
        }
        else if (unit == dtunit_t::minutes)
        {
            div_t days = div(abs(offset), MINUTES_IN_DAY);
            jd = m_jd + days.quot;
            dt.hour = div(days.rem, MINUTES_IN_HOUR).quot;
            dt.min = div(days.rem, MINUTES_IN_HOUR).rem;
        }
        else if (unit == dtunit_t::seconds)
        {
            div_t days = div(abs(offset), SECONDS_IN_DAY);
            jd = m_jd + days.quot;
            div_t hours = div(days.rem, SECONDS_IN_HOUR);
            dt.hour = hours.quot;
            dt.min = div(hours.rem, SECONDS_IN_HOUR).quot;
            dt.sec = div(hours.rem, SECONDS_IN_HOUR).rem;
        }
        jd += hms_to_hh(dt) * (offset < 0 ? -1 : 1);
        break;
    }
    default:
        throw datetime_exception(datetime_exception::kind::not_implemented);
    }
    if (jd < 0)
        trunc_to_jd_grain(jd);
    if (!is_valid(jd))
        throw datetime_exception(datetime_exception::kind::invalid_datetime_value);
    m_jd = jd;
    return *this;
}

datetime::jd_t datetime::jd_grain()
{
    // Code to get grain value:
    /*
    data_t dt0; 
    dt0.msec = 1;
    double grain = hms_to_hh(dt0);
    */
    return 1.16E-08; // 1 millisecond converted to JD
}

void datetime::trunc_to_jd_grain(jd_t& jd)
{
    if (abs(jd) < jd_grain())
        jd = 0.0;
}


datepart_t datetime::days_in_month(datepart_t month, datepart_t year)
{
	static datepart_t days[MONTHS_IN_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (month < 0 || month > MONTHS_IN_YEAR)
        throw datetime_exception(datetime_exception::kind::invalid_date_part, (int)dtunit_t::months);
	if (month == 2 && is_leap_year(year))
		return 29;
	return days[month - 1];
}


bool datetime::is_leap_year() const
{
    return datetime::is_leap_year(year());
}

bool datetime::is_leap_year(const datetime& dt)
{
    return datetime::is_leap_year(dt.year());
}

bool datetime::is_leap_year(const datepart_t year)
{
    return ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
}
