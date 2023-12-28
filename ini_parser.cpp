#include "ini_parser.h"
#include "log.h"
#include "Poco/AutoPtr.h"
#include "Poco/Exception.h"
#include "Poco/Util/IniFileConfiguration.h"

Iniparser* Iniparser::iniparser_ = nullptr;
Poco::Mutex Iniparser::singletonIniParserMutex_;

Iniparser::Iniparser()
{
    parkingLotLocationCode_ = "";
    timerForFilteringSnapShot_ = 0;
}

Iniparser* Iniparser::getInstance()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(singletonIniParserMutex_);

    if (iniparser_ == nullptr)
    {
        iniparser_ = new Iniparser();
    }
    return iniparser_;
}

void Iniparser::FnIniParserInit()
{
    AppLogger::getInstance()->FnLog("Ini parser initialization.");

    try
    {
        pConf_ = new Poco::Util::IniFileConfiguration("../configuration.ini");

        parkingLotLocationCode_ = pConf_->getString("setting.parking_lot_location_code");
        timerForFilteringSnapShot_ = pConf_->getInt("setting.timer_for_filtering_snapshot");
        cameraIP_ = pConf_->getString("setting.camera_ip");
        centralIP_ = pConf_->getString("setting.central_ip");
        centralServerPort_ = pConf_->getInt("setting.central_server_port");
        timerTimeoutForCameraHeartbeat_ = pConf_->getInt("setting.timer_timeout_for_camera_heartbeat");
        timerForCameraTimeSync_ = pConf_->getInt("setting.timer_for_camera_time_sync");
        timerTimeoutForDeviceStatusUpdateToCentral_ = pConf_->getInt("setting.timer_timeout_for_device_status_update_to_central");
        timerCentralHeartBeat_ = pConf_->getInt("setting.timer_central_heartbeat");
    }
    catch (Poco::Exception& ex)
    {
        AppLogger::getInstance()->FnLog(ex.displayText());
    }
}

std::string Iniparser::FnGetParkingLotLocationCode()
{
    return parkingLotLocationCode_;
}

int Iniparser::FnGetTimerForFilteringSnapShot()
{
    return timerForFilteringSnapShot_;
}

std::string Iniparser::FnGetCameraIP()
{
    return cameraIP_;
}

std::string Iniparser::FnGetCentralIP()
{
    return centralIP_;
}

int Iniparser::FnGetCentralServerPort()
{
    return centralServerPort_;
}

int Iniparser::FnGetTimerTimeoutForCameraHeartbeat()
{
    return timerTimeoutForCameraHeartbeat_;
}

int Iniparser::FnGetTimerForCameraTimeSync()
{
    return timerForCameraTimeSync_;
}

int Iniparser::FnGetTimerTimeoutForDeviceStatusUpdateToCentral()
{
    return timerTimeoutForDeviceStatusUpdateToCentral_;
}

int Iniparser::FnGetTimerCentralHeartBeat()
{
    return timerCentralHeartBeat_;
}