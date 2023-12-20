#include <iostream>
#include "event.h"
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"

ParkingEventManager* ParkingEventManager::parkingEventManager_ = nullptr;

ParkingEventManager::ParkingEventManager()
{
    parkInEvent_ += Poco::delegate(this, &ParkingEventManager::handleParkInEvent);
    parkOutEvent_ += Poco::delegate(this, &ParkingEventManager::handleParkOutEvent);
}

ParkingEventManager* ParkingEventManager::getInstance()
{
    if (parkingEventManager_ == nullptr)
    {
        parkingEventManager_ = new ParkingEventManager();
    }

    return parkingEventManager_;
}

void ParkingEventManager::FnTriggerParkInEvent(const Database::parking_lot_t& parkInInfo)
{
    parkInEvent_.notifyAsync(this, parkInInfo);
}

void ParkingEventManager::FnTriggerParkOutEvent(const Database::parking_lot_t& parkOutInfo)
{
    parkOutEvent_.notifyAsync(this, parkOutInfo);
}

void ParkingEventManager::handleParkInEvent(const Database::parking_lot_t& parkInInfo)
{
    std::cout << "parkInInfo.location_code : " << parkInInfo.location_code << std::endl;
    std::cout << "parkInInfo.lpn : " << parkInInfo.lpn << std::endl;

    if ((parkInInfo.lot_no.compare("1") == 0) &&
        (!parkInInfo.lpn.empty()))
    {
        if (Database::getInstance()->FnGetFirstParkingLot().parking_lot_details.lpn.empty())
        {
            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
        }
        else
        {
            if (Database::getInstance()->FnGetFirstParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0)
            {
                Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
            }
        }
    }

    if ((parkInInfo.lot_no.compare("2") == 0) &&
        (!parkInInfo.lpn.empty()))
    {
        if (Database::getInstance()->FnGetSecondParkingLot().parking_lot_details.lpn.empty())
        {
            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
        }
        else
        {
            if (Database::getInstance()->FnGetSecondParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0)
            {
                Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
            }
        }
    }

    if ((parkInInfo.lot_no.compare("3") == 0) &&
        (!parkInInfo.lpn.empty()))
    {
        if (Database::getInstance()->FnGetThirdParkingLot().parking_lot_details.lpn.empty())
        {
            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
        }
        else
        {
            if (Database::getInstance()->FnGetThirdParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0)
            {
                Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
            }
        }
    }

    Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
}

void ParkingEventManager::handleParkOutEvent(const Database::parking_lot_t& parkOutInfo)
{
    std::cout << "parkOutInfo.location_code : " << parkOutInfo.location_code << std::endl;
    std::cout << "parkOutInfo.lpn : " << parkOutInfo.lpn << std::endl;
    Database::parking_lot_t parkOutInfoTemp = parkOutInfo;

    if (parkOutInfo.lot_no.compare("1") == 0)
    {
        if (Database::getInstance()->FnGetFirstParkingLot().status.compare("occupy") == 0)
        {
            parkOutInfoTemp.lpn = Database::getInstance()->FnGetFirstParkingLot().parking_lot_details.lpn;
            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkOutInfo);
        }
    }

    if (parkOutInfo.lot_no.compare("2") == 0)
    {
        if (Database::getInstance()->FnGetSecondParkingLot().status.compare("occupy") == 0)
        {
            parkOutInfoTemp.lpn = Database::getInstance()->FnGetSecondParkingLot().parking_lot_details.lpn;
            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkOutInfo);
        }
    }

    if (parkOutInfo.lot_no.compare("3") == 0)
    {
        if (Database::getInstance()->FnGetThirdParkingLot().status.compare("occupy") == 0)
        {
            parkOutInfoTemp.lpn = Database::getInstance()->FnGetThirdParkingLot().parking_lot_details.lpn;
            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkOutInfo);
        }
    }
}