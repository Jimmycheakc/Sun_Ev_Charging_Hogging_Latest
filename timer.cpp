#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
#include "camera.h"
#include "central.h"
#include "common.h"
#include "database.h"
#include "ini_parser.h"
#include "Poco/Delegate.h"
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "log.h"
#include "timer.h"

EvtTimer* EvtTimer::evtTimer_ = nullptr;
Poco::Mutex EvtTimer::singletonTimerMutex_;

EvtTimer::EvtTimer()
    : storedFirstParkingLotInfo_{},
    storedSecondParkingLotInfo_{},
    storedThirdParkingLotInfo_{}
{
    filteringInterval = Iniparser::getInstance()->FnGetTimerForFilteringSnapShot() * 1000 * 60;
    isFirstParkingLotFilterTimerRunning_ = false;
    isSecondParkingLotFilterTimerRunning_ = false;
    isThirdParkingLotFilterTimerRunning_ = false;
    isCameraHeartbeatTimerRunning_ = false;
}

EvtTimer* EvtTimer::getInstance()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(singletonTimerMutex_);

    if (evtTimer_ == nullptr)
    {
        evtTimer_ = new EvtTimer();
    }

    return evtTimer_;
}

void EvtTimer::onFirstParkingLotFilterTimerTimeout(Poco::Timer& timer)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", storedFirstParkingLotInfo_);
    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

    storedFirstParkingLotInfo_ = {};
    isFirstParkingLotFilterTimerRunning_ = false;
}

void EvtTimer::FnStartFirstParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

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
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

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
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", storedSecondParkingLotInfo_);
    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

    storedSecondParkingLotInfo_ = {};
    isSecondParkingLotFilterTimerRunning_ = false;
}

void EvtTimer::FnStartSecondParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

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
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

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
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", storedThirdParkingLotInfo_);
    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

    storedThirdParkingLotInfo_ = {};
    isThirdParkingLotFilterTimerRunning_ = false;
}

void EvtTimer::FnStartThirdParkingLotFilterTimer(const Database::parking_lot_t& parkingLotInfo)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

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
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    storedThirdParkingLotInfo_ = {};
    pThirdParkingLotFilterTimer_->stop();
    isThirdParkingLotFilterTimerRunning_ = false;
}

bool EvtTimer::FnIsThirdParkingLotFilterTimerRunning()
{
    return isThirdParkingLotFilterTimerRunning_;
}

void EvtTimer::onCameraTimeSyncTimerTimeout(Poco::Timer& timer)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    Camera::getInstance()->FnSetCurrentTime();
}

void EvtTimer::FnStartCameraTimerSyncTimer()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);
    
    int timer = Iniparser::getInstance()->FnGetTimerForCameraTimeSync() * 1000 * 60;

    std::ostringstream msg;
    msg << __func__ << " , Timer : " << timer;
    AppLogger::getInstance()->FnLog(msg.str());

    pCameraTimeSyncTimer_ = std::make_unique<Poco::Timer>(0, timer);

    // Register Timer Callback
    pCameraTimeSyncTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onCameraTimeSyncTimerTimeout));
}

void EvtTimer::onDeviceStatusUpdateTimerTimeout(Poco::Timer& timer)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    if (!Database::getInstance()->FnGetDatabaseStatus())
    {
        if (!Camera::getInstance()->FnGetCameraStatus())
        {
            AppLogger::getInstance()->FnLog("Database status : Bad, Camera status : Bad, Error code : ERROR_CODE_CAMERA");
            Central::getInstance()->FnSendDeviceStatusUpdate(Iniparser::getInstance()->FnGetParkingLotLocationCode(), Iniparser::getInstance()->FnGetCameraIP(), Central::ERROR_CODE_CAMERA);
            Camera::getInstance()->FnSetCameraRecoveryFlag(true);
        }

        AppLogger::getInstance()->FnLog("Database status : Bad, Error code : ERROR_CODE_IPC");
        Central::getInstance()->FnSendDeviceStatusUpdate(Iniparser::getInstance()->FnGetParkingLotLocationCode(), Common::getInstance()->FnGetIpAddress(), Central::ERROR_CODE_IPC);
        // Set recovery flag to true, and try database reconnect
        Database::getInstance()->FnSetDatabaseRecoveryFlag(true);
        Database::getInstance()->FnDatabaseReconnect();
    }
    else
    {
        if (Database::getInstance()->FnGetDatabaseRecoveryFlag())
        {
            AppLogger::getInstance()->FnLog("Database status : Recover, Error code : ERROR_CODE_RECOVERED");
            Database::getInstance()->FnSetDatabaseRecoveryFlag(false);
            Database::getInstance()->FnInsertStatusRecord("tbl_ev_lot_status", Iniparser::getInstance()->FnGetParkingLotLocationCode(), Common::getInstance()->FnGetIpAddress(), Central::ERROR_CODE_RECOVERED);
            Database::getInstance()->FnSendDBDeviceStatusToCentral("tbl_ev_lot_status");
        }

        if (!Camera::getInstance()->FnGetCameraStatus())
        {
            AppLogger::getInstance()->FnLog("Database status : OK, Camera status : Bad, Error code : ERROR_CODE_CAMERA");
            Database::getInstance()->FnInsertStatusRecord("tbl_ev_lot_status", Iniparser::getInstance()->FnGetParkingLotLocationCode(), Iniparser::getInstance()->FnGetCameraIP(), Central::ERROR_CODE_CAMERA);
            Database::getInstance()->FnSendDBDeviceStatusToCentral("tbl_ev_lot_status");
            Camera::getInstance()->FnSetCameraRecoveryFlag(true);
        }
        else
        {
            if (Camera::getInstance()->FnGetCameraRecoveryFlag())
            {
                AppLogger::getInstance()->FnLog("Camera status : Recover, Error code : ERROR_CODE_RECOVERED");
                Camera::getInstance()->FnSetCameraRecoveryFlag(false);
                Database::getInstance()->FnInsertStatusRecord("tbl_ev_lot_status", Iniparser::getInstance()->FnGetParkingLotLocationCode(), Iniparser::getInstance()->FnGetCameraIP(), Central::ERROR_CODE_RECOVERED);
                Database::getInstance()->FnSendDBDeviceStatusToCentral("tbl_ev_lot_status");
            }
        }
    }
}

void EvtTimer::FnStartDeviceStatusUpdateTimer()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);
    
    int timer = Iniparser::getInstance()->FnGetTimerTimeoutForDeviceStatusUpdateToCentral() * 1000 * 60;

    std::ostringstream msg;
    msg << __func__ << " , Timer : " << timer;
    AppLogger::getInstance()->FnLog(msg.str());

    pDeviceStatusUpdateTimer_ = std::make_unique<Poco::Timer>(0, timer);

    // Register Timer Callback
    pDeviceStatusUpdateTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onDeviceStatusUpdateTimerTimeout));
}

void EvtTimer::onHeartBeatCentralTimerTimeout(Poco::Timer& timer)
{
     // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    Central::getInstance()->FnSetCentralStatus(Central::getInstance()->FnSendHeartBeatUpdate());
}

void EvtTimer::FnStartHeartBeatCentralTimer()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);
    
    int timer = Iniparser::getInstance()->FnGetTimerCentralHeartBeat() * 1000 * 60;

    std::ostringstream msg;
    msg << __func__ << " , Timer : " << timer;
    AppLogger::getInstance()->FnLog(msg.str());

    pHeartBeatCentralTimer_ = std::make_unique<Poco::Timer>(0, timer);

    // Register Timer Callback
    pHeartBeatCentralTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onHeartBeatCentralTimerTimeout));
}

void EvtTimer::onCameraHeartbeatTimerTimeout(Poco::Timer& timer)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    Camera::getInstance()->FnSetCameraStatus(false);
    isCameraHeartbeatTimerRunning_ = false;
}

void EvtTimer::FnStartCameraHeartbeatTimer()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    int heartbeatTimer = Iniparser::getInstance()->FnGetTimerTimeoutForCameraHeartbeat() * 1000 * 60;
    std::ostringstream msg;
    msg << __func__ << " , Timer : " << heartbeatTimer;
    AppLogger::getInstance()->FnLog(msg.str());

    pCameraHeartbeatTimer_ = std::make_unique<Poco::Timer>(heartbeatTimer, 0);

    // Register Timer Callback
    pCameraHeartbeatTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onCameraHeartbeatTimerTimeout));
    isCameraHeartbeatTimerRunning_ = true;
}

void EvtTimer::FnRestartCameraHeartbeatTimer()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    if (FnIsCameraHeartbeatTimerRunning())
    {
        FnStopCameraHeartbeatTimer();
    }
    FnStartCameraHeartbeatTimer();
}

bool EvtTimer::FnIsCameraHeartbeatTimerRunning()
{
    return isCameraHeartbeatTimerRunning_;
}

void EvtTimer::FnStopCameraHeartbeatTimer()
{
    pCameraHeartbeatTimer_->stop();
    isCameraHeartbeatTimerRunning_ = false;
}

void EvtTimer::onDatabaseReconnectionTimerTimeout(Poco::Timer& timer)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    AppLogger::getInstance()->FnLog(__func__);

    Database::getInstance()->FnDatabaseReconnect();
}

void EvtTimer::FnStartDatabaseReconnectionTimer()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(timerMutex_);

    Poco::UInt64 sevenHoursInMillis = 7 * 60 * 60 * 1000;

    std::ostringstream msg;
    msg << __func__ << " , Timer : " << sevenHoursInMillis;
    AppLogger::getInstance()->FnLog(msg.str());

    pDatabaseReconnectionTimer_ = std::make_unique<Poco::Timer>(sevenHoursInMillis, sevenHoursInMillis);

    // Register Timer Callback
    pDatabaseReconnectionTimer_->start(Poco::TimerCallback<EvtTimer>(*this, &EvtTimer::onDatabaseReconnectionTimerTimeout));
}