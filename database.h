#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include "Poco/Data/Session.h"
#include "Poco/Mutex.h"

class Database{

public:
    typedef struct parking_lot
    {
        std::string location_code;
        std::string lot_no;
        std::string lpn;
        std::string lot_in_image_path;
        std::string lot_out_image_path;
        std::string lot_in_dt;
        std::string lot_out_dt;
        std::string add_dt;
        std::string update_dt;
        std::string lot_in_central_sent_dt;
        std::string lot_out_central_sent_dt;
    } parking_lot_t;

    typedef struct parking_lot_info
    {
        bool start_up_flag;
        std::string status;
        parking_lot_t parking_lot_details;
    } parking_lot_info_t;

    static Database* getInstance();
    void FnDatabaseInit();
    void FnInsertRecord(const std::string& tableName, parking_lot_t lot);
    void FnSelectAllRecord(const std::string& tableName, std::vector<parking_lot_t>& v_lot);
    bool FnIsTableEmpty(const std::string& tableName);
    void FnRemoveAllRecord(const std::string& tableName);

    void FnUpdateThreeLotParkingStatus(const std::string& tableName);
    const parking_lot_info_t& FnGetFirstParkingLot() const;
    const parking_lot_info_t& FnGetSecondParkingLot() const;
    const parking_lot_info_t& FnGetThirdParkingLot() const;

    void FnSendDBParkingLotStatusToCentral(const std::string& tableName);

    Database(Database& database) = delete;

    void operator=(const Database&) = delete;

private:
    static Database* database_;
    std::unique_ptr<Poco::Data::Session> session_;
    Poco::Mutex databaseMutex_;
    parking_lot_info_t firstParkingLot_;
    parking_lot_info_t secondParkingLot_;
    parking_lot_info_t thirdParkingLot_;
    Database();
    ~Database();
};