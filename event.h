#pragma once

#include <iostream>
#include <queue>
#include "database.h"
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "Poco/Mutex.h"
#include "Poco/Thread.h"

class BaseEvent
{

public:
    virtual ~BaseEvent() = default;
    std::string event_name;
};

class ParkingLotIn_DBEvent : public BaseEvent
{

public:
    Database::parking_lot_t park_lot_event_in_data_;

    ParkingLotIn_DBEvent(const Database::parking_lot_t& data)
        : park_lot_event_in_data_(data)
    {
        event_name = "ParkingLotIn_DBEvent";
    }
};

class ParkingLotOut_DBEvent : public BaseEvent
{

public:
    Database::parking_lot_t park_lot_event_out_data_;

    ParkingLotOut_DBEvent(const Database::parking_lot_t& data)
        : park_lot_event_out_data_(data)
    {
        event_name = "ParkingLotOut_DBEvent";
    }
};

class EventManager : public Poco::Runnable
{

public:
    virtual void run();
    static EventManager* getInstance();

    void FnStartEventThread();
    void FnStopEventThread();
    void FnEnqueueEvent(BaseEvent* event);

    EventManager(EventManager& eventManager) = delete;

    void operator=(const EventManager&) = delete;

private:
    static EventManager* eventManager_;
    static Poco::Mutex singletonEventMutex_;
    std::queue<BaseEvent*> eventQueue_;
    Poco::Mutex queueMutex_;
    Poco::Thread eventThread_;
    bool stopThread_;
    EventManager();
    void processEventFromQueue();
    void handleParkInEvent(const Database::parking_lot_t& parkInInfo);
    void handleParkOutEvent(const Database::parking_lot_t& parkOutInfo);
};