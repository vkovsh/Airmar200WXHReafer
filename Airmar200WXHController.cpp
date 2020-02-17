#include "Airmar200WXHController.h"
#include <QDebug>
#include <QList>

using namespace Airmar200WXH;

Airmar200WXHController::Airmar200WXHController(const SerialPortSettings& serialPortSettings,
                                               const uint8_t TRIES_BEFORE_FAIL):
                                                _ioDev(serialPortSettings),
                                                TRIES_BEFORE_FAIL(TRIES_BEFORE_FAIL){}

Airmar200WXHController::~Airmar200WXHController()
{}

RetCode Airmar200WXHController::open()
{
    return _ioDev.open();
}

void Airmar200WXHController::close()
{
    _ioDev.close();
}

RetCode Airmar200WXHController::checkConnetion()
{
    RetCode ret = _ioDev.isOpen();
    if (ret == RetCode::RC_SUCCESS)
        return ret;
    ret = _ioDev.open();
    return ret;
}

void Airmar200WXHController::_updateTime(const QByteArray& timeStr)
{
    if ((false == timeStr.isEmpty()) && (timeStr.size() >= 6))
    {
        const uint8_t hour = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
        const uint8_t min = (timeStr[2] - '0') * 10 + (timeStr[3] - '0');
        const uint8_t sec = (timeStr[4] - '0') * 10 + (timeStr[5] - '0');
        _time.setTime(hour, min, sec);
        _time.resetTimer();
        _time.setValidState(true);
    }
}

void Airmar200WXHController::_updateLatitude(const QByteArray& latitudeStr, const QByteArray& latitudeTypeStr)
{
    if ((false == latitudeStr.isEmpty()) && (latitudeStr.size() >= 9))
    {
        _location.latitude.degree = (latitudeStr[0] - '0') * 10 + (latitudeStr[1] - '0');
        _location.latitude.min = (latitudeStr[2] - '0') * 10 + (latitudeStr[3] - '0');
        _location.latitude.sec = std::atoi(&latitudeStr.data()[5]);
        _location.latitude.type = (latitudeTypeStr == "N") ? Location::North : Location::South;
        _location.latitude.timer.reset();
        _location.latitude.valid = true;
    }
}

void Airmar200WXHController::_updateLongitude(const QByteArray& longitudeStr, const QByteArray& longitudeTypeStr)
{
    if ((false == longitudeStr.isEmpty()) && (longitudeStr.size() >= 10))
    {
        _location.longitude.degree = (longitudeStr[0] - '0') * 100 + (longitudeStr[1] - '0') * 10 + (longitudeStr[2] - '0');
        _location.longitude.min = (longitudeStr[3] - '0') * 10 + (longitudeStr[4] - '0');
        _location.longitude.sec = std::atoi(&longitudeStr.data()[6]);
        _location.longitude.type = (longitudeTypeStr == "E") ? Location::East : Location::West;
        _location.longitude.timer.reset();
        _location.longitude.valid = true;
    }
}

void Airmar200WXHController::_updateAltitude(const QByteArray& altitudeStr)
{
    if (altitudeStr.isEmpty() == false)
    {
        _location.altitude.altitudeM = altitudeStr.toFloat();
        _location.altitude.valid = true;
        _location.altitude.timer.reset();
    }
}

void    Airmar200WXHController::_updateLocationStatus()
{
    _location.valid = _location.latitude.valid &&
            _location.longitude.valid &&
            _location.altitude.valid;
}

void Airmar200WXHController::_updateDate(const QByteArray& dayStr, const QByteArray& monthStr, const QByteArray& yearStr)
{
    if ((false == dayStr.isEmpty()) && (false == monthStr.isEmpty()) && (false == yearStr.isEmpty()))
    {
        const int day = (dayStr[0] - '0') * 10 + (dayStr[1] - '0');
        const int month = (monthStr[0] - '0') * 10 + (monthStr[1] - '0');
        const int year = (yearStr[0] - '0') * 10 + (yearStr[1] - '0');
        _date.setDate(day, month, year);
        _date.resetTimer();
        _date.setValidState(true);
    }
}

void Airmar200WXHController::_updateCompass(const QByteArray& headingStr,
                    const QByteArray& deviationStr,
                    const QByteArray& deviationDirectionStr,
                    const QByteArray& variationStr,
                    const QByteArray& variationDirectionStr)
{
    if ((false == headingStr.isEmpty()) &&
            (false == deviationStr.isEmpty()) &&
            (false == deviationDirectionStr.isEmpty()) &&
            (false == variationStr.isEmpty()) &&
            (false == variationDirectionStr.isEmpty()))
    {
        _compass.heading = headingStr.toFloat();
        _compass.deviation = deviationStr.toFloat();
        _compass.devType = (deviationDirectionStr == "E") ?
                    Compass::East_Deviation : Compass::West_Deviation;
        _compass.variation = variationStr.toFloat();
        _compass.varType = (variationDirectionStr == "E") ?
                    Compass::East_Variation : Compass::West_Variation;
        _compass.valid = true;
        _compass.timer.reset();
    }
}


void Airmar200WXHController::_updateBarometer(const QByteArray& inchesPressureStr,
                      const QByteArray& barPressureStr)
{
    if ((false == inchesPressureStr.isEmpty()) ||
            (false == barPressureStr.isEmpty()))
    {
        _barometer.inchesPressure = inchesPressureStr.toFloat();
        _barometer.barPressure = barPressureStr.toFloat();
        _barometer.timer.reset();
        _barometer.valid = true;
    }
}


void Airmar200WXHController::_updateAirTempCelsium(const QByteArray& airTempCelsiumStr)
{
    if (airTempCelsiumStr.isEmpty() == false)
    {
        _airTemperature.TempCelsium = airTempCelsiumStr.toFloat();
        _airTemperature.valid = true;
        _airTemperature.timer.reset();
    }
}

void Airmar200WXHController::_updateWaterTempCelsium(const QByteArray& waterTempCelsiumStr)
{
    if (waterTempCelsiumStr.isEmpty() == false)
    {
        _waterTemperature.TempCelsium = waterTempCelsiumStr.toFloat();
        _waterTemperature.valid = true;
        _waterTemperature.timer.reset();
    }
}

void Airmar200WXHController::_updateHumidity(const QByteArray& relHumidityStr,
                     const QByteArray& absHumidityStr,
                     const QByteArray& dewPointStr)
{
    if ((false == relHumidityStr.isEmpty()) &&
            (false == absHumidityStr.isEmpty()) &&
            (false == dewPointStr.isEmpty()))
    {
        _humidity.relHumidity = relHumidityStr.toFloat();
        _humidity.absHumidity = absHumidityStr.toFloat();
        _humidity.dewPoint = dewPointStr.toFloat();
        _humidity.valid = true;
        _humidity.timer.reset();
    }
}

void Airmar200WXHController::_updateWind(const QByteArray& windDirectionDegreesTrueStr,
                 const QByteArray& windDirectionDegreesMagneticStr,
                 const QByteArray& windSpeedKnotsStr,
                 const QByteArray& windSpeedMetersStr)
{
    if (windDirectionDegreesTrueStr.isEmpty() == false &&
            windDirectionDegreesMagneticStr.isEmpty() == false &&
            windSpeedKnotsStr.isEmpty() == false &&
            windSpeedMetersStr.isEmpty() == false)
    {
        _wind.windDirectionDegreesTrue = windDirectionDegreesTrueStr.toFloat();
        _wind.windDirectionDegreesMagnetic = windDirectionDegreesMagneticStr.toFloat();
        _wind.windSpeedKnots = windSpeedKnotsStr.toFloat();
        _wind.windSpeedMeters = windSpeedMetersStr.toFloat();
        _wind.resetTimer();
        _wind.setValidState(true);
    }
}

void Airmar200WXHController::_updateGPSfixType(const QByteArray& fixTypeStr)
{
    if (fixTypeStr.isEmpty() == false)
    {
        switch (fixTypeStr[0] - '0')
        {
        case 0:
            _location.fixType = Location::Invalid;
            break ;
        case 1:
            _location.fixType = Location::GPSfix;
            break ;
        case 2:
            _location.fixType = Location::DGPSfix;
            break ;
        default:
            _location.fixType = Location::Invalid;
            break;
        }
    }
}

void Airmar200WXHController::_updateSatellitesInUse(const QByteArray& satellitesInUseStr)
{
    if (satellitesInUseStr.isEmpty() == false)
    {
        _location.satellitesInUse = std::atoi(satellitesInUseStr.data());
    }
}

void Airmar200WXHController::_updateHDOPfactor(const QByteArray& HDOPfactorStr)
{
    if (HDOPfactorStr.isEmpty() == false)
    {
        _location.HDOPfactor = HDOPfactorStr.toFloat();
    }
}

void Airmar200WXHController::_updateGeoSeparation(const QByteArray& geoSeparationStr)
{
    if (geoSeparationStr.isEmpty() == false)
    {
        _location.geoSeparation = geoSeparationStr.toFloat();
    }
}

void Airmar200WXHController::_updateGeoDiffLastUpTime(const QByteArray& geoDiffLastUpTimeStr)
{
    if (geoDiffLastUpTimeStr.isEmpty() == false)
    {
        _location.geoDiffLastUpTime = geoDiffLastUpTimeStr.toInt();
    }
}

void Airmar200WXHController::_updateGeoDiffStationId(const QByteArray& geoDiffStationIdStr)
{
    if (geoDiffStationIdStr.isEmpty() == false)
    {
        _location.geoDiffStationID = geoDiffStationIdStr.toInt();
    }
}

RetCode Airmar200WXHController::readDataLoop()
{
    {
        RetCode ret = checkConnetion();
        if (ret != RetCode::RC_SUCCESS)
        return ret;
    }

//    qDebug() << _airTemperature.valid;
//    qDebug() << _airTemperature.TempCelsium << "Celsium";
    RetCode ret = RetCode::RC_ERR;
    QByteArray rxBuf;
    constexpr size_t DELAY_MSEC = 300;
    ret = _ioDev.recieve(rxBuf, DELAY_MSEC);
//    qDebug() << rxBuf.size();
    if (ret == RetCode::RC_SUCCESS)
    {
        QList<QByteArray> splittedRxBuf = rxBuf.split('\n');
        for (int cmdIndex = 0; cmdIndex < splittedRxBuf.size(); ++cmdIndex)
        {
            QByteArray cmdStr = splittedRxBuf.at(cmdIndex);
            QList<QByteArray> splittedCmdStr = cmdStr.split(',');
            QByteArray cmdName = splittedCmdStr.at(0);
            //done
            if (cmdName == "$GPGGA")
            {
                _updateTime(QByteArray(splittedCmdStr[1]));
                _updateGPSfixType(splittedCmdStr[6]);
                if (_location.fixType != Location::Invalid)
                {
                    _updateLatitude(splittedCmdStr[2], splittedCmdStr[3]);
                    _updateLongitude(splittedCmdStr[4], splittedCmdStr[5]);
                    _updateSatellitesInUse(splittedCmdStr[7]);
                    _updateHDOPfactor(splittedCmdStr[8]);
                    _updateAltitude(splittedCmdStr[9]);
                }
                //дифференциальній режим
                if (_location.fixType == Location::DGPSfix)
                {
                    _updateGeoDiffLastUpTime(splittedCmdStr[13]);
                    _updateGeoDiffStationId(splittedCmdStr[14]);
                }
                _updateGeoSeparation(splittedCmdStr[11]);
            }
            //done
            else if (splittedCmdStr.at(0) == "$GPZDA")
            {
                _updateTime(splittedCmdStr[1]);
                _updateDate(splittedCmdStr[2], splittedCmdStr[3], splittedCmdStr[4]);
                if (splittedCmdStr.size() >= 7)
                {
                    _updateLocalTimeDescription(splittedCmdStr[5], splittedCmdStr[6]);
                }
            }
            //done
            else if (splittedCmdStr.at(0) == "$HCHDG")
            {
                _updateCompass(splittedCmdStr[1],
                        splittedCmdStr[2],
                        splittedCmdStr[3],
                        splittedCmdStr[4],
                        splittedCmdStr[5]);
            }
            //done
            else if (splittedCmdStr.at(0) == "$WIMDA")
            {
                _updateBarometer(splittedCmdStr[1], splittedCmdStr[3]);
                _updateAirTempCelsium(splittedCmdStr[5]);
                _updateWaterTempCelsium(splittedCmdStr[7]);
                _updateHumidity(splittedCmdStr[9], splittedCmdStr[10], splittedCmdStr[11]);
                _updateWind(splittedCmdStr[13], splittedCmdStr[15], splittedCmdStr[17], splittedCmdStr[19]);
            }
            else if (splittedCmdStr.at(0) == "$GPVTG")
            {
                _updateNorthTrackSpeed(splittedCmdStr[1],
                        splittedCmdStr[3],
                        splittedCmdStr[5],
                        splittedCmdStr[7]);
                //курс и скорость относительно земли
            }
            else if (splittedCmdStr.at(0) == "$HCHDT")
            {

            }
            else if (splittedCmdStr.at(0) == "$YXXDR")
            {

            }
            else if (splittedCmdStr.at(0) == "$WIMWV")
            {
                if (splittedCmdStr.size() >= 6 &&
                        splittedCmdStr[5] == "A") //data valid
                {
                    _updateWindWithUnit(splittedCmdStr[1],
                            splittedCmdStr[3],
                            splittedCmdStr[5]);
                }
            }
            else
            {
                qDebug() << splittedCmdStr.at(0);
            }
        }
    }
    _checkIsDataOutdated();
    return ret;
}

void Airmar200WXHController::_updateWindWithUnit(const QByteArray& angle,
                         const QByteArray& speed,
                         const QByteArray& speedParam)
{
    _wind.windAngleWithCenterline = angle.toFloat();
    if (speedParam == "K")
    {
        _wind.windSpeedWithUnit = speed.toFloat() * 1000 / 3600;
    }
    else if (speedParam == "M")
    {
        _wind.windSpeedWithUnit = speed.toFloat();
    }
}

void Airmar200WXHController::_updateNorthTrackSpeed(const QByteArray& trackDegreesTrueStr,
                       const QByteArray& trackDegreesMagneticStr,
                       const QByteArray& speedInKnotsStr,
                       const QByteArray& speedInKMetersPerHourStr)
{
    if (trackDegreesTrueStr.isEmpty() == false &&
            trackDegreesMagneticStr.isEmpty() == false &&
            speedInKnotsStr.isEmpty() == false &&
            speedInKMetersPerHourStr.isEmpty() == false)
    {
        _trackSpeed.trackDegreesTrue = trackDegreesTrueStr.toFloat();
        _trackSpeed.trackDegreesMagnetic = trackDegreesMagneticStr.toFloat();
        _trackSpeed.speedInKnots = speedInKnotsStr.toFloat();
        _trackSpeed.speedInMetersPerSecond = speedInKMetersPerHourStr.toFloat() * 1000.0 / 3600.0;
    }
}

void Airmar200WXHController::_updateLocalTimeDescription(const QByteArray& localZoneDescrHoursStr,
                                 const QByteArray& localZoneDescrMinStr)
{
    if (localZoneDescrHoursStr.isEmpty() == false)
    {
        _time.localTimeZoneHours = localZoneDescrHoursStr.toFloat();
    }
    if (localZoneDescrMinStr.isEmpty() == false)
    {
        _time.localTimeZoneMinutes = localZoneDescrMinStr.toFloat();
    }
}

Airmar200WXHController::Timer* t = NULL;

void Airmar200WXHController::_checkIsDataOutdated()
{
    if (_time.elapsedMSecAfterUpdate() >= 1000)
    {
        _time.setValidState(false);
    }
    if (_date.elapsedMSecAfterUpdate() >= 3000)
    {
        _date.setValidState(false);
    }
    if (_location.altitude.timer.elapsed() >= 3000)
    {
        _location.altitude.valid = false;
    }
    if (_location.longitude.timer.elapsed() >= 3000)
    {
        _location.longitude.valid = false;
    }
    if (_location.latitude.timer.elapsed() >= 3000)
    {
        _location.latitude.valid = false;
    }
    _updateLocationStatus();
    if (_compass.timer.elapsed() >= 3000)
    {
        _compass.valid = false;
    }
    if (_barometer.timer.elapsed() >= 3000)
    {
        _barometer.valid = false;
    }
    if (_airTemperature.timer.elapsed() >= 10000)
    {
        _airTemperature.valid = false;
    }
    if (_waterTemperature.timer.elapsed() >= 10000)
    {
        _waterTemperature.valid = false;
    }
    if (_humidity.timer.elapsed() >= 3000)
    {
        _humidity.valid = false;
    }
    if (_wind.elapsedMSecAfterUpdate() >= 1000)
    {
        _wind.setValidState(false);
    }
    if (_trackSpeed.timer.elapsed() >= 3000)
    {
        _trackSpeed.valid = false;
    }
}
