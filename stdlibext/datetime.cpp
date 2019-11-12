/**
 * Log: datetime.cpp
 *
 * Revision 1.0.2 / 2001-01-05
 * - initial revision generelized from production code
 * Revision 2.0.0 / 2019-10-31
 * - refactoring to use C++11 features and safe functions
 * - changed naming convention (STL compliance)
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
    : m_jd(0.0)
{ }

datetime::datetime(const datepart_t year, const datepart_t month, const datepart_t day,
                   const datepart_t hour, const datepart_t minute, const datepart_t second, 
                   const datepart_t millisecond)
{
    m_jd = gregorian_to_jd(data_t(year, month, day, hour, minute, second, millisecond));
    if (!is_valid())
    {
        string s = strutils::format("Year: %d, month: %d, day: %d, hour: %d, minute: %d, second: %d",
            year, month, day, hour, minute, second);
        throw datetime_exception(datetime_exception::kind::invalid_datetime_string, s.c_str());
    }
}

datetime::datetime(const struct tm &t)
{
    //m_data.sec = t.tm_sec;
    //m_data.min = t.tm_min;
    //m_data.hour = t.tm_hour;
    //m_data.day = t.tm_mday;
    //m_data.month = t.tm_mon + 1;
    //m_data.year = t.tm_year + 1900;
}

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
    : m_jd(dt.m_jd)
{ }

datetime& datetime::operator =(const datetime& dt)
{
    m_jd = dt.m_jd;
    return *this;
}

datetime::datetime(datetime&& dt) noexcept
    : m_jd(std::move(dt.m_jd))
{ }

datetime& datetime::operator =(datetime&& dt) noexcept
{
    m_jd = std::move(dt.m_jd);
    return *this;
}



datetime::jdn_t datetime::gregorian_to_jdn(const datepart_t year, const datepart_t month, const datepart_t day)
{
    return jd_to_jdn(
        (1461 * (year + 4800 + (month - 14) / 12.0)) / 4.0 + 
        (367 * (month - 2 - 12 * ((month - 14) / 12.0))) / 12.0 - 
        (3 * ((year + 4900 + (month - 14) / 12) / 100.0)) / 4.0 + 
        day - 32075
    );
}

datetime::jdn_t datetime::julian_to_jdn(const datepart_t year, const datepart_t month, const datepart_t day)
{
    return 
        367 * year - 
        div(7 * (year + 5001 + div(month - 9, 7).quot), 4).quot +
        div(275 * month, 9).quot + day + 1729777;
}

datetime::jdn_t datetime::jd_to_jdn(const jd_t jd)
{
    return (jdn_t)trunc(jd);
}

datetime::jd_t datetime::jdn_to_jd(const jdn_t jdn,
                                   const datepart_t hour, const datepart_t minute, const datepart_t second, 
                                   const datepart_t millisecond)
{
    data_t dt(0, 0, 0, hour, minute, second, millisecond);
    return jdn + hms_to_hh(dt);
}

datetime::jd_t datetime::gregorian_to_jd(const data_t& dt)
{
    return jdn_to_jd(gregorian_to_jdn(dt.year, dt.month, dt.day), dt.hour, dt.min, dt.sec, dt.msec);
}

datetime::jd_t datetime::julian_to_jd(const data_t& dt)
{
    return jdn_to_jd(julian_to_jdn(dt.year, dt.month, dt.day), dt.hour, dt.min, dt.sec, dt.msec);
}

datetime::data_t datetime::jd_to_calendar(const datetime::calendar cal, const jd_t jd)
{
    long jdn = jd_to_jdn(jd);
    long f = jdn + 1401;
    if (cal == calendar::gregorian)
        f += div(div(4 * jdn + 274277, (long)146097).quot * 3, (long)4).quot - 38;
    const long m = 2;
    const long n = 12;
    const long r = 4;
    const long v = 3;
    const long p = 1461;
    const long s = 153;
    const long u = 5;
    const long w = 2;
    long e = r * f + v;
    long g = div(div(e, p).rem, r).quot;
    long h = u * g + w;
    data_t dt;
    dt.day = (datepart_t)(div(div(h, s).rem, u).quot + 1);
    dt.month = (datepart_t)(div(div(h, s).quot + m, n).rem + 1);
    dt.year = (datepart_t)(div(e, p).quot - 4716 + div(n + m - dt.month, n).quot);
    return dt;
}

datetime::data_t datetime::hh_to_hms(const double hh)
{
    data_t dt;
    double ipart;
    double fpart = modf(hh * 24, &ipart);
    dt.hour = (datepart_t)ipart + 12;
    if (dt.hour > 23)
        dt.hour -= 24;
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
    double hh = ((((ss / 60.0) + time.min) / 60.0) + (time.hour - 12)) / 24.0;
    if (hh < 0)
        return hh + 1;
    return hh;
    //return time.hour / 24.0 + time.min / 1440.0 + ss / 86400.0;
    //return ((((ss / 60.0) + time.min) / 60.0) + time.hour) / 100.0;
}


datetime datetime::now()
{
    time_t cur_time;
    time(&cur_time);
    struct tm t;
    localtime_s(&t, &cur_time);
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
    m_jd = gregorian_to_jd(data_t(year, month, day, hour, minute, second, millisec));
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


datetime::datepart_t datetime::get_datepart(const datepart part) const
{
    data_t dt = jd_to_calendar(calendar::gregorian, m_jd);
    switch (part)
	{
    case datepart::millisecond:
        return dt.msec;
    case datepart::second:
		return dt.sec;
	case datepart::minute:
		return dt.min;
	case datepart::hour:
		return dt.hour;
	case datepart::day_of_week:
		return day_of_week();
	case datepart::day_of_month:
		return dt.day;
	case datepart::day_of_year:
        return 0; // diff(m_data.day, m_data.month, m_data.year, 1, 1, m_data.year, datetime_unit::days) + 1;
	case datepart::month:
		return dt.month;
	case datepart::quarter:
		return get_quarter_of_month(dt.month);
	case datepart::year:
		return dt.year;
	default:
        throw datetime_exception(datetime_exception::kind::invalid_date_part, (int)part);
	}
}


datetime::datepart_t datetime::get_quarter_of_month(datepart_t month)
{
    return div(month - 1, 3).quot + 1;
}


void datetime::increment(int offset, datetime_unit unit)
{
    throw datetime_exception(datetime_exception::kind::not_implemented);
    /*datepart_t seconds = get_value(datetime_unit::seconds);
	switch (unit)
	{
	case datetime_unit::seconds:
		seconds += offset;
		break;
	case datetime_unit::minutes:
		seconds += offset * SECONDS_IN_MINUTE;
		break;
	case datetime_unit::hours:
		seconds += offset * SECONDS_IN_HOUR;
		break;
	case datetime_unit::days:
		seconds += offset * SECONDS_IN_DAY;
		break;
	case datetime_unit::months:
    {
        int month = m_data.month + offset;
        if (month > 1)
        {
            m_data.year += month / MONTHS_IN_YEAR;
            m_data.month = month % MONTHS_IN_YEAR;
        }
        else if (month == 0)
        {
            m_data.year--;
            m_data.month = MONTHS_IN_YEAR;
        }
        else  // month < 0
        {
            m_data.year -= abs(month) / MONTHS_IN_YEAR + 1;
            m_data.month = (MONTHS_IN_YEAR + month) % MONTHS_IN_YEAR;
        }
        if (m_data.day > days_in_month(m_data.month, m_data.year))
            m_data.day = days_in_month(m_data.month, m_data.year);
        is_valid();
        return;
    }
	case datetime_unit::years:
		m_data.year += offset;
		is_valid();
		return;
	default:
        throw datetime_exception(datetime_exception::kind::invalid_time_unit, (int)unit);
	}
    datepart_t days = seconds / SECONDS_IN_DAY;
	m_data = getDateFromMJD(days).m_data;
	if (seconds < 0)
	{
		seconds = ((-seconds / SECONDS_IN_DAY + 1) * SECONDS_IN_DAY + seconds) % SECONDS_IN_DAY;
	}
	m_data.sec = seconds % SECONDS_IN_MINUTE;
	m_data.min = seconds / SECONDS_IN_MINUTE % MINUTES_IN_HOUR;
	m_data.hour = seconds / SECONDS_IN_HOUR % HOURS_IN_DAY;
	is_valid();
    */
}


bool datetime::is_valid()
{
    return m_jd >= 0;
}


datetime::datepart_t datetime::day_of_week(const jd_t& jd)
{
    return (datepart_t)(div(datetime::jd_to_jdn(jd), (long)7).rem + 1);
}

datetime::datepart_t datetime::day_of_week() const
{
    return datetime::day_of_week(m_jd);
}


long datetime::diff(
    datepart_t day1, datepart_t month1, datepart_t year1,
    datepart_t day2, datepart_t month2, datepart_t year2,
    datetime_unit unit)
{
    switch (unit)
    {
    case datetime_unit::days:
    {
        if (month1 > 2)
            month1++;
        else
        {
            month1 += 13;
            year1--;
        }
        if (month2 > 2)
            month2++;
        else
        {
            month2 += 13;
            year2--;
        }
        int diff = (long)((trunc(365.25 * year2) + trunc(30.6 * month2) + day2) -
            (trunc(365.25 * year1) + trunc(30.6 * month1) + day1));
        return abs(diff);
    }
    case datetime_unit::months:
    {
        short cf = 0;
        if (day1 > day2)
            cf = 1;
        short m = month1 - month2 - cf;
        cf = 0;
        if (m < 0)
        {
            m += 12;
            cf = 1;
        }
        short y = year1 - year2 - cf;
        return abs(y * 12) + m;
    }
    case datetime_unit::years:
    {
        short cf = 0;
        if (day1 > day2)
            cf = 1;
        if ((month1 - month2 - cf) < 0)
            cf = 1;
        else
            cf = 0;
        short y = year1 - year2 - cf;
        return abs(y);
    }
    default:
        return 0;
    }
}


datetime::datepart_t datetime::days_in_month(datepart_t month, datepart_t year)
{
	static datepart_t days[MONTHS_IN_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if (month < 0 || month > MONTHS_IN_YEAR)
        throw datetime_exception(datetime_exception::kind::invalid_date_part, (int)datepart::month);
	if (month == 2 && is_leap_year(year))
		return 29;
	return days[month - 1];
}


bool datetime::is_leap_year()
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
