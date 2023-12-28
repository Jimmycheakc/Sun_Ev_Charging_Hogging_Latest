#pragma once

#include <iostream>
#include <string>
#include "Poco/AutoPtr.h"
#include "Poco/Mutex.h"
#include "Poco/Util/IniFileConfiguration.h"

class Iniparser{

public:
    static Iniparser* getInstance();
    void FnIniParserInit();
    std::string FnGetParkingLotLocationCode();
    int FnGetTimerForFilteringSnapShot();
    std::string FnGetCameraIP();
    std::string FnGetCentralIP();
    int FnGetCentralServerPort();
    int FnGetTimerTimeoutForCameraHeartbeat();
    int FnGetTimerForCameraTimeSync();
    int FnGetTimerTimeoutForDeviceStatusUpdateToCentral();
    int FnGetTimerCentralHeartBeat();

    Iniparser(Iniparser& iniparser) = delete;

    void operator=(const Iniparser&) = delete;

private:
    static Iniparser* iniparser_;
    static Poco::Mutex singletonIniParserMutex_;
    Poco::AutoPtr<Poco::Util::IniFileConfiguration> pConf_;
    std::string parkingLotLocationCode_;
    int timerForFilteringSnapShot_;
    std::string cameraIP_;
    std::string centralIP_;
    int centralServerPort_;
    int timerTimeoutForCameraHeartbeat_;
    int timerForCameraTimeSync_;
    int timerTimeoutForDeviceStatusUpdateToCentral_;
    int timerCentralHeartBeat_;
    Iniparser();
};