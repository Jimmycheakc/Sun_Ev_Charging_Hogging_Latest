#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
#include "central.h"
#include "database.h"
#include "ini_parser.h"
#include "Poco/Delegate.h"
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "log.h"
#include "timer.h"

EvtTimer* EvtTimer::evtTimer_ = nullptr;

EvtTimer::EvtTimer()
    : storedFirstParkingLotInfo_{},
    storedSecondParkingLotInfo_{},
    storedThirdParkingLotInfo_{}
{
    filteringInterval = Iniparser::getInstance()->FnGetTimerForFilteringSnapShot() * 1000 * 60;
    isFirstParkingLotFilterTimerRunning_ = false;
    isSecondParkingLotFilterTimerRunning_ = false;
    isThirdParkingLotFilterTimerRunning_ = false;
    isCameraHeartbeatSendToCentralTimerRunning_ = false;
}

EvtTimer* EvtTimer::getInstance()
{
    if (evtTimer_ == nullptr)
    {
        evtTimer_ = new EvtTimer();
    }

    return evtTimer_;
}

void EvtTimer::onFirstParkingLotFilterTimerTimeout(Poco::Timer& timer)
{
    AppLogger::getInstance()->FnLog(__func__);

    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", storedFirstParkingLotInfo_);
    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

    storedFirstParkingLotInfo_ = {};
    isFirstParkingLotFilterTimerRunning_ = false;
}

void EvtTimer::FnStartFirstParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo)
{
    std::ostringstream msg;
    msg << __func__ << " , Timer : " << filteringInterval;
    AppLogger::getInstance()->FnLog(msg.str());

    pFirstParkingLotFilterTimer_ = std::make_unique<Poco::Timer>(filteringInterval, 0);

    // Register Timer Callback
    storedFirstParkingLotInfo_ = parkingLotInfo;
    pFirstParkingLotFilterTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onFirstParkingLotFilterTimerTimeout));
    isFirstParkingLotFilterTimerRunning_ = true;
}

void EvtTimer::FnStopFirstParkingLotFilterTimer()
{
    AppLogger::getInstance()->FnLog(__func__);

    storedFirstParkingLotInfo_ = {};
    pFirstParkingLotFilterTimer_->stop();
    isFirstParkingLotFilterTimerRunning_ = false;
}

bool EvtTimer::FnIsFirstParkingLotFilterTimerRunning()
{
    return isFirstParkingLotFilterTimerRunning_;
}

void EvtTimer::onSecondParkingLotFilterTimerTimeout(Poco::Timer& timer)
{
    AppLogger::getInstance()->FnLog(__func__);

    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", storedSecondParkingLotInfo_);
    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

    storedSecondParkingLotInfo_ = {};
    isSecondParkingLotFilterTimerRunning_ = false;
}

void EvtTimer::FnStartSecondParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo)
{
    std::ostringstream msg;
    msg << __func__ << " , Timer : " << filteringInterval;
    AppLogger::getInstance()->FnLog(msg.str());

    pSecondParkingLotFilterTimer_ = std::make_unique<Poco::Timer>(filteringInterval, 0);

    // Register Timer Callback
    storedSecondParkingLotInfo_ = parkingLotInfo;
    pSecondParkingLotFilterTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onSecondParkingLotFilterTimerTimeout));
    isSecondParkingLotFilterTimerRunning_ = true;
}

void EvtTimer::FnStopSecondParkingLotFilterTimer()
{
    AppLogger::getInstance()->FnLog(__func__);

    storedSecondParkingLotInfo_ = {};
    pSecondParkingLotFilterTimer_->stop();
    isSecondParkingLotFilterTimerRunning_ = false;
}

bool EvtTimer::FnIsSecondParkingLotFilterTimerRunning()
{
    return isSecondParkingLotFilterTimerRunning_;
}

void EvtTimer::onThirdParkingLotFilterTimerTimeout(Poco::Timer& timer)
{
    AppLogger::getInstance()->FnLog(__func__);

    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", storedThirdParkingLotInfo_);
    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

    storedThirdParkingLotInfo_ = {};
    isThirdParkingLotFilterTimerRunning_ = false;
}

void EvtTimer::FnStartThirdParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo)
{
    std::ostringstream msg;
    msg << __func__ << " , Timer : " << filteringInterval;
    AppLogger::getInstance()->FnLog(msg.str());

    pThirdParkingLotFilterTimer_ = std::make_unique<Poco::Timer>(filteringInterval, 0);

    // Register Timer Callback
    storedThirdParkingLotInfo_ = parkingLotInfo;
    pThirdParkingLotFilterTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onThirdParkingLotFilterTimerTimeout));
    isThirdParkingLotFilterTimerRunning_ = true;
}

void EvtTimer::FnStopThirdParkingLotFilterTimer()
{
    AppLogger::getInstance()->FnLog(__func__);

    storedThirdParkingLotInfo_ = {};
    pThirdParkingLotFilterTimer_->stop();
    isThirdParkingLotFilterTimerRunning_ = false;
}

bool EvtTimer::FnIsThirdParkingLotFilterTimerRunning()
{
    return isThirdParkingLotFilterTimerRunning_;
}

void EvtTimer::onCameraHeartbeatSendToCentralTimerTimeout(Poco::Timer& timer)
{
    AppLogger::getInstance()->FnLog(__func__);

    Central::getInstance()->FnSendDeviceStatusUpdate(Iniparser::getInstance()->FnGetCameraIP(), Central::ERROR_CODE_CAMERA);
    isCameraHeartbeatSendToCentralTimerRunning_ = false;
}

void EvtTimer::FnStartCameraHeartbeatSendToCentralTimer()
{
    int heartbeatTimer = Iniparser::getInstance()->FnGetTimerTimeoutForCameraHeartbeatSendToCentral() * 1000 * 60;
    std::ostringstream msg;
    msg << __func__ << " , Timer : " << heartbeatTimer;
    AppLogger::getInstance()->FnLog(msg.str());
    
    pCameraHeartbeatSendToCentralTimer_ = std::make_unique<Poco::Timer>(heartbeatTimer, 0);

    // Register Timer Callback
    pCameraHeartbeatSendToCentralTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onCameraHeartbeatSendToCentralTimerTimeout));
    isCameraHeartbeatSendToCentralTimerRunning_ = true;
}

void EvtTimer::FnRestartCameraHeartbeatSendToCentralTimer()
{
    if (FnIsCameraHeartbeatSendToCentralTimerRunning())
    {
        FnStopCameraHeartbeatSendToCentralTimer();
    }
    FnStartCameraHeartbeatSendToCentralTimer();
}

void EvtTimer::FnStopCameraHeartbeatSendToCentralTimer()
{
    AppLogger::getInstance()->FnLog(__func__);

    pCameraHeartbeatSendToCentralTimer_->stop();
    isCameraHeartbeatSendToCentralTimerRunning_ = false;
}

bool EvtTimer::FnIsCameraHeartbeatSendToCentralTimerRunning()
{
    return isCameraHeartbeatSendToCentralTimerRunning_;
}