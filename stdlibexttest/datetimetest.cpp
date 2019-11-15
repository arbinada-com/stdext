#include "stdafx.h"
#include "CppUnitTest.h"
#include "datetime.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

namespace stdexttest
{		
	TEST_CLASS(DatetimeTest)
	{
    private:
        void CheckTimeIsZero(const datetime& dt, const wstring title)
        {
            Assert::AreEqual((datepart_t)0, dt.hour(), title.c_str());
            Assert::AreEqual((datepart_t)0, dt.minute(), title.c_str());
            Assert::AreEqual((datepart_t)0, dt.second(), title.c_str());
            Assert::AreEqual((datepart_t)0, dt.millisecond(), title.c_str());
        }

        void CheckDateParts(
            const datetime& dt,
            const datepart_t year, const datepart_t month, const datepart_t day,
            const datepart_t hour, const datepart_t minute, const datepart_t second, 
            const datepart_t msec,
            const wstring title)
        {
            Assert::AreEqual(year, dt.year(), (title + L": year").c_str());
            Assert::AreEqual(month, dt.month(), (title + L": month").c_str());
            Assert::AreEqual(day, dt.day(), (title + L": day").c_str());
            Assert::AreEqual(hour, dt.hour(), (title + L": hour").c_str());
            Assert::AreEqual(minute, dt.minute(), (title + L": minute").c_str());
            Assert::AreEqual(second, dt.second(), (title + L": second").c_str());
            Assert::AreEqual(msec, dt.millisecond(), (title + L": millisecond").c_str());
        }
        void CheckDateParts(
            const datetime& dt,
            const datepart_t year, const datepart_t month, const datepart_t day,
            const wstring title)
        {
            CheckDateParts(dt, year, month, day, 0, 0, 0, 0, title);
        }
        
        void CompareDates(const datetime& dt1, const datetime& dt2, const wstring title)
        {
            Assert::AreEqual((int)dt1.calendar(), (int)dt2.calendar(), (title + L": calendar").c_str());
            CheckDateParts(dt1, dt2.year(), dt2.month(), dt2.day(), dt2.hour(), dt2.minute(), dt2.second(), dt2.millisecond(), title);
        }

        void CheckTimeParts(
            const datetime& dt,
            const datepart_t hour, const datepart_t minute, const datepart_t second, 
            const datepart_t msec,
            const wstring title)
        {
            CheckDateParts(dt, 0, 0, 0, hour, minute, second, msec, title);
        }

        void CompareDatetimeData(const datetime::data_t& expected, const datetime::data_t& value, wstring title)
        {
            Assert::AreEqual(expected.year, value.year, (title + L": year").c_str());
            Assert::AreEqual(expected.month, value.month, (title + L": month").c_str());
            Assert::AreEqual(expected.day, value.day, (title + L": day").c_str());
            Assert::AreEqual(expected.hour, value.hour, (title + L": hour").c_str());
            Assert::AreEqual(expected.min, value.min, (title + L": minute").c_str());
            Assert::AreEqual(expected.sec, value.sec, (title + L": second").c_str());
            Assert::AreEqual(expected.msec, value.msec, (title + L": millisecond").c_str());
        }
	public:
        TEST_METHOD(TestTimePart)
        {
            datetime::data_t dt1(0, 0, 0, 18, 0, 0, 0);
            double hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.75 - hh) < 0.0001, L"HH 1.1");
            datetime::data_t dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 1.2");
            //
            dt1 = datetime::data_t(0, 0, 0, 6, 0, 0, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.25 - hh) < 0.0001, L"HH 2.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 2.2");
            //
            dt1 = datetime::data_t(0, 0, 0, 18, 31, 27, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.77184028 - hh) < 1E-7, L"HH 3.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 3.2");
            //
            dt1 = datetime::data_t(0, 0, 0, 10, 5, 43, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.42063657 - hh) < 1E-7, L"HH 4.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 4.2");
            // Extreme time values
            dt1 = datetime::data_t(0, 0, 0, 0, 0, 0, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.0 - hh) < 0.01, L"HH 5.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 5.2");
            dt1 = datetime::data_t(0, 0, 0, 23, 59, 59, 999);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.999 - hh) < 0.01, L"HH 6.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 6.2");
            // Noon
            dt1 = datetime::data_t(0, 0, 0, 12, 0, 0, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.5 - hh) < 0.01, L"HH 7.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 7.2");
        }

        TEST_METHOD(TestJulianDateCalcs)
        {
            // Epoch date 1 Jan 4713 BC 12:00 (julian) = 24 Nov 4714 BC 12:00 (gregorian)
            // Important note: for NNNN BC years the year parameter is specified as -(NNNN + 1)
            // I.e. 4713 BC => -4712, 1 BC => 0
            //Assert::AreEqual(0.0, datetime::calendar_to_jd(calendar_t::julian, datetime::data_t(-4712, 1, 1)), L"Epoch day 1");
            datetime::data_t d1 = datetime::data_t(-4712, 1, 1, 12, 0, 0, 0);
            datetime::jd_t jd1 = datetime::calendar_to_jd(calendar_t::julian, d1);
            Assert::AreEqual(0.0, jd1, L"Epoch date J1");
            datetime::data_t d2 = datetime::jd_to_calendar(calendar_t::julian, 0.0);
            CompareDatetimeData(d1, d2, L"Epoch date J2");
            d1 = datetime::data_t(-4713, 11, 24, 12, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::gregorian, d1);
            Assert::AreEqual(0.0, jd1, L"Epoch date G1");
            d2 = datetime::jd_to_calendar(calendar_t::gregorian, 0.0);
            CompareDatetimeData(d1, d2, L"Epoch date G2");
            // Other dates
            //
            d1 = datetime::data_t(-3001, 5, 10, 0, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::julian, d1);
            Assert::AreEqual(625071.5, jd1, L"10 May 3000 BC at 00:00:00 - 1");
            d2 = datetime::jd_to_calendar(calendar_t::julian, jd1);
            CompareDatetimeData(d1, d2, L"10 May 3000 BC at 00:00:00 - 2");
            //
            d1 = datetime::data_t(-1000, 4, 20, 0, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::julian, d1);
            Assert::AreEqual(1355917.5, jd1, L"20 Avr 999 BC at 00:00:00 - 1");
            d2 = datetime::jd_to_calendar(calendar_t::julian, jd1);
            CompareDatetimeData(d1, d2, L"20 Avr 999 BC at 00:00:00 - 2");
            //
            d1 = datetime::data_t(-1000, 2, 29, 0, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::julian, d1);
            Assert::AreEqual(1355866.5, jd1, L"29 Feb 999 BC - 1");
            d2 = datetime::jd_to_calendar(calendar_t::julian, jd1);
            CompareDatetimeData(d1, d2, L"29 Feb 999 BC - 2");
            //
            // First Gregorian day
            d1 = datetime::data_t(1582, 10, 15, 0, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::gregorian, d1);
            Assert::AreEqual(2299160.5, jd1, L"15 Oct 1582 - 1");
            d2 = datetime::jd_to_calendar(calendar_t::gregorian, jd1);
            CompareDatetimeData(d1, d2, L"15 Oct 1582 - 2");
            // Using julian dates in gregorian epoch
            //
            d1 = datetime::data_t(1815, 8, 15, 6, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::julian, d1);
            Assert::AreEqual(2384212.75, jd1, L"15 Aug 1815 - 1");
            d2 = datetime::jd_to_calendar(calendar_t::julian, jd1);
            CompareDatetimeData(d1, d2, L"15 Aug 1815 - 2");
            //
            d1 = datetime::data_t(1985, 2, 17, 6, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::gregorian, d1);
            Assert::AreEqual(2446113.75, jd1, L"1985-02-17 at 06:00:00 - 1");
            d2 = datetime::jd_to_calendar(calendar_t::gregorian, jd1);
            CompareDatetimeData(d1, d2, L"1985-02-17 at 06:00:00 - 2");
            Assert::AreEqual(2446113.5, datetime::calendar_to_jd(calendar_t::gregorian, datetime::data_t(1985, 2, 17)), L"1985-02-17 at 00:00:00");
            //
            d1 = datetime::data_t(1980, 1, 1, 0, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::gregorian, d1);
            Assert::AreEqual(2444239.5, jd1, L"1980-01-01 at 00:00:00 - 1");
            d2 = datetime::jd_to_calendar(calendar_t::gregorian, jd1);
            CompareDatetimeData(d1, d2, L"1980-01-01 at 00:00:00 - 2");
            //
            d1 = datetime::data_t(2000, 1, 1, 12, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::gregorian, d1);
            Assert::AreEqual(2451545.0, jd1, L"1 Jan 2000 at 12:00:00 - 1");
            d2 = datetime::jd_to_calendar(calendar_t::gregorian, jd1);
            CompareDatetimeData(d1, d2, L"1 Jan 2000 at 12:00:00 - 2");
            //
            d1 = datetime::data_t(2000, 1, 1, 18, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::gregorian, d1);
            Assert::AreEqual(2451545.25, jd1, L"1 Jan 2000 at 18:00:00 - 1");
            d2 = datetime::jd_to_calendar(calendar_t::gregorian, jd1);
            CompareDatetimeData(d1, d2, L"1 Jan 2000 at 12:00:00 - 2");
            //
            d1 = datetime::data_t(1942, 8, 2, 15, 0, 0, 0);
            jd1 = datetime::calendar_to_jd(calendar_t::gregorian, d1);
            Assert::AreEqual(2430574.125, jd1, L"2 Aug 1942 at 15:00:00 - 1");
            d2 = datetime::jd_to_calendar(calendar_t::gregorian, jd1);
            CompareDatetimeData(d1, d1, L"2 Aug 1942 at 15:00:00 - 1");
        }

        TEST_METHOD(TestConstructors)
        {
            datetime d11;
            Assert::IsTrue(d11.calendar() == calendar_t::julian, L"ctor 1.1");
            CheckDateParts(d11, -4712, 1, 1, 12, 0, 0, 0, L"ctor 1.2");
            datetime d12("0001-01-01");
            Assert::IsTrue(d12.calendar() == calendar_t::julian, L"ctor 1.3");
            //
            CheckDateParts(datetime("1999-02-03"), 1999, 02, 03, L"ctor 2.1");
            CheckDateParts(datetime("1900-01-01"), 1900, 01, 01, L"ctor 2.2");
            CheckDateParts(datetime("2000-10-11"), 2000, 10, 11, L"ctor 2.3");
            datetime d21("1582-10-15");
            Assert::IsTrue(d21.calendar() == calendar_t::gregorian, L"ctor 2.4");
            datetime d22("1999-01-01");
            Assert::IsTrue(d22.calendar() == calendar_t::gregorian, L"ctor 2.5");
            //
            CheckDateParts(datetime("2019-10-11T01:02:03"), 2019, 10, 11, 1, 2, 3, 0, L"ctor 3.1");
            CheckDateParts(datetime("2019-10-11 11:22:33"), 2019, 10, 11, 11, 22, 33, 0, L"ctor 3.2");
            //
            datetime d41 = datetime::now();
            CheckDateParts(datetime("01:02:03"), d41.year(), d41.month(), d41.day(), 1, 2, 3, 0, L"ctor 4.1");
            CheckDateParts(datetime("11:22:33"), d41.year(), d41.month(), d41.day(), 11, 22, 33, 0, L"ctor 4.2");
            CheckDateParts(datetime("22:33"), d41.year(), d41.month(), d41.day(), 22, 33, 0, 0, L"ctor 4.3");
            //
            CheckDateParts(datetime("2019-10-11T00:00:00.000"), 2019, 10, 11, 0, 0, 0, 0, L"ctor 5.1");
            CheckDateParts(datetime("2019-10-11T01:02:03.456"), 2019, 10, 11, 1, 2, 3, 456, L"ctor 5.1");
            CheckDateParts(datetime("2019-10-11T23:59:59.999"), 2019, 10, 11, 23, 59, 59, 999, L"ctor 5.3");
            //
            // Copy/move constructors
            {
                datetime d21("2019-01-02T03:04:05");
                CheckDateParts(d21, 2019, 1, 2, 3, 4, 5, 0, L"ctor copy 1");
                datetime d22(d21);
                CompareDates(d21, d22, L"ctor copy 2");
                datetime d23 = d21;
                CompareDates(d21, d23, L"ctor copy 3");
                datetime d31(std::move(d21));
                CheckDateParts(d31, 2019, 1, 2, 3, 4, 5, 0, L"ctor move 1");
                datetime d32 = std::move(d22);
                CheckDateParts(d32, 2019, 1, 2, 3, 4, 5, 0, L"ctor move 2");
            }
        }

        TEST_METHOD(TestQuarterOfMonth)
        {
            auto chkQuarter = [](datepart_t month, datepart_t quarter)
            {
                char s[] = "9999-99-99";
                sprintf_s(s, sizeof(s), "2019-%2d-01", month);
                datetime d(s);
                Assert::AreEqual(quarter, datetime::get_quarter_of_month(month), L"get_quarter_of_month");
            };
            chkQuarter(1, 1);
            chkQuarter(2, 1);
            chkQuarter(3, 1);
            chkQuarter(4, 2);
            chkQuarter(5, 2);
            chkQuarter(6, 2);
            chkQuarter(7, 3);
            chkQuarter(8, 3);
            chkQuarter(9, 3);
            chkQuarter(10, 4);
            chkQuarter(11, 4);
            chkQuarter(12, 4);
        }
        
        TEST_METHOD(TestDateParts)
		{
            datetime d1("2019-10-20 21:22:23");
            Assert::AreEqual((datepart_t)2019, d1.year(), L"year 1");
            Assert::AreEqual((datepart_t)10, d1.month(), L"month 1");
            Assert::AreEqual((datepart_t)4, d1.quarter(), L"quarter 1");
            Assert::AreEqual((datepart_t)20, d1.day(), L"day 1");
            Assert::AreEqual((datepart_t)7, d1.day_of_week(), L"day_of_week 1");
            Assert::AreEqual((datepart_t)21, d1.hour(), L"hour 1");
            Assert::AreEqual((datepart_t)22, d1.minute(), L"minute 1");
            Assert::AreEqual((datepart_t)23, d1.second(), L"second 1");
            datetime d2("1999-01-02 03:04:05.678");
            Assert::AreEqual((datepart_t)1999, d2.year(), L"year 2");
            Assert::AreEqual((datepart_t)1, d2.month(), L"month 2");
            Assert::AreEqual((datepart_t)1, d2.quarter(), L"quarter 2");
            Assert::AreEqual((datepart_t)2, d2.day(), L"day 2");
            Assert::AreEqual((datepart_t)6, d2.day_of_week(), L"day_of_week 2");
            Assert::AreEqual((datepart_t)3, d2.hour(), L"hour 2");
            Assert::AreEqual((datepart_t)4, d2.minute(), L"minute 2");
            Assert::AreEqual((datepart_t)5, d2.second(), L"second 2");
            Assert::AreEqual((datepart_t)678, d2.millisecond(), L"millisecond 2");
        }

        TEST_METHOD(TestDayOfWeek)
        {
            Assert::AreEqual((datepart_t)1, datetime("2019-11-04").day_of_week(), L"DOW 1.1");
            Assert::AreEqual((datepart_t)2, datetime("2019-11-05").day_of_week(), L"DOW 1.2");
            Assert::AreEqual((datepart_t)3, datetime("2019-11-06").day_of_week(), L"DOW 1.3");
            Assert::AreEqual((datepart_t)4, datetime("2019-11-07").day_of_week(), L"DOW 1.4");
            Assert::AreEqual((datepart_t)5, datetime("2019-11-08").day_of_week(), L"DOW 1.5");
            Assert::AreEqual((datepart_t)6, datetime("2019-11-09").day_of_week(), L"DOW 1.6");
            Assert::AreEqual((datepart_t)7, datetime("2019-11-10").day_of_week(), L"DOW 1.7");
            Assert::AreEqual((datepart_t)7, datetime("2019-11-10 12:00:00").day_of_week(), L"DOW 1.71");
            Assert::AreEqual((datepart_t)7, datetime("2019-11-10 23:59:59").day_of_week(), L"DOW 1.72");
            Assert::AreEqual((datepart_t)7, datetime("1985-02-17").day_of_week(), L"DOW 2");
        }

        TEST_METHOD(TestDayOfYear)
        {
            Assert::AreEqual((datepart_t)1, datetime(-4712, 1, 1, 12, 0, 0, 0, calendar_t::julian).day_of_year(), L"DOY 1");
            Assert::AreEqual((datepart_t)1, datetime("2019-01-01").day_of_year(), L"DOY 2");
        }

        TEST_METHOD(TestDateDiff)
        {
            Assert::AreEqual(366, (int)datetime("2012-01-01").diff(datetime_unit_t::days, datetime("2013-01-01")), L"Diff 1.1");
        }

    };
}