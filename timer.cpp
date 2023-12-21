#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
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
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", storedFirstParkingLotInfo_);
    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

    FnStopFirstParkingLotFilterTimer();
}

void EvtTimer::FnStartFirstParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo)
{
    pFirstParkingLotFilterTimer_ = std::make_unique<Poco::Timer>(filteringInterval, 0);

    // Register Timer Callback
    storedFirstParkingLotInfo_ = parkingLotInfo;
    pFirstParkingLotFilterTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onFirstParkingLotFilterTimerTimeout));
    isFirstParkingLotFilterTimerRunning_ = true;
}

void EvtTimer::FnStopFirstParkingLotFilterTimer()
{
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
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", storedSecondParkingLotInfo_);
    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

    FnStopSecondParkingLotFilterTimer();
}

void EvtTimer::FnStartSecondParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo)
{
    pSecondParkingLotFilterTimer_ = std::make_unique<Poco::Timer>(filteringInterval, 0);

    // Register Timer Callback
    storedSecondParkingLotInfo_ = parkingLotInfo;
    pSecondParkingLotFilterTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onSecondParkingLotFilterTimerTimeout));
    isSecondParkingLotFilterTimerRunning_ = true;
}

void EvtTimer::FnStopSecondParkingLotFilterTimer()
{
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
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", storedThirdParkingLotInfo_);
    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

    FnStopThirdParkingLotFilterTimer();
}

void EvtTimer::FnStartThirdParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo)
{
    pThirdParkingLotFilterTimer_ = std::make_unique<Poco::Timer>(filteringInterval, 0);

    // Register Timer Callback
    storedThirdParkingLotInfo_ = parkingLotInfo;
    pThirdParkingLotFilterTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onThirdParkingLotFilterTimerTimeout));
    isThirdParkingLotFilterTimerRunning_ = true;
}

void EvtTimer::FnStopThirdParkingLotFilterTimer()
{
    storedThirdParkingLotInfo_ = {};
    pThirdParkingLotFilterTimer_->stop();
    isThirdParkingLotFilterTimerRunning_ = false;
}

bool EvtTimer::FnIsThirdParkingLotFilterTimerRunning()
{
    return isThirdParkingLotFilterTimerRunning_;
}