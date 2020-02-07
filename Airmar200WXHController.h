#pragma once

#include "IODev.h"
#include <chrono>

namespace Airmar200WXH
{
    class Airmar200WXHController
    {
        class Timer
        {
        private:
            std::chrono::high_resolution_clock::time_point _t0;
        public:
            Timer(): _t0(std::chrono::high_resolution_clock::now()){}

            int64_t elapsed()
            {
                const auto t1 = std::chrono::high_resolution_clock::now();
                const auto dt = std::chrono::duration<double, std::milli>(t1 - _t0);
                return dt.count();
            }

            void reset()
            {
                _t0 = std::chrono::high_resolution_clock::now();
            }

            ~Timer()
            {
//                qDebug() << "dt" << elapsed() << "microsec";
            }
        };

    public:
        struct Location
        {
            enum eLatitude
            {
                No_Latitude = 0,
                North = 1,
                South = 2,
            };

            enum eLongitude
            {
                No_Longitude = 0,
                East = 1,
                West = 2,
            };

            bool valid = false;

            Timer timer;
            uint16_t latitude;
            uint16_t longitude;
        };

        class Date
        {
        public:
            void    setDate(const uint8_t year,
                            const uint8_t month,
                            const uint8_t day)
            {
                _year = year;
                _month = month;
                _day = day;
            }
        public:
            Date(): _valid(false), _year(0), _month(0), _day(0) {}
            ~Date(){}
        public:
            void resetTimer()
            {
                _timer.reset();
            }

            void setValidState(bool state)
            {
                _valid = state;
            }

            int64_t elapsedMSecAfterUpdate()
            {
                return _timer.elapsed();
            }
        private:
            Timer _timer;
            bool _valid = false;
            uint16_t _year;
            uint8_t _month;
            uint8_t _day;
        };

        class Time
        {
        public:
            void    setTime(const uint8_t hour,
                            const uint8_t min,
                            const uint8_t sec)
            {
                _hour = hour;
                _min = min;
                _sec = sec;
            }

            bool  isValid() const { return _valid; }
            uint8_t getHour() const { return _hour; }
            uint8_t getMin() const { return _min; }
            uint8_t getSec() const { return _sec; }

        public:
            Time(): _valid(false), _hour(0), _min(0), _sec(0) {}
            ~Time(){}
        public:
            void resetTimer()
            {
                _timer.reset();
            }

            void setValidState(bool state)
            {
                _valid = state;
            }

            int64_t elapsedMSecAfterUpdate()
            {
                return _timer.elapsed();
            }

        private:
            Timer _timer;
            bool _valid = false;
            uint8_t _hour;
            uint8_t _min;
            uint8_t _sec;
        };

	public:
        explicit Airmar200WXHController(const SerialPortSettings& serialPortSettings, const uint8_t TRIES_BEFORE_FAIL);
        virtual ~Airmar200WXHController();
	public:
		RetCode open();
		void close();
		RetCode checkConnetion();
	public:
        RetCode readLocation(Location& loc);
    public:
        RetCode readDataLoop();
    private:
        Airmar200WXHController() = delete;
	private:
		IODev _ioDev;
    private:
        Time _time;
        Location _location;
	private:
		const uint8_t TRIES_BEFORE_FAIL;
    };
}
