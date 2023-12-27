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

#include <iostream>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/Net/IPAddress.h>

int main(int argc, char* agrv[])
{
    std::ostringstream info;
    info << "start " << agrv[0] << " , version: 0.0.1 build:" << __DATE__ << " " << __TIME__;
    AppLogger::getInstance()->FnLog(info.str());

    Iniparser::getInstance()->FnIniParserInit();
    Database::getInstance()->FnDatabaseInit();
    Common::getInstance()->FnRetrieveIpAddressFromNetInterface();
    EventManager::getInstance()->FnStartEventThread();
    EvtTimer::getInstance()->FnStartHeartBeatCentralTimer();
    EvtTimer::getInstance()->FnStartCameraTimerSyncTimer();
    EvtTimer::getInstance()->FnStartDeviceStatusUpdateTimer();
    Camera::getInstance()->FnSubscribeToSnapShot();

    EventManager::getInstance()->FnStopEventThread();

    return 0;
}