#include <iostream>
#include <sstream>
#include "central.h"
#include "event.h"
#include "log.h"
#include "timer.h"
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "Poco/RunnableAdapter.h"

EventManager* EventManager::eventManager_ = nullptr;
Poco::Mutex EventManager::singletonEventMutex_;

EventManager::EventManager()
    : eventQueue_(),
    queueMutex_(),
    eventThread_(),
    stopThread_(false)
{
}

EventManager* EventManager::getInstance()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(singletonEventMutex_);

    if (eventManager_ == nullptr)
    {
        eventManager_ = new EventManager();
    }

    return eventManager_;
}

void EventManager::run()
{
    while(!stopThread_)
    {
        processEventFromQueue();
        Poco::Thread::sleep(100);
    }
}

void EventManager::FnStartEventThread()
{
    eventThread_.start(*this);
}

void EventManager::FnStopEventThread()
{
    stopThread_ = true;
    if (eventThread_.isRunning())
    {
        eventThread_.join();
    }
}

void EventManager::FnEnqueueEvent(BaseEvent* event)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(queueMutex_);
    eventQueue_.push(event);
}

void EventManager::processEventFromQueue()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(queueMutex_);
    while(!eventQueue_.empty())
    {
        BaseEvent* event = eventQueue_.front();
        eventQueue_.pop();

        std::ostringstream msg;
        msg << __func__ << " : " << event->event_name;
        AppLogger::getInstance()->FnLog(msg.str());

        if (event->event_name == "ParkingLotIn_DBEvent")
        {
            ParkingLotIn_DBEvent* parkingLotIn_DBEventData = dynamic_cast<ParkingLotIn_DBEvent*>(event);
            if (parkingLotIn_DBEventData)
            {
                handleParkInEvent(parkingLotIn_DBEventData->park_lot_event_in_data_);
            }
        }
        else if (event->event_name == "ParkingLotOut_DBEvent")
        {
            ParkingLotOut_DBEvent* parkingLotOut_DBEventData = dynamic_cast<ParkingLotOut_DBEvent*>(event);
            if (parkingLotOut_DBEventData)
            {
                handleParkOutEvent(parkingLotOut_DBEventData->park_lot_event_out_data_);
            }
        }

        delete event;
    }
}

void EventManager::handleParkInEvent(const Database::parking_lot_t& parkInInfo)
{
    bool sendFirstLotToCentralAndUpdateStatus = false;
    bool sendSecondLotToCentralAndUpdateStatus = false;
    bool sendThirdLotToCentralAndUpdateStatus = false;

    // Handle start up process case
    if ((Database::getInstance()->FnGetFirstParkingLot().start_up_flag == false) ||
        (Database::getInstance()->FnGetSecondParkingLot().start_up_flag == false) ||
        (Database::getInstance()->FnGetThirdParkingLot().start_up_flag == false)
    )
    {
        if ((parkInInfo.lot_no.compare("1") == 0) &&
            (Database::getInstance()->FnGetFirstParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0))
        {
            std::ostringstream msgFirstLot;
            msgFirstLot << "Handle startup lot 1, different LPN, LPN : " << parkInInfo.lpn;
            AppLogger::getInstance()->FnLog(msgFirstLot.str());

            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
            Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
            Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
            Database::getInstance()->FnSetFirstParkingLotStartUpFlag();
        }
        else if ((parkInInfo.lot_no.compare("2") == 0) &&
            (Database::getInstance()->FnGetSecondParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0))
        {
            std::ostringstream msgSecondLot;
            msgSecondLot << "Handle startup lot 2, different LPN, LPN : " << parkInInfo.lpn;
            AppLogger::getInstance()->FnLog(msgSecondLot.str());

            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
            Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
            Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
            Database::getInstance()->FnSetSecondParkingLotStartUpFlag();
        }
        else if ((parkInInfo.lot_no.compare("3") == 0) &&
            (Database::getInstance()->FnGetThirdParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0))
        {
            std::ostringstream msgThirdLot;
            msgThirdLot << "Handle startup lot 3, different LPN, LPN : " << parkInInfo.lpn;
            AppLogger::getInstance()->FnLog(msgThirdLot.str());

            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
            Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
            Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
            Database::getInstance()->FnSetThirdParkingLotStartUpFlag();
        }

        return;
    }


    // Handle normal case and filtering case
    if ((parkInInfo.lot_no.compare("1") == 0) &&
        (!parkInInfo.lpn.empty()))
    {
        if (EvtTimer::getInstance()->FnIsFirstParkingLotFilterTimerRunning())
        {
            AppLogger::getInstance()->FnLog("Handle normal case, lot 1 -> Stop timer if timer is running");
            EvtTimer::getInstance()->FnStopFirstParkingLotFilterTimer();
        }
        else
        {
            if (Database::getInstance()->FnGetFirstParkingLot().status.compare("release") == 0)
            {
                AppLogger::getInstance()->FnLog("Handle normal case, lot 1 -> if last status == release and no timer running");
                Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                sendFirstLotToCentralAndUpdateStatus = true;
            }
            else if (Database::getInstance()->FnGetFirstParkingLot().status.compare("occupy") == 0)
            {
                if (Database::getInstance()->FnGetFirstParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0)
                {
                    AppLogger::getInstance()->FnLog("Handle normal case, lot 1 -> if last status == occupy and no timer running and lpn different");
                    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                    sendFirstLotToCentralAndUpdateStatus = true;
                }
            }
        }
    }
    else if ((parkInInfo.lot_no.compare("2") == 0) &&
        (!parkInInfo.lpn.empty()))
    {
        if (EvtTimer::getInstance()->FnIsSecondParkingLotFilterTimerRunning())
        {
            AppLogger::getInstance()->FnLog("Handle normal case, lot 2 -> Stop timer if timer is running");
            EvtTimer::getInstance()->FnStopSecondParkingLotFilterTimer();
        }
        else
        {
            if (Database::getInstance()->FnGetSecondParkingLot().status.compare("release") == 0)
            {
                AppLogger::getInstance()->FnLog("Handle normal case, lot 2 -> if last status == release and no timer running");
                Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                sendSecondLotToCentralAndUpdateStatus = true;
            }
            else if (Database::getInstance()->FnGetSecondParkingLot().status.compare("occupy") == 0)
            {
                if (Database::getInstance()->FnGetSecondParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0)
                {
                    AppLogger::getInstance()->FnLog("Handle normal case, lot 2 -> if last status == occupy and no timer running and lpn different");
                    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                    sendSecondLotToCentralAndUpdateStatus = true;
                }
            }
        }
    }
    else if ((parkInInfo.lot_no.compare("3") == 0) &&
        (!parkInInfo.lpn.empty()))
    {
        if (EvtTimer::getInstance()->FnIsThirdParkingLotFilterTimerRunning())
        {
            AppLogger::getInstance()->FnLog("Handle normal case, lot 3 -> Stop timer if timer is running");
            EvtTimer::getInstance()->FnStopThirdParkingLotFilterTimer();
        }
        else
        {
            if (Database::getInstance()->FnGetThirdParkingLot().status.compare("release") == 0)
            {
                AppLogger::getInstance()->FnLog("Handle normal case, lot 3 -> if last status == release and no timer running");
                Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                sendThirdLotToCentralAndUpdateStatus = true;
            }
            else if (Database::getInstance()->FnGetThirdParkingLot().status.compare("occupy") == 0)
            {
                if (Database::getInstance()->FnGetThirdParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0)
                {
                    AppLogger::getInstance()->FnLog("Handle normal case, lot 3 -> if last status == occupy and no timer running and lpn different");
                    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                    sendThirdLotToCentralAndUpdateStatus = true;
                }
            }
        }
    }

    if (sendFirstLotToCentralAndUpdateStatus || sendSecondLotToCentralAndUpdateStatus || sendThirdLotToCentralAndUpdateStatus)
    {
        Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
        Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
    }
}

void EventManager::handleParkOutEvent(const Database::parking_lot_t& parkOutInfo)
{
    Database::parking_lot_t parkOutInfoTemp = parkOutInfo;

    // Handle start up process case
    if ((Database::getInstance()->FnGetFirstParkingLot().start_up_flag == false) ||
        (Database::getInstance()->FnGetSecondParkingLot().start_up_flag == false) ||
        (Database::getInstance()->FnGetThirdParkingLot().start_up_flag == false)
    )
    {
        Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkOutInfo);
        Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
        Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

        if (parkOutInfo.lot_no.compare("1") == 0)
        {
            AppLogger::getInstance()->FnLog("Handle startup case, lot 1");
            Database::getInstance()->FnSetFirstParkingLotStartUpFlag();
        }
        else if (parkOutInfo.lot_no.compare("2") == 0)
        {
            AppLogger::getInstance()->FnLog("Handle startup case, lot 2");
            Database::getInstance()->FnSetSecondParkingLotStartUpFlag();
        }
        else if (parkOutInfo.lot_no.compare("3") == 0)
        {
            AppLogger::getInstance()->FnLog("Handle startup case, lot 3");
            Database::getInstance()->FnSetThirdParkingLotStartUpFlag();
        }

        return;
    }

    // Handle normal case and filtering case

    if (parkOutInfo.lot_no.compare("1") == 0)
    {
        if (Database::getInstance()->FnGetFirstParkingLot().status.compare("occupy") == 0)
        {
            AppLogger::getInstance()->FnLog("Handle normal case, lot 1 -> start timer and last status == occupy");
            parkOutInfoTemp.lpn = Database::getInstance()->FnGetFirstParkingLot().parking_lot_details.lpn;
            EvtTimer::getInstance()->FnStartFirstParkingLotFilterTimer(parkOutInfoTemp);
        }
    }
    else if (parkOutInfo.lot_no.compare("2") == 0)
    {
        if (Database::getInstance()->FnGetSecondParkingLot().status.compare("occupy") == 0)
        {
            AppLogger::getInstance()->FnLog("Handle normal case, lot 2 -> start timer and last status == occupy");
            parkOutInfoTemp.lpn = Database::getInstance()->FnGetSecondParkingLot().parking_lot_details.lpn;
            EvtTimer::getInstance()->FnStartSecondParkingLotFilterTimer(parkOutInfoTemp);
        }
    }
    else if (parkOutInfo.lot_no.compare("3") == 0)
    {
        if (Database::getInstance()->FnGetThirdParkingLot().status.compare("occupy") == 0)
        {
            AppLogger::getInstance()->FnLog("Handle normal case, lot 3 -> start timer and last status == occupy");
            parkOutInfoTemp.lpn = Database::getInstance()->FnGetThirdParkingLot().parking_lot_details.lpn;
            EvtTimer::getInstance()->FnStartThirdParkingLotFilterTimer(parkOutInfoTemp);
        }
    }
}
