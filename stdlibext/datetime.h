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
        static const datepart_t epoch_year  = 1858;
        static const datepart_t epoch_month = 11;
        static const datepart_t epoch_day   = 17;
    public:
        /*
         * Default constructor initializes the object with the epoch date (November 17, 1858 at 00:00:00)
         */
        datetime();

        datetime(const struct tm &t);

        datetime(const datepart_t year, const datepart_t month, const datepart_t day,
            const datepart_t hour, const datepart_t minute, const datepart_t second);

        /*
         Initializes the object with the values taken from the ISO-formatted string like:
            "YYYY-MM-DD hh:mm:ss"
            "YYYY-MM-DDThh:mm:ss"
            "YYYY-MM-DD hh:mm"
            "YYYY-MM-DD"
            "hh:mm:ss"
            "hh:mm"
         Unspecified time values will be set to 0
         Unspecified date value will be set to current one
         */
        datetime(const char* str);
        datetime(const std::string& str);
        datetime(const wchar_t* str);
        datetime(const std::wstring& str);

        /*
         Copy constructor
         */
        datetime(const datetime& dt);

        /*
         Assignment operator
         */
        datetime& operator =(const datetime& dt);

        /*
         Move constructor
         */
        datetime(datetime&& dt) noexcept;

        /*
         Move assignment operator
         */
        datetime& operator =(datetime&& dt) noexcept;

        enum class datetime_unit
        {
            seconds,
            minutes,
            hours,
            days,
            months,
            years
        };

        enum class datepart
        {
            second,
            minute,
            hour,
            day_of_week,
            day_of_month,
            day_of_year,
            month,
            quarter,
            year
        };

        /*
         * Returns the part of date
         *
         * E.g. if the initial date is "10 Jan 1973"
         * get_datepart(year) returns 1973
         * get_datepart(month) returns 1
         * get_datepart(day_of_month) returns 10
         */
        datepart_t get_datepart(const datepart part) const noexcept(false);

        datepart_t year()        const { return get_datepart(datepart::year); }
        datepart_t month()       const { return get_datepart(datepart::month); }
        datepart_t quarter()     const { return datetime::get_quarter_of_month(month()); }
        datepart_t day()         const { return get_datepart(datepart::day_of_month); }
        datepart_t day_of_week() const { return datetime::day_of_week(*this); };
        datepart_t hour()        const { return get_datepart(datepart::hour); }
        datepart_t minute()      const { return get_datepart(datepart::minute); }
        datepart_t second()      const { return get_datepart(datepart::second); }

        /*
         Returns the number of the required date/time unit since the epoch
         If the format is TIME, returns the 0 for all date units.
         By example, the initial date is 01/01/1973 00:00:00 then
         get_value(hours) returns the number of hours since the epoch
         get_value(seconds) returns the number of seconds since the epoch
         */
        datepart_t get_value(datetime_unit unit) noexcept(false);

        /*
         Returns the date and time in format "YYYY-MM-DD hh:mm:ss"
         */
        std::string to_str();

        /* 
         Increments the date/time value. 
         The offset is specified in the date/time units
         */
        void increment(int offset, datetime_unit unit);

        /*
         Returns the modified Julian date (the number of days since 17/11/1858)
         of the date specified by the day/month/year
         */
        static datepart_t getMJDFromDate(datepart_t day, datepart_t month, datepart_t year);

        /*
         Returns the date structure given the number of days since 17/11/1858
         */
        static datetime getDateFromMJD(datepart_t days);

        /*
         Returns the quarter number given the month number
         */
        static datepart_t get_quarter_of_month(datepart_t month);

        /*
         Returns the day of week (1 - Monday, 2 - Tuesday, ... 7 - Sunday)
         of the date specified by the day/month/year
         */
        static datepart_t day_of_week(const datetime& dt);

        /*
         Returns the absolute value between two dates in specified time units
         Warning: only days, months and years units are accepted
         otherwise returns 0
         */
        static datepart_t diff(
            datepart_t day1, datepart_t month1, datepart_t year1,
            datepart_t day2, datepart_t month2, datepart_t year2,
            datetime_unit unit);

        /*
         Returns true if the specified year is a leap year
         */
        bool is_leap_year();
        static bool is_leap_year(const datetime& dt);
        static bool is_leap_year(const datepart_t year);

        /*
         Returns the number of days in the specified month/year
         */
        static datepart_t days_in_month(datepart_t month, datepart_t year);

        /*
         Returns current date and time
        */
        static datetime now();
    private:
        /*
         Common initializers
         */
        void init(const struct tm &t);
        void init(const datetime& dt);
        void parse(const char* str) noexcept(false);

        /*
         Validates the stored (m_data) date/time value
         */
        bool is_valid();

        /*
         Truncate the fractional part of the floating point value
         */
        inline static datepart_t trunc(double dv) { return (datepart_t)dv; }
    private:
        typedef struct datetime_data
        {
            datepart_t sec;    // seconds after the minute - [0, 59] including leap second
            datepart_t min;    // minutes after the hour - [0, 59]
            datepart_t hour;   // hours since midnight - [0, 23]
            datepart_t day;    // day of the month - [1, 31]
            datepart_t month;  // months since January - [0, 11]
            datepart_t year;   // year [0, 9999)
            datetime_data()
            {
                sec = min = hour = day = month = year = 0;
            }
        } datetime_data_t;

        datetime_data_t m_data;
    };
}

