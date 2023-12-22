#pragma once

#include <iostream>
#include <memory>
#include "database.h"
#include "Poco/Timer.h"
#include "Poco/Thread.h"

class EvtTimer
{

public:
    static EvtTimer* getInstance();
    void FnStartFirstParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo);
    void FnStopFirstParkingLotFilterTimer();
    bool FnIsFirstParkingLotFilterTimerRunning();
    void FnStartSecondParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo);
    void FnStopSecondParkingLotFilterTimer();
    bool FnIsSecondParkingLotFilterTimerRunning();
    void FnStartThirdParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo);
    void FnStopThirdParkingLotFilterTimer();
    bool FnIsThirdParkingLotFilterTimerRunning();

    void FnStartCameraHeartbeatSendToCentralTimer();
    void FnRestartCameraHeartbeatSendToCentralTimer();
    void FnStopCameraHeartbeatSendToCentralTimer();
    bool FnIsCameraHeartbeatSendToCentralTimerRunning();

    EvtTimer(EvtTimer& evtTimer) = delete;

    void operator=(const EvtTimer&) = delete;

private:
    static EvtTimer* evtTimer_;
    int filteringInterval;
    std::unique_ptr<Poco::Timer> pFirstParkingLotFilterTimer_;
    Database::parking_lot_t storedFirstParkingLotInfo_;
    bool isFirstParkingLotFilterTimerRunning_;
    std::unique_ptr<Poco::Timer> pSecondParkingLotFilterTimer_;
    Database::parking_lot_t storedSecondParkingLotInfo_;
    bool isSecondParkingLotFilterTimerRunning_;
    std::unique_ptr<Poco::Timer> pThirdParkingLotFilterTimer_;
    Database::parking_lot_t storedThirdParkingLotInfo_;
    bool isThirdParkingLotFilterTimerRunning_;
    std::unique_ptr<Poco::Timer> pCameraHeartbeatSendToCentralTimer_;
    bool isCameraHeartbeatSendToCentralTimerRunning_;
    EvtTimer();
    void onFirstParkingLotFilterTimerTimeout(Poco::Timer& timer);
    void onSecondParkingLotFilterTimerTimeout(Poco::Timer& timer);
    void onThirdParkingLotFilterTimerTimeout(Poco::Timer& timer);
    void onCameraHeartbeatSendToCentralTimerTimeout(Poco::Timer& timer);
};