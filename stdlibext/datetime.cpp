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

using namespace std;
using namespace stdext;

string str_format(const std::string fmt, ...)
{
    size_t size = fmt.size() * 2;
    string str;
    va_list ap;
    while (true)
    {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf_s((char*)str.data(), size, size - 1, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && (size_t)n < size)
        {
            str.resize(n);
            return str;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
    return str;
}

/**
 * datetime_exception class
 */
datetime_exception::datetime_exception(const datetime_exception::kind kind)
    : datetime_exception(kind, "")
{ }

datetime_exception::datetime_exception(const datetime_exception::kind kind, const char* value)
    : m_kind(kind)
{
    m_message = str_format(msg_by_kind(kind), value);
}

datetime_exception::datetime_exception(const datetime_exception::kind kind, const int value)
    : m_kind(kind)
{

}

const char* datetime_exception::what() const throw()
{
    return m_message.c_str();
}

std::string datetime_exception::msg_by_kind(const datetime_exception::kind kind)
{
    switch(kind)
    {
        case datetime_exception::kind::none:
            return "No error";
        case datetime_exception::kind::invalid_datetime_string:
            return "Invalid date/time initial string \"%s\"";
        case datetime_exception::kind::invalid_datetime_value:
            return "Invalid date/time unit value \"%s\"";
        case datetime_exception::kind::invalid_date_part:
            return "Invalid date part. Code = %d";
        case datetime_exception::kind::invalid_time_unit:
            return "Invalid time unit. Code = %d";
        case datetime_exception::kind::invalid_date_part_for_time_format:
            return "Invalid date part when object has the TIME format. Code = %d";
        default:
            return "Unsupported type";
    }
}



/**
 * datetime class
 */
datetime::datetime() 
	: m_format(Format::DATETIME)
{
	time_t cur_time;
    time(&cur_time);
    struct tm t;
    localtime_s(&t, &cur_time);
    init(t);
}

datetime::datetime(const struct tm &t)
    : m_format(Format::DATETIME)
{
    init(t);
}

datetime::datetime(const char* str)
{
	parse(str);
}


datetime::datetime(const std::string& str)
{
	parse(str.c_str());
}

datetime::datetime(const datetime& dt)
{ 
    init(dt);
}

datetime& datetime::operator =(const datetime& dt)
{
    init(dt);
    return *this;
}

void datetime::init(const struct tm &t)
{
    m_data.sec = t.tm_sec;
    m_data.min = t.tm_min;
    m_data.hour = t.tm_hour;
    m_data.day = t.tm_mday;
    m_data.month = t.tm_mon + 1;
    m_data.year = t.tm_year + 1900;
}

void datetime::init(const datetime& dt)
{
    m_data = dt.m_data;
    m_format = dt.m_format;
}


void datetime::parse(const char* str)
{
	m_format = Format::INVALID;
    datepart_t year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	char month_name[32];
	if (strchr(str, '-') != NULL)
	{
		// try to read date and time
		int npos = sscanf_s(str, "%2d-%3s-%4d %2d:%2d:%2d",
                        &day, month_name, &year, &hour, &minute, &second);
		if ((npos < 3) || (npos == EOF) || (npos == 4)
			|| (month = getMonthNumberByName(month_name)) == 0)
        {
			npos = sscanf_s(str, "%4d-%2d-%2d %2d:%2d:%2d",
                            &year, &month, &day, &hour, &minute, &second);
			if ((npos < 3) || (npos == EOF) || (npos == 4))
			{
                throw datetime_exception(datetime_exception::kind::invalid_datetime_string, str);
			}
		}
		if (npos == 3)
			m_format = Format::DATE;
		else
			m_format = Format::DATETIME;
	}
	else
	{
		// try to read time only
		int npos = sscanf_s(str, "%2d:%2d:%2d", &hour, &minute, &second);
		if ((npos < 2) || (npos == EOF))
		{
            throw datetime_exception(datetime_exception::kind::invalid_datetime_string, str);
		}
		else
		{
			m_format = Format::TIME;
		}
	}

	m_data.year = year;
	m_data.month = month;
	m_data.day = day;
	m_data.hour = hour;
	m_data.min = minute;
	m_data.sec = second;

	if (!isValid())
        throw datetime_exception(datetime_exception::kind::invalid_datetime_value, str);
}


string datetime::to_str()
{
	char buf[64];
	sprintf_s(buf, sizeof(buf) - 1, "%04hu-%02hu-%02hu %02hu:%02hu:%02hu",
            m_data.year, m_data.month, m_data.day, m_data.hour, m_data.min, m_data.sec);
	return string(buf);
}


datetime::datepart_t datetime::get_datepart(const datepart part) const
{
	switch (part)
	{
    case datepart::SECOND:
		return m_data.sec;
	case datepart::MINUTE:
		return m_data.min;
	case datepart::HOUR:
		return m_data.hour;
	case datepart::TIME_HM:
		return m_data.hour * 100 + m_data.min;
	case datepart::TIME_HMS:
		return m_data.hour * 10000 + m_data.min * 100 + m_data.sec;
	case datepart::DATE_ONLY:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return m_data.year * 10000 + m_data.month * 100 + m_data.day;
	case datepart::DAY_OF_WEEK:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return day_of_week(*this);
	case datepart::DAY_OF_MONTH:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return m_data.day;
	case datepart::DAY_OF_YEAR:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return diff(m_data.day, m_data.month, m_data.year, 1, 1, m_data.year, TimeUnit::DAYS) + 1;
	case datepart::MONTH_OF_YEAR:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return m_data.month;
	case datepart::MONTH_AND_DAY:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return m_data.month * 100 + m_data.day;
	case datepart::QUARTER:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return get_quarter_of_month(m_data.month);
	case datepart::YEAR:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return m_data.year;
	case datepart::YEAR_AND_MONTH:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return m_data.year * 100 + m_data.month;
	case datepart::YEAR_AND_QUARTER:
		if (m_format == Format::TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, (int)part);
		return m_data.year * 100 + get_quarter_of_month(m_data.month);
	default:
        throw datetime_exception(datetime_exception::kind::invalid_date_part, (int)part);
	}
}


datetime::datepart_t datetime::get_quarter_of_month(datepart_t month)
{
    return div(month - 1, 3).quot + 1;
}


datetime::datepart_t datetime::get_value(TimeUnit unit)
{
    datepart_t days = (m_format != Format::TIME) ? getMJDFromDate(m_data.day, m_data.month, m_data.year) : 0;
	switch (unit)
	{
	case TimeUnit::SECONDS:
		return (days * 3600 * 24) + (m_data.hour * 3600) + (m_data.min * 60) + m_data.sec;
	case TimeUnit::MINUTES:
		return (days * 60 * 24) + (m_data.hour * 60) + m_data.min;
	case TimeUnit::HOURS:
		return (days * 24) + m_data.hour;
	case TimeUnit::DAYS:
		return days;
	case TimeUnit::MONTHS:
	{
		if (m_format == Format::TIME)
			return 0;
		// is the same as the difference in months between this date and 17/11/1858
		return diff(m_data.day, m_data.month, m_data.year,
                            17, 11, 1858, TimeUnit::MONTHS);
	}
	case TimeUnit::YEARS:
	{
		if (m_format == Format::TIME)
			return 0;
		// is the same as the difference in years between this date and 17/11/1858
		return diff(m_data.day, m_data.month, m_data.year,
                            17, 11, 1858, TimeUnit::YEARS);
	}
	default:
        throw datetime_exception(datetime_exception::kind::invalid_time_unit, (int)unit);
	}
}


void datetime::increment(int offset, TimeUnit unit)
{
    datepart_t seconds = get_value(TimeUnit::SECONDS);
	switch (unit)
	{
	case TimeUnit::SECONDS:
		seconds += offset;
		break;
	case TimeUnit::MINUTES:
		seconds += offset * SECONDS_IN_MINUTE;
		break;
	case TimeUnit::HOURS:
		seconds += offset * SECONDS_IN_HOUR;
		break;
	case TimeUnit::DAYS:
		if (m_format != Format::TIME)
		{
			seconds += offset * SECONDS_IN_DAY;
		}
		break;
	case TimeUnit::MONTHS:
		if (m_format != Format::TIME)
		{
			int months = m_data.month + offset;
			if (months > 1)
			{
				m_data.year += months / MONTHS_IN_YEAR;
				m_data.month = months % MONTHS_IN_YEAR;
			}
			else if (months == 0)
			{
				m_data.year--;
				m_data.month = MONTHS_IN_YEAR;
			}
			else  // months < 0
			{
				m_data.year -= abs(months) / MONTHS_IN_YEAR + 1;
				m_data.month = (MONTHS_IN_YEAR + months) % MONTHS_IN_YEAR;
			}

			if (m_data.day > getDaysInMonth(m_data.month, m_data.year))
				m_data.day = getDaysInMonth(m_data.month, m_data.year);

			isValid();
		}
		return;
	case TimeUnit::YEARS:
		if (m_format != Format::TIME)
		{
			m_data.year += offset;
			isValid();
		}
		return;
	default:
        throw datetime_exception(datetime_exception::kind::invalid_time_unit, (int)unit);
	}
	if (m_format != Format::TIME)
	{
        datepart_t days = seconds / SECONDS_IN_DAY;
		m_data = getDateFromMJD(days).m_data;
	}
	if (seconds < 0)
	{
		seconds = ((-seconds / SECONDS_IN_DAY + 1) * SECONDS_IN_DAY + seconds) % SECONDS_IN_DAY;
	}
	m_data.sec = seconds % SECONDS_IN_MINUTE;
	m_data.min = seconds / SECONDS_IN_MINUTE % MINUTES_IN_HOUR;
	m_data.hour = seconds / SECONDS_IN_HOUR % HOURS_IN_DAY;
	isValid();
}


bool datetime::isValid()
{
	if (m_format == Format::INVALID)
		return false;
    if (
            (
                ((m_format == Format::DATETIME) || (m_format == Format::DATE)) &&
                (
                    (m_data.year < 1858) || (m_data.month < 1)  ||
                    (m_data.month > MONTHS_IN_YEAR) || (m_data.day < 1) ||
                    (m_data.day > getDaysInMonth(m_data.month, m_data.year))
                )
            )
            ||
            (
                    (m_data.hour < 0) || (m_data.hour > 23) ||
                    (m_data.min < 0)  || (m_data.min > 59) ||
                    (m_data.sec < 0)  || (m_data.sec > 59)
            )
        )
    {
		m_format = Format::INVALID;
		return false;
	}
	return true;
}


datetime::datepart_t datetime::getMJDFromDate(datepart_t day, datepart_t month, datepart_t year)
{
    datepart_t b, a = 10000 * year + 100 * month + day;
	if (month <= 2)
	{
		month += MONTHS_IN_YEAR;
		year -= 1;
	}
	if (a < 15821004)
	{
        b = -2 + trunc((year + 4716) / 4) - 1179;
	}
	else
	{
		b = trunc(year / 400) - trunc(year / 100) + trunc(year / 4);
	}
	a = 365 * year - 679004;
	return a + b + trunc(30.6001 * (month + 1)) + day;
}


datetime datetime::getDateFromMJD(datepart_t days)
{
	struct tm tm;
    datepart_t a = days + 2400001;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
    datepart_t b, c;
    if (a < 2299161)
	{
		b = 0;
		c = a + 1524;
	}
	else
	{
        b = trunc((a - 1867216.25) / 36524.25);
        c = a + b - trunc(b / 4) + 1525;
	}
    datepart_t d = trunc((c - 122.1) / 365.25);
    datepart_t e = 365 * d + trunc(d / 4);
    datepart_t f = trunc((c - e) / 30.6001);
	tm.tm_mday = trunc(c - e + 0.5) - trunc(f * 30.6001);
	tm.tm_mon = f - 1 - 12 * trunc(f / 14);
    tm.tm_year = d - 4715 - trunc((7 + tm.tm_mon) / 10);
	return datetime(tm);
}


datetime::datepart_t datetime::day_of_week(const datetime& dt)
{
    datepart_t month = dt.month(), year = dt.year();
	if (month > 2)
		month++;
	else
	{
		month += 13;
		year--;
	}
    datepart_t n = trunc(365.25 * year) + trunc(30.6 * month) + dt.day() - 621050;
	return (n - trunc(n / 7) * 7 + 1);
}


datetime::datepart_t datetime::diff(
    datepart_t day1, datepart_t month1, datepart_t year1,
    datepart_t day2, datepart_t month2, datepart_t year2,
    TimeUnit unit)
{
    switch (unit)
    {
    case TimeUnit::DAYS:
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
        int diff = (trunc(365.25 * year2) + trunc(30.6 * month2) + day2) -
            (trunc(365.25 * year1) + trunc(30.6 * month1) + day1);
        return abs(diff);
    }
    case TimeUnit::MONTHS:
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
    case TimeUnit::YEARS:
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


datetime::datepart_t datetime::getDaysInMonth(datepart_t month, datepart_t year)
{
	static datepart_t days[MONTHS_IN_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if ((month < 0) || (month > MONTHS_IN_YEAR))
		return 0;
	if ((month == 2) && is_leap_year(year))
		return 29;
	return days[month - 1];
}


datetime::datepart_t datetime::getMonthNumberByName(const char* name)
{
    static const char* names[MONTHS_IN_YEAR] =
    { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };

    for (int i = 0; i < MONTHS_IN_YEAR; i++)
    {
        if (_stricmp(name, names[i]) == 0)
        {
            return i + 1;
        }
    }
    return 0;
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
