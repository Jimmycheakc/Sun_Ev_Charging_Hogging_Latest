#pragma once

#include "database.h"
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"

class ParkingEventManager
{

public:
    static ParkingEventManager* getInstance();
    void FnTriggerParkInEvent(const Database::parking_lot_t& parkInInfo);
    void FnTriggerParkOutEvent(const Database::parking_lot_t& parkOutInfo);

    ParkingEventManager(ParkingEventManager& parkingEventManager) = delete;

    void operator=(const ParkingEventManager&) = delete;

private:
    static ParkingEventManager* parkingEventManager_;
    Poco::BasicEvent<const Database::parking_lot_t> parkInEvent_;
    Poco::BasicEvent<const Database::parking_lot_t> parkOutEvent_;
    ParkingEventManager();
    void handleParkInEvent(const Database::parking_lot_t& parkInInfo);
    void handleParkOutEvent(const Database::parking_lot_t& parkOutInfo);
};