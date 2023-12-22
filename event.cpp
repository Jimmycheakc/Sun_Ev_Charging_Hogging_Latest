#include <iostream>
#include "event.h"
#include "timer.h"
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
    parkInEvent_.notify(this, parkInInfo);
}

void ParkingEventManager::FnTriggerParkOutEvent(const Database::parking_lot_t& parkOutInfo)
{
    parkOutEvent_.notify(this, parkOutInfo);
}

void ParkingEventManager::handleParkInEvent(const Database::parking_lot_t& parkInInfo)
{
    std::cout << __func__ << "parkInInfo.location_code : " << parkInInfo.location_code << std::endl;
    std::cout << __func__ << "parkInInfo.lpn : " << parkInInfo.lpn << std::endl;
    bool sendFirstLotToCentralAndUpdateStatus = false;
    bool sendSecondLotToCentralAndUpdateStatus = false;
    bool sendThirdLotToCentralAndUpdateStatus = false;

    // Handle start up process case
    if ((Database::getInstance()->FnGetFirstParkingLot().start_up_flag == false) ||
        (Database::getInstance()->FnGetSecondParkingLot().start_up_flag == false) ||
        (Database::getInstance()->FnGetThirdParkingLot().start_up_flag == false)
    )
    {
        std::cout << __func__ << "Handle startup case" << std::endl;
        if ((parkInInfo.lot_no.compare("1") == 0) &&
            (Database::getInstance()->FnGetFirstParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0))
        {
            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
            Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
            Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
            Database::getInstance()->FnSetFirstParkingLotStartUpFlag();
            std::cout << __func__ << "Handle startup case 1" << std::endl;
        }
        else if ((parkInInfo.lot_no.compare("2") == 0) &&
            (Database::getInstance()->FnGetSecondParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0))
        {
            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
            Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
            Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
            Database::getInstance()->FnSetSecondParkingLotStartUpFlag();
            std::cout << __func__ << "Handle startup case 2" << std::endl;
        }
        else if ((parkInInfo.lot_no.compare("3") == 0) &&
            (Database::getInstance()->FnGetThirdParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0))
        {
            Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
            Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
            Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
            Database::getInstance()->FnSetThirdParkingLotStartUpFlag();
            std::cout << __func__ << "Handle startup case 3" << std::endl;
        }

        return;
    }


    // Handle normal case and filtering case
    if ((parkInInfo.lot_no.compare("1") == 0) &&
        (!parkInInfo.lpn.empty()))
    {
        if (EvtTimer::getInstance()->FnIsFirstParkingLotFilterTimerRunning())
        {
            std::cout << __func__ << "Handle normal case 1 --> timer running and stop timer" << std::endl;
            EvtTimer::getInstance()->FnStopFirstParkingLotFilterTimer();
        }
        else
        {
            if (Database::getInstance()->FnGetFirstParkingLot().status.compare("release") == 0)
            {
                std::cout << __func__ << "Handle normal case 1 --> no timer and last status == release" << std::endl;
                Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                sendFirstLotToCentralAndUpdateStatus = true;
            }
            else if (Database::getInstance()->FnGetFirstParkingLot().status.compare("occupy") == 0)
            {
                if (Database::getInstance()->FnGetFirstParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0)
                {
                    std::cout << __func__ << "Handle normal case 1 --> no timer and last status == occupy and lpn different" << std::endl;
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
            std::cout << __func__ << "Handle normal case 2 --> timer running and stop timer" << std::endl;
            EvtTimer::getInstance()->FnStopSecondParkingLotFilterTimer();
        }
        else
        {
            if (Database::getInstance()->FnGetSecondParkingLot().status.compare("release") == 0)
            {
                std::cout << __func__ << "Handle normal case 2 --> no timer and last status == release" << std::endl;
                Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                sendSecondLotToCentralAndUpdateStatus = true;
            }
            else if (Database::getInstance()->FnGetSecondParkingLot().status.compare("occupy") == 0)
            {
                if (Database::getInstance()->FnGetSecondParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0)
                {
                    std::cout << __func__ << "Handle normal case 2 --> no timer and last status == occupy and lpn different" << std::endl;
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
            std::cout << __func__ << "Handle normal case 3 --> timer running and stop timer" << std::endl;
            EvtTimer::getInstance()->FnStopThirdParkingLotFilterTimer();
        }
        else
        {
            if (Database::getInstance()->FnGetThirdParkingLot().status.compare("release") == 0)
            {
                std::cout << __func__ << "Handle normal case 3 --> no timer and last status == release" << std::endl;
                Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                sendThirdLotToCentralAndUpdateStatus = true;
            }
            else if (Database::getInstance()->FnGetThirdParkingLot().status.compare("occupy") == 0)
            {
                if (Database::getInstance()->FnGetThirdParkingLot().parking_lot_details.lpn.compare(parkInInfo.lpn) != 0)
                {
                    std::cout << __func__ << "Handle normal case 3 --> no timer and last status == occupy and lpn different" << std::endl;
                    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkInInfo);
                    sendThirdLotToCentralAndUpdateStatus = true;
                }
            }
        }
    }

    if (sendFirstLotToCentralAndUpdateStatus || sendSecondLotToCentralAndUpdateStatus || sendThirdLotToCentralAndUpdateStatus)
    {
        std::cout << __func__ << "Send to central and update lot status : " << "1:" << sendFirstLotToCentralAndUpdateStatus << " 2:"<< sendSecondLotToCentralAndUpdateStatus << " 3:" << sendThirdLotToCentralAndUpdateStatus << std::endl;
        Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
        Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
    }
}

void ParkingEventManager::handleParkOutEvent(const Database::parking_lot_t& parkOutInfo)
{
    std::cout << __func__ << "parkOutInfo.location_code : " << parkOutInfo.location_code << std::endl;
    std::cout << __func__ << "parkOutInfo.lpn : " << parkOutInfo.lpn << std::endl;
    Database::parking_lot_t parkOutInfoTemp = parkOutInfo;

    // Handle start up process case
    if ((Database::getInstance()->FnGetFirstParkingLot().start_up_flag == false) ||
        (Database::getInstance()->FnGetSecondParkingLot().start_up_flag == false) ||
        (Database::getInstance()->FnGetThirdParkingLot().start_up_flag == false)
    )
    {
        std::cout << __func__ << "Handle startup case" << std::endl;
        Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", parkOutInfo);
        Database::getInstance()->FnSendDBParkingLotStatusToCentral("tbl_ev_lot_trans");
        Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");

        if (parkOutInfo.lot_no.compare("1") == 0)
        {
            std::cout << __func__ << "Handle startup case 1" << std::endl;
            Database::getInstance()->FnSetFirstParkingLotStartUpFlag();
        }
        else if (parkOutInfo.lot_no.compare("2") == 0)
        {
            std::cout << __func__ << "Handle startup case 2" << std::endl;
            Database::getInstance()->FnSetFirstParkingLotStartUpFlag();
        }
        else if (parkOutInfo.lot_no.compare("3") == 0)
        {
            std::cout << __func__ << "Handle startup case 3" << std::endl;
            Database::getInstance()->FnSetFirstParkingLotStartUpFlag();
        }

        return;
    }

    // Handle normal case and filtering case

    if (parkOutInfo.lot_no.compare("1") == 0)
    {
        if (Database::getInstance()->FnGetFirstParkingLot().status.compare("occupy") == 0)
        {
            std::cout << __func__ << "Handle normal case 1 --> start timer and last status == occupy" << std::endl;
            parkOutInfoTemp.lpn = Database::getInstance()->FnGetFirstParkingLot().parking_lot_details.lpn;
            EvtTimer::getInstance()->FnStartFirstParkingLotFilterTimer(parkOutInfoTemp);
        }
    }
    else if (parkOutInfo.lot_no.compare("2") == 0)
    {
        if (Database::getInstance()->FnGetSecondParkingLot().status.compare("occupy") == 0)
        {
            std::cout << __func__ << "Handle normal case 2 --> start timer and last status == occupy" << std::endl;
            parkOutInfoTemp.lpn = Database::getInstance()->FnGetSecondParkingLot().parking_lot_details.lpn;
            EvtTimer::getInstance()->FnStartSecondParkingLotFilterTimer(parkOutInfoTemp);
        }
    }
    else if (parkOutInfo.lot_no.compare("3") == 0)
    {
        if (Database::getInstance()->FnGetThirdParkingLot().status.compare("occupy") == 0)
        {
            std::cout << __func__ << "Handle normal case 3 --> start timer and last status == occupy" << std::endl;
            parkOutInfoTemp.lpn = Database::getInstance()->FnGetThirdParkingLot().parking_lot_details.lpn;
            EvtTimer::getInstance()->FnStartThirdParkingLotFilterTimer(parkOutInfoTemp);
        }
    }
}