#pragma once

#include "IODev.h"
#include <chrono>

namespace Airmar200WXH
{
    class Airmar200WXHController
    {
    public:
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

            enum eGPSfixType
            {
                Invalid = 0,
                GPSfix = 1,
                DGPSfix = 2,
            };

            bool valid = false;

            struct Latitude
            {
                bool valid =false;
                Timer timer = Timer();
                eLatitude type = No_Latitude;
                uint8_t degree = 0;
                uint8_t min = 0;
                uint16_t sec = 0;
            };

            struct Longitude
            {
                bool valid = false;
                Timer timer = Timer();
                eLongitude type = No_Longitude;
                uint8_t degree = 0;
                uint8_t min = 0;
                uint16_t sec = 0;
            };

            struct Altitude
            {
                bool valid = false;
                Timer timer = Timer();
                float altitudeM = 0;
            };

            int geoDiffStationID;
            int geoDiffLastUpTime;
            uint8_t satellitesInUse;
            eGPSfixType fixType;
            float HDOPfactor;
            Latitude latitude;
            Longitude longitude;
            Altitude altitude;
//            Geoidal separation (Diff. between WGS-84 earth ellipsoid and
//            mean sea level.  -=geoid is below WGS-84 ellipsoid)
            float geoSeparation;
        };

        struct Compass
        {
            enum eDeviation
            {
                No_Deviation = 0,
                East_Deviation = 1,
                West_Deviation = 2,

            };

            enum eVariation
            {
                No_Variation = 0,
                East_Variation = 1,
                West_Variation = 2,
            };

            bool valid = false;
            Timer timer = Timer();
            float heading = 0.0;
            float deviation = 0.0;
            eDeviation devType = No_Deviation;
            float variation = 0.0;
            eVariation varType = No_Variation;
        };

        struct Barometer
        {
            bool valid = false;
            Timer timer = Timer();
            float inchesPressure = 0;
            float barPressure = 0;
        };

        struct Temperature
        {
            bool valid = false;
            Timer timer = Timer();
            float TempCelsium = 0;
        };

        struct Humidity
        {
            bool valid = false;
            Timer timer = Timer();
            float relHumidity = 0;
            float absHumidity = 0;
            float dewPoint = 0;
        };

        struct NorthTrackSpeed
        {
            bool valid = false;
            Timer timer = Timer();
            float trackDegreesTrue = 0;
            float trackDegreesMagnetic = 0;
            float speedInMetersPerSecond = 0;
            float speedInKnots = 0;
        };

        class Wind
        {
        public:
            //To centerline
            float windAngleWithCenterline = 0;
            float windSpeedWithUnit = 0;

            //To north
            float windDirectionDegreesTrue = 0;
            float windDirectionDegreesMagnetic = 0;
            float windSpeedKnots = 0;
            float windSpeedMeters = 0;
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
            bool _valid = false;
            Timer _timer = Timer();
        public:
            Wind(){}
            ~Wind(){}
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

        public:
            float localTimeZoneHours;
            float localTimeZoneMinutes;
        private:
            Timer _timer;
            bool _valid = false;
            uint8_t _hour;
            uint8_t _min;
            uint8_t _sec;
        };

    public:
        explicit Airmar200WXHController(const SerialPortSettings& serialPortSettings, const uint8_t TRIES_BEFORE_FAIL);
        explicit Airmar200WXHController(const SocketClientSettings& socketClientSettings, const uint8_t TRIES_BEFORE_FAIL);
        virtual ~Airmar200WXHController();
	public:
		RetCode open();
		void close();
		RetCode checkConnetion();
    public:
        RetCode readDataLoop();
    private:
        //GPGGA group
        void _updateTime(const QByteArray& timeStr);
        void _updateGPSfixType(const QByteArray& fixTypeStr);
        void _updateSatellitesInUse(const QByteArray& satellitesInUseStr);
        void _updateLatitude(const QByteArray& latitudeStr, const QByteArray& latitudeTypeStr);
        void _updateLongitude(const QByteArray& longitudeStr, const QByteArray& longitudeTypeStr);
        void _updateAltitude(const QByteArray& altitudeStr);
        void _updateLocationStatus();
        void _updateHDOPfactor(const QByteArray& HDOPfactorStr);
        void _updateGeoSeparation(const QByteArray& geoSeparationStr);
        void _updateGeoDiffLastUpTime(const QByteArray& geoDiffLastUpTimeStr);
        void _updateGeoDiffStationId(const QByteArray& geoDiffStationIdStr);
        //GPZDA group
        void _updateDate(const QByteArray& dayStr, const QByteArray& monthStr, const QByteArray& yearStr);
        void _updateLocalTimeDescription(const QByteArray& localZoneDescrHoursStr,
                                         const QByteArray& localZoneDescrMinStr);

        //HCHDG group
        void _updateCompass(const QByteArray& headingStr,
                            const QByteArray& deviationStr,
                            const QByteArray& deviationDirectionStr,
                            const QByteArray& variationStr,
                            const QByteArray& variationDirectionStr);
        //WIMDA group
        void _updateBarometer(const QByteArray& inchesPressureStr,
                              const QByteArray& barPressureStr);
        void _updateAirTempCelsium(const QByteArray& airTempCelsiumStr);
        void _updateWaterTempCelsium(const QByteArray& waterTempCelsiumStr);
        void _updateHumidity(const QByteArray& relHumidityStr,
                             const QByteArray& absHumidityStr,
                             const QByteArray& dewPointStr);
        void _updateWind(const QByteArray& windDirectionDegreesTrueStr,
                         const QByteArray& windDirectionDegreesMagneticStr,
                         const QByteArray& windSpeedKnotsStr,
                         const QByteArray& windSpeedMetersStr);
        //GPVTG group
        void _updateNorthTrackSpeed(const QByteArray& trackDegreesTrueStr,
                               const QByteArray& trackDegreesMagneticStr,
                               const QByteArray& speedInKnotsStr,
                               const QByteArray& speedInKMetersPerHourStr);
        //WIMWV group
        void _updateWindWithUnit(const QByteArray& angle,
                                 const QByteArray& speed,
                                 const QByteArray& speedParam);
    private:
        void _checkIsDataOutdated();
    private:
        Airmar200WXHController() = delete;
	private:
		IODev _ioDev;
    private:
        Time _time;
        Date _date;
        Location _location;
        Compass _compass;
        Barometer _barometer;
        Temperature _airTemperature;
        Temperature _waterTemperature;
        Humidity _humidity;
        Wind _wind;
        NorthTrackSpeed _trackSpeed;
	private:
		const uint8_t TRIES_BEFORE_FAIL;
    };
}
