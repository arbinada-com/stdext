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
        typedef int datepart_t;
        struct datetime_data
        {
            datepart_t sec;    // seconds after the minute - [0, 60] including leap second
            datepart_t min;    // minutes after the hour - [0, 59]
            datepart_t hour;   // hours since midnight - [0, 23]
            datepart_t day;    // day of the month - [1, 31]
            datepart_t month;  // months since January - [0, 11]
            datepart_t year;   // year [0, 9999)
        };
    public:
        /*
         * Default constructor initializes the object with the current date and time
         */
        datetime();

        datetime(const struct tm &t);

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

        enum class Format
        {
            INVALID = 0,
            DATETIME,
            DATE,
            TIME
        };


        enum class TimeUnit
        {
            SECONDS,
            MINUTES,
            HOURS,
            DAYS,
            MONTHS,
            YEARS,
        };

        enum class datepart
        {
            NONE = -1,
            SECOND = 0,
            MINUTE,
            HOUR,
            TIME_HM,
            TIME_HMS,
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
         * E.g. if the initial date is "10 Jan 1973"
         * get_datepart(YEAR) returns 1973
         * get_datepart(MONTH_OF_YEAR) returns 1
         * get_datepart(DAY_OF_MONTH) returns 10
         */
        datepart_t get_datepart(const datepart part) const;

        datepart_t year()        const { return get_datepart(datepart::YEAR); }
        datepart_t month()       const { return get_datepart(datepart::MONTH_OF_YEAR); }
        datepart_t quarter()     const { return datetime::get_quarter_of_month(month()); }
        datepart_t day()         const { return get_datepart(datepart::DAY_OF_MONTH); }
        datepart_t day_of_week() const { return datetime::day_of_week(*this); };
        datepart_t hour()        const { return get_datepart(datepart::HOUR); }
        datepart_t minute()      const { return get_datepart(datepart::MINUTE); }
        datepart_t second()      const { return get_datepart(datepart::SECOND); }

        /*
         * Returns the number of the required date/time unit since the epoch (currently is 00:00:00 November 17, 1858)
         * If the format is TIME, returns the 0 for all date units.
         *
         * E.g. if the initial date is 01/01/1973 00:00:00
         * get_value(HOURS) returns the number of hours since the epoch
         * get_value(SECONDS) returns the number of seconds since the epoch
         */
        datepart_t get_value(TimeUnit unit);

        /*
         * Returns internal format, see the enumeration
         */
        Format format() { return m_format; }

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
        static datepart_t getMJDFromDate(datepart_t day, datepart_t month, datepart_t year);

        /*
         * Returns the date structure given the number of days since 17/11/1858
         */
        static datetime getDateFromMJD(datepart_t days);

        /*
         * Returns the quarter number given the month number
         */
        static datepart_t get_quarter_of_month(datepart_t month);

        /*
         * Returns the day of week (1 - Monday, 2 - Tuesday, ... 7 - Sunday)
         * of the date specified by the day/month/year
         */
        static datepart_t day_of_week(const datetime& dt);

        /*
         * Returns the absolute value of the difference between two dates
         * specified by day/month/year in the specified time units
         * Warning: only DAYS, MONTHS and YEARS units accepted
         * otherwise returns 0
         */
        static datepart_t diff(
            datepart_t day1, datepart_t month1, datepart_t year1,
            datepart_t day2, datepart_t month2, datepart_t year2,
            TimeUnit unit);

        /*
         * Returns true if the specified year is a leap year
         */
        bool is_leap_year();
        static bool is_leap_year(const datetime& dt);
        static bool is_leap_year(const datepart_t year);

        /*
         * Returns the number of days in the specified month/year
         */
        static datepart_t getDaysInMonth(datepart_t month, datepart_t year);

        /*
         * Returns the number of days in the specified month/year
         */
        static datepart_t getMonthNumberByName(const char* name);

    private:
        void init(const struct tm &t);
        void init(const datetime& dt);

        /*
         * Common initializer
         */
        void parse(const char* str);

        /*
         * Validate the stored (in m_data) date/time value
         * If not valid, sets the format to INVALID
         */
        bool isValid();

        /*
         * Truncate the fractional part of the floating point value
         */
        inline static datepart_t trunc(double dv) { return (datepart_t)dv; }
    private:
        struct datetime_data m_data;
        Format m_format;
    };
}

