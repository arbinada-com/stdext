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
        if (n > -1 && n < size)
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
	: m_format(datetime::DATETIME)
{
	time_t cur_time;
    time(&cur_time);
    localtime_s(&m_tm, &cur_time);
	m_tm.tm_year += 1900;
	m_tm.tm_mon++;
}


datetime::datetime(const char* str)
{
	parse(str);
}


datetime::datetime(const std::string& str)
{
	parse(str.c_str());
}

datetime& datetime::operator =(const datetime& dt)
{
    m_tm = dt.m_tm;
    m_format = dt.m_format;
    return *this;
}

datetime::datetime(const datetime& dt)
    : m_tm(dt.m_tm),
      m_format(dt.m_format)
{ }


void datetime::parse(const char* str)
{
	m_format = datetime::INVALID;
	unsigned short year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	char month_name[32];
	int npos;
	if (strchr(str, '-') != NULL)
	{
		// try to read date and time
		npos = sscanf_s(str, "%2hu-%3s-%4hu %2hu:%2hu:%2hu",
                        &day, month_name, &year, &hour, &minute, &second);
		if ((npos < 3) || (npos == EOF) || (npos == 4)
			|| (month = getMonthNumberByName(month_name)) == 0)
        {
			npos = sscanf_s(str, "%4hu-%2hu-%2hu %2hu:%2hu:%2hu",
                            &year, &month, &day, &hour, &minute, &second);
			if ((npos < 3) || (npos == EOF) || (npos == 4))
			{
                throw datetime_exception(datetime_exception::kind::invalid_datetime_string, str);
			}
		}
		if (npos == 3)
			m_format = datetime::DATE;
		else
			m_format = datetime::DATETIME;
	}
	else
	{
		// try to read time only
		npos = sscanf_s(str, "%2hu:%2hu:%2hu", &hour, &minute, &second);
		if ((npos < 2) || (npos == EOF))
		{
            throw datetime_exception(datetime_exception::kind::invalid_datetime_string, str);
		}
		else
		{
			m_format = datetime::TIME;
		}
	}

	m_tm.tm_year = year;
	m_tm.tm_mon = month;
	m_tm.tm_mday = day;
	m_tm.tm_hour = hour;
	m_tm.tm_min = minute;
	m_tm.tm_sec = second;

	if (!isValid())
        throw datetime_exception(datetime_exception::kind::invalid_datetime_value, str);
}


string datetime::to_str()
{
	char buf[64];
	sprintf_s(buf, sizeof(buf) - 1, "%04hu-%02hu-%02hu %02hu:%02hu:%02hu",
            m_tm.tm_year, m_tm.tm_mon, m_tm.tm_mday, m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
	return string(buf);
}


datetime::DatePartValue datetime::GetDatePart(const DatePart part) const
{
	switch (part)
	{
	case SECOND:
		return (DatePartValue) m_tm.tm_sec;
	case MINUTE:
		return (DatePartValue) m_tm.tm_min;
	case HOUR:
		return (DatePartValue) m_tm.tm_hour;
	case TIME_HM:
		return (DatePartValue) m_tm.tm_hour * 100 + m_tm.tm_min;
	case TIME_HMS:
		return (DatePartValue) m_tm.tm_hour * 10000 + m_tm.tm_min * 100 + m_tm.tm_sec;
	case DATE_TIME_HM:
		return (DatePartValue) m_tm.tm_year * 100000000 + m_tm.tm_mon * 1000000 + m_tm.tm_mday * 10000 +
                m_tm.tm_hour * 100 + m_tm.tm_min;
	case DATE_TIME_HMS:
		return (DatePartValue) m_tm.tm_year * (__int64)100000000 * 100 + m_tm.tm_mon * 100000000 + m_tm.tm_mday * 1000000 +
                m_tm.tm_hour * 10000 + m_tm.tm_min * 100 + m_tm.tm_sec;
	case DATE_ONLY:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) m_tm.tm_year * 10000 + m_tm.tm_mon * 100 + m_tm.tm_mday;
	case DAY_OF_WEEK:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) getDayOfWeek(m_tm.tm_mday, m_tm.tm_mon, m_tm.tm_year);
	case DAY_OF_MONTH:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) m_tm.tm_mday;
	case DAY_OF_YEAR:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) getDateDiff(m_tm.tm_mday, m_tm.tm_mon, m_tm.tm_year, 1, 1, m_tm.tm_year, DAYS) + 1;
	case MONTH_OF_YEAR:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) m_tm.tm_mon;
	case MONTH_AND_DAY:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) m_tm.tm_mon * 100 + m_tm.tm_mday;
	case QUARTER:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) getQuarterFromMonth(m_tm.tm_mon);
	case YEAR:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) m_tm.tm_year;
	case YEAR_AND_MONTH:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) m_tm.tm_year * 100 + m_tm.tm_mon;
	case YEAR_AND_QUARTER:
		if (m_format == TIME)
            throw datetime_exception(datetime_exception::kind::invalid_date_part_for_time_format, part);
		return (DatePartValue) m_tm.tm_year * 100 + getQuarterFromMonth(m_tm.tm_mon);
	default:
        throw datetime_exception(datetime_exception::kind::invalid_date_part, part);
	}
}


short datetime::getQuarterFromMonth(short month)
{
    return trunc((month - 1) / 3) + 1;
}


datetime::Value datetime::getValue(TimeUnit unit)
{
	Value days = (m_format != TIME) ? getMJDFromDate(m_tm.tm_mday, m_tm.tm_mon, m_tm.tm_year) : 0;
	switch (unit)
	{
	case SECONDS:
		return (days * 3600 * 24) + (m_tm.tm_hour * 3600) + (m_tm.tm_min * 60) + m_tm.tm_sec;
	case MINUTES:
		return (days * 60 * 24) + (m_tm.tm_hour * 60) + m_tm.tm_min;
	case HOURS:
		return (days * 24) + m_tm.tm_hour;
	case DAYS:
		return days;
	case MONTHS:
		{
			if (m_format == TIME)
				return 0;
			// is the same as the difference in months between this date and 17/11/1858
			return getDateDiff(m_tm.tm_mday, m_tm.tm_mon, m_tm.tm_year,
                               17, 11, 1858, MONTHS);
		}
	case YEARS:
		{
			if (m_format == TIME)
				return 0;
			// is the same as the difference in years between this date and 17/11/1858
			return getDateDiff(m_tm.tm_mday, m_tm.tm_mon, m_tm.tm_year,
                               17, 11, 1858, YEARS);
		}
	default:
        throw datetime_exception(datetime_exception::kind::invalid_time_unit, unit);
	}
}


void datetime::increment(int offset, TimeUnit unit)
{
	Value seconds = getValue(SECONDS);
	switch (unit)
	{
	case SECONDS:
		seconds += offset;
		break;
	case MINUTES:
		seconds += offset * SECONDS_IN_MINUTE;
		break;
	case HOURS:
		seconds += offset * SECONDS_IN_HOUR;
		break;
	case DAYS:
		if (m_format != TIME)
		{
			seconds += offset * SECONDS_IN_DAY;
		}
		break;
	case MONTHS:
		if (m_format != TIME)
		{
			int months = m_tm.tm_mon + offset;
			if (months > 1)
			{
				m_tm.tm_year += months / MONTHS_IN_YEAR;
				m_tm.tm_mon = months % MONTHS_IN_YEAR;
			}
			else if (months == 0)
			{
				m_tm.tm_year--;
				m_tm.tm_mon = MONTHS_IN_YEAR;
			}
			else  // months < 0
			{
				m_tm.tm_year -= abs(months) / MONTHS_IN_YEAR + 1;
				m_tm.tm_mon = (MONTHS_IN_YEAR + months) % MONTHS_IN_YEAR;
			}

			if (m_tm.tm_mday > getDaysInMonth(m_tm.tm_mon, m_tm.tm_year))
				m_tm.tm_mday = getDaysInMonth(m_tm.tm_mon, m_tm.tm_year);

			isValid();
		}
		return;
	case YEARS:
		if (m_format != TIME)
		{
			m_tm.tm_year += offset;
			isValid();
		}
		return;
	default:
        throw datetime_exception(datetime_exception::kind::invalid_time_unit, unit);
	}
	if (m_format != TIME)
	{
		Value days = seconds / SECONDS_IN_DAY;
		m_tm = getDateFromMJD(days);
	}
	if (seconds < 0)
	{
		seconds = ((-seconds / SECONDS_IN_DAY + 1) * SECONDS_IN_DAY + seconds) % SECONDS_IN_DAY;
	}
	m_tm.tm_sec = seconds % SECONDS_IN_MINUTE;
	m_tm.tm_min = seconds / SECONDS_IN_MINUTE % MINUTES_IN_HOUR;
	m_tm.tm_hour = seconds / SECONDS_IN_HOUR % HOURS_IN_DAY;
	isValid();
}


bool datetime::isValid()
{
	if (m_format ==INVALID)
		return false;
    if (
            (
                ((m_format == datetime::DATETIME) || (m_format == datetime::DATE)) &&
                (
                    (m_tm.tm_year < 1858) || (m_tm.tm_mon < 1)  ||
                    (m_tm.tm_mon > MONTHS_IN_YEAR) || (m_tm.tm_mday < 1) ||
                    (m_tm.tm_mday > getDaysInMonth(m_tm.tm_mon, m_tm.tm_year))
                )
            )
            ||
            (
                    (m_tm.tm_hour < 0) || (m_tm.tm_hour > 23) ||
                    (m_tm.tm_min < 0)  || (m_tm.tm_min > 59) ||
                    (m_tm.tm_sec < 0)  || (m_tm.tm_sec > 59)
            )
        )
    {
		m_format = INVALID;
		return false;
	}
	return true;
}


datetime::Value datetime::getMJDFromDate(short day, short month, short year)
{
	Value b, a = 10000 * year + 100 * month + day;
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


struct tm datetime::getDateFromMJD(Value days)
{
	struct tm tm;
	Value a, b, c, d, e, f;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;

	a = days + 2400001;
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

    d = trunc((c - 122.1) / 365.25);
    e = 365 * d + trunc(d / 4);
    f = trunc((c - e) / 30.6001);

	tm.tm_mday = trunc(c - e + 0.5) - trunc(f * 30.6001);
	tm.tm_mon = f - 1 - 12 * trunc(f / 14);
    tm.tm_year = d - 4715 - trunc((7 + tm.tm_mon) / 10);

	return tm;
}


short datetime::getDayOfWeek(short day, short month, short year)
{
	if (month > 2)
	{
		month++;
	}
	else
	{
		month += 13;
		year--;
	}
	Value n = trunc(365.25 * year) + trunc(30.6 * month) + day - 621050;
	return (n - trunc(n / 7) * 7 + 1);
}


datetime::Value
        datetime::getDateDiff
        (
                short day1, short month1, short year1,
                short day2, short month2, short year2,
                TimeUnit unit
               )
{
	switch (unit)
	{
	case DAYS:
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
	case MONTHS:
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
	case YEARS:
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


short datetime::getDaysInMonth(short month, short year)
{
	static short days[MONTHS_IN_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if ((month < 0) || (month > MONTHS_IN_YEAR))
		return 0;
	if ((month == 2) && isLeapYear(year))
		return 29;
	return days[month - 1];
}


short datetime::getMonthNumberByName(const char *name)
{
	static const char* monthNames[MONTHS_IN_YEAR] =
    { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };

	for (int i = 0; i < MONTHS_IN_YEAR; i++)
	{
		if (_stricmp(name, monthNames[i]) == 0)
		{
			return i + 1;
		}
	}
	return 0;
}

