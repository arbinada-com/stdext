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
                invalid_date_part_for_time_format,
                not_implemented
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
        typedef double jd_t;
        typedef long jdn_t;
        typedef short datepart_t;
        typedef struct data
        {
            datepart_t msec;   // milliseconds
            datepart_t sec;    // seconds after the minute - [0, 60] including leap second
            datepart_t min;    // minutes after the hour - [0, 59]
            datepart_t hour;   // hours since midnight - [0, 23]
            datepart_t day;    // day of the month - [1, 31]
            datepart_t month;  // months of year - [1, 12]
            datepart_t year;   // year starting from epoch 1 Jan 4713 BC (julian)
            data()
                : year(0), month(0), day(0), hour(0), min(0), sec(0), msec(0)
            { }
            data(const datepart_t year, const datepart_t month, const datepart_t day)
                : year(year), month(month), day(day), hour(0), min(0), sec(0), msec(0)
            { }
            data(const datepart_t year, const datepart_t month, const datepart_t day,
                 const datepart_t hour, const datepart_t minute, const datepart_t second, 
                 const datepart_t millisecond)
                : year(year), month(month), day(day), hour(hour), min(minute), sec(second), msec(millisecond)
            { }
        } data_t;
    public:
        /*
         * Default constructor initializes the object with the zero date (November 17, 1858 at 00:00:00)
         */
        datetime();

        datetime(const struct tm &t);

        datetime(const datepart_t year, const datepart_t month, const datepart_t day,
                 const datepart_t hour, const datepart_t minute, const datepart_t second, 
                 const datepart_t millisecond);

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

        enum class calendar
        {
            julian,
            gregorian
        };

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
            millisecond,
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
        datepart_t hour()        const { return get_datepart(datepart::hour); }
        datepart_t minute()      const { return get_datepart(datepart::minute); }
        datepart_t second()      const { return get_datepart(datepart::second); }
        datepart_t millisecond() const { return get_datepart(datepart::millisecond); }

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
         Returns the quarter number given the month number
         */
        static datepart_t get_quarter_of_month(datepart_t month);

        /*
         Returns the ISO number of day of week (1 - Monday, 2 - Tuesday, ... 7 - Sunday)
         */
        static datepart_t day_of_week(const jd_t& jd);
        datepart_t day_of_week() const;

        /*
         Returns the absolute value between two dates in specified time units
         Warning: only days, months and years units are accepted
         otherwise returns 0
         */
        static long diff(
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

        /*
         Julian date (JD) and day number (JDN) calculations
         */
        static jdn_t gregorian_to_jdn(const datepart_t year, const datepart_t month, const datepart_t day);
        static jdn_t julian_to_jdn(const datepart_t year, const datepart_t month, const datepart_t day);
        static jd_t gregorian_to_jd(const data_t& dt);
        static jd_t julian_to_jd(const data_t& dt);
        static data_t jd_to_calendar(const datetime::calendar cal, const jd_t jd);
        static jdn_t jd_to_jdn(const jd_t jd);
        static jd_t jdn_to_jd(const jdn_t jdn, 
                              const datepart_t hour, const datepart_t minute, const datepart_t second,
                              const datepart_t millisecond);
        static inline jd_t jd_to_mjd(const jd_t jd) { return jd - 2400000.5; }
        static data_t hh_to_hms(const double hh);
        static double hms_to_hh(const data_t time);

        jd_t jd() { return m_jd; }
        jd_t mjd() { return jd_to_mjd(m_jd); }
        jdn_t jdn() { return jd_to_jdn(m_jd); }
    private:
        /*
         Common initializers
         */
        void parse(const char* str) noexcept(false);

        /*
         Validates stored date/time value
         */
        bool is_valid();
    private:
        jd_t m_jd = 0.0;

    };
}

