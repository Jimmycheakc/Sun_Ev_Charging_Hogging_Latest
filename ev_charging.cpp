#include <iostream>
#include <string.h>
#include <sstream>
#include <thread>
#include "camera.h"
#include "central.h"
#include "common.h"
#include "event.h"
#include "database.h"
#include "ini_parser.h"
#include "log.h"
#include "timer.h"

int main(int argc, char* agrv[])
{
    std::ostringstream info;
    info << "start " << agrv[0] << " , version: 0.0.1 build:" << __DATE__ << " " << __TIME__;
    AppLogger::getInstance()->FnLog(info.str());

    Iniparser::getInstance()->FnIniParserInit();
    Database::getInstance()->FnDatabaseInit();
    Camera::getInstance()->FnCameraInit();
    Common::getInstance()->FnRetrieveIpAddressFromNetInterface();
    Central::getInstance()->FnSetCentralStatus(Central::getInstance()->FnSendHeartBeatUpdate());
    EventManager::getInstance()->FnStartEventThread();
    EvtTimer::getInstance()->FnStartHeartBeatCentralTimer();
    EvtTimer::getInstance()->FnStartCameraTimerSyncTimer();
    EvtTimer::getInstance()->FnStartDeviceStatusUpdateTimer();
    EvtTimer::getInstance()->FnStartDatabaseReconnectionTimer();
    EvtTimer::getInstance()->FnStartSendOfflineDBParkingLotStatusToCentralTimer();

    if (Central::getInstance()->FnGetCentralStatus() && Database::getInstance()->FnGetDatabaseStatus())
    {
        std::stringstream ss;
        ss << "Central status : " << Central::getInstance()->FnGetCentralStatus() << " , Database status : " << Database::getInstance()->FnGetDatabaseStatus();
        AppLogger::getInstance()->FnLog(ss.str());
        Central::getInstance()->FnSendDeviceStatusUpdate(Iniparser::getInstance()->FnGetParkingLotLocationCode(), Common::getInstance()->FnGetIpAddress(), Central::ERROR_CODE_RECOVERED, Common::getInstance()->FnCurrentFormatDateYYYY_MM_DD_HH_MM_SS());
    }

    if (Central::getInstance()->FnGetCentralStatus() && Camera::getInstance()->FnGetCameraStatus())
    {
        std::stringstream ss;
        ss << "Central status : " << Central::getInstance()->FnGetCentralStatus() << " , Camera status : " << Camera::getInstance()->FnGetCameraStatus();
        AppLogger::getInstance()->FnLog(ss.str());
        Central::getInstance()->FnSendDeviceStatusUpdate(Iniparser::getInstance()->FnGetParkingLotLocationCode(), Iniparser::getInstance()->FnGetCameraIP(), Central::ERROR_CODE_RECOVERED, Common::getInstance()->FnCurrentFormatDateYYYY_MM_DD_HH_MM_SS());
    }

    while(true)
    {
        Camera::getInstance()->FnSubscribeToSnapShot();
    }

    EventManager::getInstance()->FnStopEventThread();

    return 0;
}