/*
 C++ standard library extensions
 (c) 2001-2021 Serguei Tarassov (see license.txt)

 The datetime class helps to manipulate date and time values
 */
#pragma once

#include <time.h>
#include <string>
#include <exception>

namespace stdext
{

    typedef int datepart_t;
    enum class calendar_t
    {
        julian,
        gregorian,
        date_default
    };

    enum class dtunit_t
    {
        seconds,
        minutes,
        hours,
        days,
        months,
        years
    };

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
                unknown_calendar,
                not_implemented
            };
        public:
            explicit datetime_exception(const datetime_exception::kind kind);
            datetime_exception(const datetime_exception::kind kind, const char* value);
            datetime_exception(const datetime_exception::kind kind, const int value);
            const char* what() const noexcept override;
        private:
            static std::string msg_by_kind(const datetime_exception::kind kind);
            std::string msg_by_kind() { return msg_by_kind(m_kind); }
        private:
            std::string              m_message;
            datetime_exception::kind m_kind;
    };


    class datetime
    {
    public:
        constexpr static short SECONDS_IN_MINUTE = 60;
        constexpr static short MINUTES_IN_HOUR = 60;
        constexpr static short SECONDS_IN_HOUR = SECONDS_IN_MINUTE * MINUTES_IN_HOUR;
        constexpr static short HOURS_IN_DAY = 24;
        constexpr static int   SECONDS_IN_DAY = SECONDS_IN_HOUR * HOURS_IN_DAY;
        constexpr static int   MINUTES_IN_DAY = MINUTES_IN_HOUR * HOURS_IN_DAY;
        constexpr static short MONTHS_IN_YEAR = 12;

    public:
        typedef double jd_t;
        typedef long jdn_t;

        typedef struct data
        {
            datepart_t year;   // year starting from epoch 1 Jan 4713 BC (julian)
            datepart_t month;  // months of year - [1, 12]
            datepart_t day;    // day of the month - [1, 31]
            datepart_t hour;   // hours since midnight - [0, 23]
            datepart_t min;    // minutes after the hour - [0, 59]
            datepart_t sec;    // seconds after the minute - [0, 60] including leap second
            datepart_t msec;   // milliseconds
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
            data(const data&) = default;
            data& operator =(const data&) = default;
            data(data&&) = default;
            data& operator =(data&&) = default;
        } data_t;

    public:
        /*
         * Default constructor initializes the object with the zero date (November 17, 1858 at 00:00:00)
         */
        datetime();
        explicit datetime(const struct tm &t);
        datetime(const datepart_t year, const datepart_t month, const datepart_t day,
                 const datepart_t hour, const datepart_t minute, const datepart_t second,
                 const datepart_t millisecond,
                 const calendar_t cal = calendar_t::date_default);
        datetime(const datepart_t year, const datepart_t month, const datepart_t day,
                 const calendar_t cal = calendar_t::date_default);
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
        explicit datetime(const char* str);
        explicit datetime(const std::string& str);
        explicit datetime(const wchar_t* str);
        explicit datetime(const std::wstring& str);

        /*
         * Copy/move constructors and assigment operators
         */
        datetime(const datetime& dt) = default;
        datetime& operator =(const datetime& dt) = default;
        datetime(datetime&& dt) noexcept = default;
        datetime& operator =(datetime&& dt) noexcept = default;

        /*
         * Operators
         */
        bool operator ==(const datetime& rhs) const { return this->equal(rhs); }
        bool operator !=(const datetime& rhs) const { return !this->equal(rhs); }
        bool equal(const datetime& val) const;
        bool operator <(const datetime& rhs) const { return jd() < rhs.jd(); }
        bool operator <=(const datetime& rhs) const { return jd() <= rhs.jd(); }
        bool operator >(const datetime& rhs) const { return jd() > rhs.jd(); }
        bool operator >=(const datetime& rhs) const { return jd() >= rhs.jd(); }

        /*
         * Returns the parts of date
         *
         * E.g. if the initial date is "10 Jan 1973"
         * year() returns 1973
         * month() returns 1
         * day() returns 10
         */
        datepart_t year()        const;
        datepart_t month()       const;
        datepart_t day()         const;
        datepart_t day_of_year() const;
        datepart_t hour()        const;
        datepart_t minute()      const;
        datepart_t second()      const;
        datepart_t millisecond() const;
        /*
         Returns the ISO number of day of week (1 - Monday, 2 - Tuesday, ... 7 - Sunday)
         */
        datepart_t day_of_week() const;
        static datepart_t day_of_week(const jd_t& jd);
        /*
         Returns the quarter number given the month number
         */
        datepart_t quarter() const;
        static datepart_t get_quarter_of_month(datepart_t month);


        /*
         Returns the date and time in the specified format or
         in the ISO format "YYYY-MM-DD hh:mm:ss.zzz" by default
         */
        std::string to_string();
        std::string to_string(const std::string& format);

        /*
         Increments the date/time value.
         The offset is specified in the date/time units
         */
        datetime& inc(const dtunit_t unit, const int offset) noexcept(false);

        /*
         Returns the absolute value between two dates in specified time units
         Warning: only days, months and years units are accepted
         otherwise throw exception
         */
        long diff(const dtunit_t unit, const datetime& dt_then) const noexcept(false);

        /*
         Returns true if the specified year is a leap year
         */
        bool is_leap_year() const;
        static bool is_leap_year(const datetime& dt);
        static bool is_leap_year(const datepart_t year);

        /*
         Returns the number of days in the specified month/year
         */
        static datepart_t days_in_month(datepart_t month, datepart_t year) noexcept(false);

        /*
         Returns current date and time
        */
        static datetime now();

        /*
         Julian date (JD) and day number (JDN) calculations
         */
        static jd_t calendar_to_jd(const calendar_t cal, const data_t& dt);
        static data_t jd_to_calendar(const calendar_t cal, const jd_t jd);
        static inline jd_t jd_to_mjd(const jd_t jd) { return jd - 2400000.5; }
        static data_t hh_to_hms(const double hh);
        static double hms_to_hh(const data_t& time);

        jd_t jd() const { return m_jd; }
        jd_t mjd() const { return jd_to_mjd(m_jd); }
        calendar_t calendar() const { return m_cal; }

    private:
        /*
         Common initializers
         */
        void init(const datepart_t year, const datepart_t month, const datepart_t day,
                  const datepart_t hour, const datepart_t minute, const datepart_t second,
                  const datepart_t millisecond,
                  const calendar_t cal = calendar_t::date_default);
        void parse(const char* str) noexcept(false);

        static void trunc_to_jd_grain(jd_t& jd);
        static jd_t jd_grain();

        /*
         Validates stored date/time value
         */
        bool is_valid();
        static bool is_valid(jd_t jd);
    private:
        jd_t       m_jd = 0.0;
        calendar_t m_cal = calendar_t::julian;
    };
}

