/*
 * C++ standard library extensions
 * The datetime class helps to manipulate date and time values
 *
 * (c) 2001-2019 Serguei TARASSOV
 * www.arbinada.com
 * st@arbinada.com
 */

#pragma once

#include <time.h>
#include <string>
#include <exception>

namespace stdext
{

    const short SECONDS_IN_MINUTE   = 60;
    const short MINUTES_IN_HOUR     = 60;
    const short SECONDS_IN_HOUR     = SECONDS_IN_MINUTE * MINUTES_IN_HOUR;
    const short HOURS_IN_DAY	    = 24;
    const int   SECONDS_IN_DAY  	= SECONDS_IN_HOUR * HOURS_IN_DAY;
    const int   MINUTES_IN_DAY 		= MINUTES_IN_HOUR * HOURS_IN_DAY;
    const short MONTHS_IN_YEAR		= 12;


    class datetime_exception : public std::exception
    {
        public:
            enum class kind
            {
                none,
                invalid_datetime_string,
                invalid_datetime_value,
                invalid_date_part,
                invalid_time_unit,
                invalid_date_part_for_time_format
            };

        public:
            datetime_exception(const datetime_exception::kind kind);
            datetime_exception(const datetime_exception::kind kind, const char* value);
            datetime_exception(const datetime_exception::kind kind, const int value);
            const char* what() const throw() override;
        private:
            static std::string msg_by_kind(const datetime_exception::kind kind);
        private:
            std::string              m_message;
            datetime_exception::kind m_kind;
    };


    class datetime
    {
    public:

        typedef __int64 Value;
        typedef __int64 DatePartValue;

        /*
         * Default constructor initializes the object with the current date and time
         */
        datetime();

        /*
         * Initializes the object with the values taken from the formatted string like:
         * "YYYY-MM-DD hh:mm:ss"
         * "YYYY-MM-DD hh:mm"
         * "YYYY-MM-DD"
         * "hh:mm:ss"
         * "hh:mm"
         * where number of the seconds is 0 by default
         */
        datetime(const char* str);
        datetime(const std::string& str);

        /*
         * Copy constructor
         */
        datetime(const datetime& dt);

        /*
         * Assignment operator
         */
        datetime& operator =(const datetime& dt);

        enum Format
        {
            INVALID = 0,
            DATETIME,
            DATE,
            TIME
        };


        enum TimeUnit
        {
            SECONDS,
            MINUTES,
            HOURS,
            DAYS,
            MONTHS,
            YEARS,
        };

        enum DatePart
        {
            NONE = -1,
            SECOND = 0,
            MINUTE,
            HOUR,
            TIME_HM,
            TIME_HMS,
            DATE_TIME_HM,
            DATE_TIME_HMS,
            DATE_ONLY,
            DAY_OF_WEEK,
            DAY_OF_MONTH,
            DAY_OF_YEAR,
            MONTH_OF_YEAR,
            MONTH_AND_DAY,
            QUARTER,
            YEAR,
            YEAR_AND_MONTH,
            YEAR_AND_QUARTER,
        };

        /*
         * Returns the part of date
         *
         * E.g. if the initial date is 01/01/1973 00:00:00
         * GetDatePart(YEAR) returns 1973
         * GetDatePart(MONTH_OF_YEAR) returns 1
         * GetDatePart(DAY_OF_MONTH) returns 1
         */
        DatePartValue GetDatePart(const DatePart part) const;

        int GetYear()  const { return GetDatePart(DatePart::YEAR); }
        int GetMonth() const { return GetDatePart(DatePart::MONTH_OF_YEAR); }
        int GetDay()   const { return GetDatePart(DatePart::DAY_OF_MONTH); }

        /*
         * Returns the number of the required date/time unit since the epoch (currently is 00:00:00 November 17, 1858)
         * If the format is TIME, returns the 0 for all date units.
         *
         * E.g. if the initial date is 01/01/1973 00:00:00
         * getValue( HOURS ) returns the number of hours since the epoch
         * getValue( SECONDS ) returns the number of seconds since the epoch
         */
        Value getValue(TimeUnit unit);

        /*
         * Returns one of the datetime formats, see the Format enumeration
         */
        Format getFormat()
        {
            return m_format;
        }

        /*
         * Returns the date and time in format "YYYY-MM-DD hh:mm:ss"
         */
        std::string to_str();

        /*
         * Increments the date/time value. The offset is expressed in the time units
         * specified in timeUnit.
         */
        void increment(int offset, TimeUnit timeUnit);

        /*
         * Returns the modified Julian date (the number of days since 17/11/1858)
         * of the date specified by the day/month/year
         */
        static Value getMJDFromDate(short day, short month, short year);

        /*
         * Returns the date structure given the number of days since 17/11/1858
         * Note that the tm_year member already contains the right year (not the year minus 1900),
         * and the tm_mon member starts with 1, not 0.
         */
        static struct tm getDateFromMJD(Value days);

        /*
         * Returns the quarter number given the month number
         */
        static short getQuarterFromMonth(short month);

        /*
         * Returns the day of week (1 - Monday, 2 - Tuesday, ... 7 - Sunday)
         * of the date specified by the day/month/year
         */
        static short getDayOfWeek(short day, short month, short year);

        /*
         * Returns the absolute value of the difference between two dates
         * specified by day/month/year in the specified time units
         * Warning: only DAYS, MONTHS and YEARS units accepted
         * otherwise returns 0
         */
        static Value getDateDiff
                (
                        short day1, short month1, short year1,
                        short day2, short month2, short year2,
                        TimeUnit unit
                        );

        /*
         * Returns true if the specified year is a leap year
         */
        static bool isLeapYear(short year)
        {
            return ( (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)) );
        }

        /*
         * Returns the number of days in the specified month/year
         */
        static short getDaysInMonth(short month, short year);

        /*
         * Returns the number of days in the specified month/year
         */
        static short getMonthNumberByName(const char *name);

    private:

        /*
         * Common initializer
         */
        void parse(const char* str);

        /*
         * Validate the stored (in m_tm) date/time value
         * If not valid, sets the format to INVALID
         */
        bool isValid();

        /*
         * Truncate the fractional part of the floating point value
         */
        inline static Value trunc(double dv)
        {
            return (Value) dv;
        }

        /*
         * m_tm structure is not usable for date/time system functions
         * internal use only
         * Note that the tm_year member already contains the right year (not the year minus 1900),
         * and the tm_mon member starts with 1, not 0.
         */
        struct tm m_tm;

        Format m_format;
    };
}

