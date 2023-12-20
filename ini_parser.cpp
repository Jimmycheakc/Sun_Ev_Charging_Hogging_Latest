#include "ini_parser.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/IniFileConfiguration.h"

Iniparser* Iniparser::iniparser_ = nullptr;

Iniparser::Iniparser()
{
    parkingLotLocationCode_ = "";
    timerForFilteringSnapShot_ = 0;
}

Iniparser* Iniparser::getInstance()
{
    if (iniparser_ == nullptr)
    {
        iniparser_ = new Iniparser();
    }
    return iniparser_;
}

void Iniparser::FnIniParserInit()
{
    pConf_ = new Poco::Util::IniFileConfiguration("../configuration.ini");

    parkingLotLocationCode_ = pConf_->getString("setting.parking_lot_location_code");
    timerForFilteringSnapShot_ = pConf_->getInt("setting.timer_for_filtering_snapshot");
    cameraIP_ = pConf_->getString("setting.camera_ip");
    centralIP_ = pConf_->getString("setting.central_ip");
    centralServerPort_ = pConf_->getInt("setting.central_server_port");
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