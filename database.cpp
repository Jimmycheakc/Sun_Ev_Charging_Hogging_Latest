#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "central.h"
#include "common.h"
#include "database.h"
#include "log.h"
#include "Poco/Base64Encoder.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Mutex.h"

Database* Database::database_ = nullptr;
Poco::Mutex Database::singletonDatabaseMutex_;

Database::Database()
    : firstParkingLot_{},
    secondParkingLot_{},
    thirdParkingLot_{}
{
    firstParkingLot_.start_up_flag = false;
    secondParkingLot_.start_up_flag = false;
    thirdParkingLot_.start_up_flag = false;
    dbRecoveryFlag_ = false;
}

Database::~Database()
{
    Poco::Data::MySQL::Connector::unregisterConnector();
}

Database* Database::getInstance()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(singletonDatabaseMutex_);

    if (database_ == nullptr)
    {
        database_ = new Database();
    }
    return database_;
}

void Database::FnDatabaseInit()
{
    try
    {
        AppLogger::getInstance()->FnLog("Database initialization.");
        // Initialize MySQL connector
        Poco::Data::MySQL::Connector::registerConnector();

        // Establish a session
        session_ = std::make_unique<Poco::Data::Session>("MySQL", "host=localhost;user=root;password=yzxh2007;db=ev_charging_hogging_database");

        // Temp: Need to remove
        //Database::getInstance()->FnRemoveAllRecord("tbl_ev_lot_trans");
        //Database::getInstance()->FnRemoveAllRecord("tbl_ev_lot_status");
        if (!FnIsTableEmpty("tbl_ev_lot_trans"))
        {
            AppLogger::getInstance()->FnLog("'tbl_ev_lot_trans' database not empty, update 3 parking lots.");
            FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
        }

        if (!FnIsTableEmpty("tbl_ev_lot_status"))
        {
            AppLogger::getInstance()->FnLog("'tbl_ev_lot_status' database not empty, trying send status to central.");
            FnSendDBDeviceStatusToCentral("tbl_ev_lot_status");
        }
    }
    catch (const Poco::Exception& ex)
    {
        std::ostringstream msg;
        msg << "POCO Exception: " << ex.displayText() << std::endl;
        std::cerr << msg.str();
        AppLogger::getInstance()->FnLog(msg.str());
    }
}

bool Database::FnGetDatabaseStatus()
{
    return session_->isGood();
}

void Database::FnDatabaseReconnect()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    session_->reconnect();
}

void Database::FnSetDatabaseRecoveryFlag(bool flag)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    dbRecoveryFlag_ = flag;
}

bool Database::FnGetDatabaseRecoveryFlag()
{
    return dbRecoveryFlag_;
}

void Database::FnInsertRecord(const std::string& tableName, parking_lot_t lot)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    if (!session_->isConnected())
    {
        session_->reconnect();
    }

    try
    {
        std::string query = "INSERT INTO " + tableName + " (location_code, lot_no, lpn, lot_in_image, lot_out_image, lot_in_dt, lot_out_dt, add_dt, update_dt, lot_in_central_sent_dt, lot_out_central_sent_dt) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        Poco::Data::Statement insert(*session_);

        Poco::Nullable<std::string> location_code(lot.location_code.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.location_code));
        Poco::Nullable<std::string> lot_no(lot.lot_no.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.lot_no));
        Poco::Nullable<std::string> lpn(lot.lpn.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.lpn));
        Poco::Nullable<std::string> lot_in_image_path(lot.lot_in_image_path.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.lot_in_image_path));
        Poco::Nullable<std::string> lot_out_image_path(lot.lot_out_image_path.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.lot_out_image_path));
        Poco::Nullable<std::string> lot_in_dt(lot.lot_in_dt.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.lot_in_dt));
        Poco::Nullable<std::string> lot_out_dt(lot.lot_out_dt.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.lot_out_dt));
        Poco::Nullable<std::string> add_dt(lot.add_dt.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.add_dt));
        Poco::Nullable<std::string> update_dt(lot.update_dt.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.update_dt));
        Poco::Nullable<std::string> lot_in_central_sent_dt(lot.lot_in_central_sent_dt.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.lot_in_central_sent_dt));
        Poco::Nullable<std::string> lot_out_central_sent_dt(lot.lot_out_central_sent_dt.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(lot.lot_out_central_sent_dt));

        insert << query,
                Poco::Data::Keywords::use(location_code),
                Poco::Data::Keywords::use(lot_no),
                Poco::Data::Keywords::use(lpn),
                Poco::Data::Keywords::use(lot_in_image_path),
                Poco::Data::Keywords::use(lot_out_image_path),
                Poco::Data::Keywords::use(lot_in_dt),
                Poco::Data::Keywords::use(lot_out_dt),
                Poco::Data::Keywords::use(add_dt),
                Poco::Data::Keywords::use(update_dt),
                Poco::Data::Keywords::use(lot_in_central_sent_dt),
                Poco::Data::Keywords::use(lot_out_central_sent_dt);

        insert.execute();
        
        // Log message
        std::ostringstream msg;
        msg << insert.toString();
        msg << " ==> (" << location_code << ", ";
        msg << lot_no << ", ";
        msg << lpn << ", ";
        msg << lot_in_image_path << ", ";
        msg << lot_out_image_path << ", ";
        msg << lot_in_dt << ", ";
        msg << lot_out_dt << ", ";
        msg << add_dt << ", ";
        msg << update_dt << ", ";
        msg << lot_in_central_sent_dt << ", ";
        msg << lot_out_central_sent_dt << ")";
        AppLogger::getInstance()->FnLog(msg.str());
    }
    catch(const Poco::Exception& ex)
    {
        std::ostringstream msg;
        msg << __func__ << " POCO Exception: " << ex.displayText() << std::endl;
        std::cerr << msg.str();
        AppLogger::getInstance()->FnLog(msg.str());
    }
    
}

void Database::FnSelectAllRecord(const std::string& tableName, std::vector<parking_lot_t>& v_lot)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    if (!session_->isConnected())
    {
        session_->reconnect();
    }

    try
    {
        std::string query = "SELECT * FROM " + tableName;
        Poco::Data::Statement select(*session_);
        select << query;

        select.execute();
        AppLogger::getInstance()->FnLog(select.toString());

        Poco::Data::RecordSet recordSet(select);
        if (recordSet.moveFirst())
        {
            do
            {
                parking_lot_t lot;
                memset(&lot, 0, sizeof(parking_lot_t));

                lot.location_code = recordSet["location_code"].isEmpty() ? "NULL" : recordSet["location_code"].convert<std::string>();
                lot.lot_no = recordSet["lot_no"].isEmpty() ? "NULL" : recordSet["lot_no"].convert<std::string>();
                lot.lpn = recordSet["lpn"].isEmpty() ? "NULL" : recordSet["lpn"].convert<std::string>();
                lot.lot_in_image_path = recordSet["lot_in_image"].isEmpty() ? "NULL" : recordSet["lot_in_image"].convert<std::string>();
                lot.lot_out_image_path = recordSet["lot_out_image"].isEmpty() ? "NULL" : recordSet["lot_out_image"].convert<std::string>();
                lot.lot_in_dt = recordSet["lot_in_dt"].isEmpty() ? "NULL" : Poco::DateTimeFormatter::format(recordSet["lot_in_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
                lot.lot_out_dt = recordSet["lot_out_dt"].isEmpty() ? "NULL" : Poco::DateTimeFormatter::format(recordSet["lot_out_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
                lot.add_dt = recordSet["add_dt"].isEmpty() ? "NULL" : Poco::DateTimeFormatter::format(recordSet["add_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
                lot.update_dt = recordSet["update_dt"].isEmpty() ? "NULL" : Poco::DateTimeFormatter::format(recordSet["update_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
                lot.lot_in_central_sent_dt = recordSet["lot_in_central_sent_dt"].isEmpty() ? "NULL" : Poco::DateTimeFormatter::format(recordSet["lot_in_central_sent_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
                lot.lot_out_central_sent_dt = recordSet["lot_out_central_sent_dt"].isEmpty() ? "NULL" : Poco::DateTimeFormatter::format(recordSet["lot_out_central_sent_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");

                v_lot.push_back(lot);

                // Log the values for each record
                std::ostringstream msg;
                msg << "Record: "
                    << "location_code=" << lot.location_code
                    << ", lot_no=" << lot.lot_no
                    << ", lpn=" << lot.lpn
                    << ", lot_in_image_path=" << lot.lot_in_image_path
                    << ", lot_out_image_path=" << lot.lot_out_image_path
                    << ", lot_in_dt=" << lot.lot_in_dt
                    << ", lot_out_dt=" << lot.lot_out_dt
                    << ", add_dt=" << lot.add_dt
                    << ", update_dt=" << lot.update_dt
                    << ", lot_in_central_sent_dt=" << lot.lot_in_central_sent_dt
                    << ", lot_out_central_sent_dt=" << lot.lot_out_central_sent_dt;
                AppLogger::getInstance()->FnLog(msg.str());
            } while (recordSet.moveNext());
        }

    }
    catch(const Poco::Exception& ex)
    {
        std::ostringstream msg;
        msg << __func__ << " POCO Exception: " << ex.displayText() << std::endl;
        std::cerr << msg.str();
        AppLogger::getInstance()->FnLog(msg.str());
    }
}

bool Database::FnIsTableEmpty(const std::string& tableName)
{
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    if (!session_->isConnected())
    {
        session_->reconnect();
    }

    try
    {
        std::string query = "SELECT COUNT(*) FROM " + tableName;
        Poco::Data::Statement select(*session_);
        select << query;

        select.execute();
        AppLogger::getInstance()->FnLog(select.toString());

        Poco::Data::RecordSet recordSet(select);
        
        if (recordSet.moveFirst())
        {
            int count;
            count = recordSet["COUNT(*)"].convert<int>();
            return count == 0;
        }

        return false;
    }
    catch(const Poco::Exception& ex)
    {
        std::ostringstream msg;
        msg << __func__ << " POCO Exception: " << ex.displayText() << std::endl;
        std::cerr << msg.str();
        AppLogger::getInstance()->FnLog(msg.str());

        return false;
    }
    
}

void Database::FnRemoveAllRecord(const std::string& tableName)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    if (!session_->isConnected())
    {
        session_->reconnect();
    }

    try
    {
        std::string query = "DELETE FROM " + tableName;
        Poco::Data::Statement remove(*session_);
        remove << query;

        remove.execute();
        AppLogger::getInstance()->FnLog(remove.toString());
    }
    catch(const Poco::Exception& ex)
    {
        std::ostringstream msg;
        msg << __func__ << " POCO Exception: " << ex.displayText() << std::endl;
        std::cerr << msg.str();
        AppLogger::getInstance()->FnLog(msg.str());
    }
}

void Database::FnUpdateThreeLotParkingStatus(const std::string& tableName)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    if (!session_->isConnected())
    {
        session_->reconnect();
    }

    try
    {
        // Query on the lot no == '1' and latest time in the record
        std::string query_first_lot = "SELECT * FROM " + tableName + " WHERE lot_no = '1' ORDER BY COALESCE(lot_in_dt, lot_out_dt) DESC LIMIT 1";
        Poco::Data::Statement select_first_lot(*session_);
        select_first_lot << query_first_lot;

        select_first_lot.execute();
        AppLogger::getInstance()->FnLog(select_first_lot.toString());

        Poco::Data::RecordSet recordSetFirstLot(select_first_lot);
        if (recordSetFirstLot.moveFirst())
        {
            parking_lot_t* pFirstParkingLot = &firstParkingLot_.parking_lot_details;

            pFirstParkingLot->location_code = recordSetFirstLot["location_code"].isEmpty() ? "" : recordSetFirstLot["location_code"].convert<std::string>();
            pFirstParkingLot->lot_no = recordSetFirstLot["lot_no"].isEmpty() ? "" : recordSetFirstLot["lot_no"].convert<std::string>();
            pFirstParkingLot->lpn = recordSetFirstLot["lpn"].isEmpty() ? "" : recordSetFirstLot["lpn"].convert<std::string>();
            pFirstParkingLot->lot_in_image_path = recordSetFirstLot["lot_in_image"].isEmpty() ? "" : recordSetFirstLot["lot_in_image"].convert<std::string>();
            pFirstParkingLot->lot_out_image_path = recordSetFirstLot["lot_out_image"].isEmpty() ? "" : recordSetFirstLot["lot_out_image"].convert<std::string>();
            pFirstParkingLot->lot_in_dt = recordSetFirstLot["lot_in_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetFirstLot["lot_in_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pFirstParkingLot->lot_out_dt = recordSetFirstLot["lot_out_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetFirstLot["lot_out_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pFirstParkingLot->add_dt = recordSetFirstLot["add_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetFirstLot["add_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pFirstParkingLot->update_dt = recordSetFirstLot["update_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetFirstLot["update_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pFirstParkingLot->lot_in_central_sent_dt = recordSetFirstLot["lot_in_central_sent_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetFirstLot["lot_in_central_sent_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pFirstParkingLot->lot_out_central_sent_dt = recordSetFirstLot["lot_out_central_sent_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetFirstLot["lot_out_central_sent_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");

            if (!pFirstParkingLot->lot_in_dt.empty())
            {
                firstParkingLot_.status = "occupy";
            }
            else if (!pFirstParkingLot->lot_out_dt.empty())
            {
                firstParkingLot_.status = "release";
            }

            // Log the values for each record
            std::ostringstream firstMsg;
            firstMsg << "Record: "
                << "pFirstParkingLot->location_code=" << pFirstParkingLot->location_code
                << ", pFirstParkingLot->lot_no=" << pFirstParkingLot->lot_no
                << ", pFirstParkingLot->lpn=" << pFirstParkingLot->lpn
                << ", pFirstParkingLot->lot_in_image_path=" << pFirstParkingLot->lot_in_image_path
                << ", pFirstParkingLot->lot_out_image_path=" << pFirstParkingLot->lot_out_image_path
                << ", pFirstParkingLot->lot_in_dt=" << pFirstParkingLot->lot_in_dt
                << ", pFirstParkingLot->lot_out_dt=" << pFirstParkingLot->lot_out_dt
                << ", pFirstParkingLot->add_dt=" << pFirstParkingLot->add_dt
                << ", pFirstParkingLot->update_dt=" << pFirstParkingLot->update_dt
                << ", pFirstParkingLot->lot_in_central_sent_dt=" << pFirstParkingLot->lot_in_central_sent_dt
                << ", pFirstParkingLot->lot_out_central_sent_dt=" << pFirstParkingLot->lot_out_central_sent_dt
                << ", firstParkingLot_.status=" << firstParkingLot_.status;
            AppLogger::getInstance()->FnLog(firstMsg.str());
        }

        // Query on the lot no == '2' and latest time in the record
        std::string query_second_lot = "SELECT * FROM " + tableName + " WHERE lot_no = '2' ORDER BY COALESCE(lot_in_dt, lot_out_dt) DESC LIMIT 1";
        Poco::Data::Statement select_second_lot(*session_);
        select_second_lot << query_second_lot;

        select_second_lot.execute();
        AppLogger::getInstance()->FnLog(select_second_lot.toString());

        Poco::Data::RecordSet recordSetSecondLot(select_second_lot);
        if (recordSetSecondLot.moveFirst())
        {
            parking_lot_t* pSecondParkingLot = &secondParkingLot_.parking_lot_details;

            pSecondParkingLot->location_code = recordSetSecondLot["location_code"].isEmpty() ? "" : recordSetSecondLot["location_code"].convert<std::string>();
            pSecondParkingLot->lot_no = recordSetSecondLot["lot_no"].isEmpty() ? "" : recordSetSecondLot["lot_no"].convert<std::string>();
            pSecondParkingLot->lpn = recordSetSecondLot["lpn"].isEmpty() ? "" : recordSetSecondLot["lpn"].convert<std::string>();
            pSecondParkingLot->lot_in_image_path = recordSetSecondLot["lot_in_image"].isEmpty() ? "" : recordSetSecondLot["lot_in_image"].convert<std::string>();
            pSecondParkingLot->lot_out_image_path = recordSetSecondLot["lot_out_image"].isEmpty() ? "" : recordSetSecondLot["lot_out_image"].convert<std::string>();
            pSecondParkingLot->lot_in_dt = recordSetSecondLot["lot_in_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetSecondLot["lot_in_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pSecondParkingLot->lot_out_dt = recordSetSecondLot["lot_out_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetSecondLot["lot_out_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pSecondParkingLot->add_dt = recordSetSecondLot["add_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetSecondLot["add_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pSecondParkingLot->update_dt = recordSetSecondLot["update_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetSecondLot["update_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pSecondParkingLot->lot_in_central_sent_dt = recordSetSecondLot["lot_in_central_sent_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetSecondLot["lot_in_central_sent_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pSecondParkingLot->lot_out_central_sent_dt = recordSetSecondLot["lot_out_central_sent_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetSecondLot["lot_out_central_sent_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");

            if (!pSecondParkingLot->lot_in_dt.empty())
            {
                secondParkingLot_.status = "occupy";
            }
            else if (!pSecondParkingLot->lot_out_dt.empty())
            {
                secondParkingLot_.status = "release";
            }

            // Log the values for each record
            std::ostringstream secondMsg;
            secondMsg << "Record: "
                << "pSecondParkingLot->location_code=" << pSecondParkingLot->location_code
                << ", pSecondParkingLot->lot_no=" << pSecondParkingLot->lot_no
                << ", pSecondParkingLot->lpn=" << pSecondParkingLot->lpn
                << ", pSecondParkingLot->lot_in_image_path=" << pSecondParkingLot->lot_in_image_path
                << ", pSecondParkingLot->lot_out_image_path=" << pSecondParkingLot->lot_out_image_path
                << ", pSecondParkingLot->lot_in_dt=" << pSecondParkingLot->lot_in_dt
                << ", pSecondParkingLot->lot_out_dt=" << pSecondParkingLot->lot_out_dt
                << ", pSecondParkingLot->add_dt=" << pSecondParkingLot->add_dt
                << ", pSecondParkingLot->update_dt=" << pSecondParkingLot->update_dt
                << ", pSecondParkingLot->lot_in_central_sent_dt=" << pSecondParkingLot->lot_in_central_sent_dt
                << ", pSecondParkingLot->lot_out_central_sent_dt=" << pSecondParkingLot->lot_out_central_sent_dt
                << ", secondParkingLot_.status=" << secondParkingLot_.status;
            AppLogger::getInstance()->FnLog(secondMsg.str());
        }

        // Query on the lot no == '3' and latest time in the record
        std::string query_third_lot = "SELECT * FROM " + tableName + " WHERE lot_no = '3' ORDER BY COALESCE(lot_in_dt, lot_out_dt) DESC LIMIT 1";
        Poco::Data::Statement select_third_lot(*session_);
        select_third_lot << query_third_lot;

        select_third_lot.execute();
        AppLogger::getInstance()->FnLog(select_third_lot.toString());

        Poco::Data::RecordSet recordSetThirdLot(select_third_lot);
        if (recordSetThirdLot.moveFirst())
        {
            parking_lot_t* pThirdParkingLot = &thirdParkingLot_.parking_lot_details;

            pThirdParkingLot->location_code = recordSetThirdLot["location_code"].isEmpty() ? "" : recordSetThirdLot["location_code"].convert<std::string>();
            pThirdParkingLot->lot_no = recordSetThirdLot["lot_no"].isEmpty() ? "" : recordSetThirdLot["lot_no"].convert<std::string>();
            pThirdParkingLot->lpn = recordSetThirdLot["lpn"].isEmpty() ? "" : recordSetThirdLot["lpn"].convert<std::string>();
            pThirdParkingLot->lot_in_image_path = recordSetThirdLot["lot_in_image"].isEmpty() ? "" : recordSetThirdLot["lot_in_image"].convert<std::string>();
            pThirdParkingLot->lot_out_image_path = recordSetThirdLot["lot_out_image"].isEmpty() ? "" : recordSetThirdLot["lot_out_image"].convert<std::string>();
            pThirdParkingLot->lot_in_dt = recordSetThirdLot["lot_in_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetThirdLot["lot_in_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pThirdParkingLot->lot_out_dt = recordSetThirdLot["lot_out_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetThirdLot["lot_out_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pThirdParkingLot->add_dt = recordSetThirdLot["add_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetThirdLot["add_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pThirdParkingLot->update_dt = recordSetThirdLot["update_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetThirdLot["update_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pThirdParkingLot->lot_in_central_sent_dt = recordSetThirdLot["lot_in_central_sent_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetThirdLot["lot_in_central_sent_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
            pThirdParkingLot->lot_out_central_sent_dt = recordSetThirdLot["lot_out_central_sent_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSetThirdLot["lot_out_central_sent_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");

             if (!pThirdParkingLot->lot_in_dt.empty())
            {
                thirdParkingLot_.status = "occupy";
            }
            else if (!pThirdParkingLot->lot_out_dt.empty())
            {
                thirdParkingLot_.status = "release";
            }

            // Log the values for each record
            std::ostringstream thirdMsg;
            thirdMsg << "Record: "
                << "pThirdParkingLot->location_code=" << pThirdParkingLot->location_code
                << ", pThirdParkingLot->lot_no=" << pThirdParkingLot->lot_no
                << ", pThirdParkingLot->lpn=" << pThirdParkingLot->lpn
                << ", pThirdParkingLot->lot_in_image_path=" << pThirdParkingLot->lot_in_image_path
                << ", pThirdParkingLot->lot_out_image_path=" << pThirdParkingLot->lot_out_image_path
                << ", pThirdParkingLot->lot_in_dt=" << pThirdParkingLot->lot_in_dt
                << ", pThirdParkingLot->lot_out_dt=" << pThirdParkingLot->lot_out_dt
                << ", pThirdParkingLot->add_dt=" << pThirdParkingLot->add_dt
                << ", pThirdParkingLot->update_dt=" << pThirdParkingLot->update_dt
                << ", pThirdParkingLot->lot_in_central_sent_dt=" << pThirdParkingLot->lot_in_central_sent_dt
                << ", pThirdParkingLot->lot_out_central_sent_dt=" << pThirdParkingLot->lot_out_central_sent_dt
                << ", thirdParkingLot_.status=" << thirdParkingLot_.status;
            AppLogger::getInstance()->FnLog(thirdMsg.str());
        }
    }
    catch(const Poco::Exception& ex)
    {
        std::ostringstream msg;
        msg << __func__ << " POCO Exception: " << ex.displayText() << std::endl;
        std::cerr << msg.str();
        AppLogger::getInstance()->FnLog(msg.str());
    }
}

const Database::parking_lot_info_t& Database::FnGetFirstParkingLot() const
{
    return firstParkingLot_;
}

void Database::FnSetFirstParkingLotStartUpFlag(bool flag/*=true*/)
{
    firstParkingLot_.start_up_flag = flag;
}

const Database::parking_lot_info_t& Database::FnGetSecondParkingLot() const
{
    return secondParkingLot_;
}

void Database::FnSetSecondParkingLotStartUpFlag(bool flag/*=true*/)
{
    secondParkingLot_.start_up_flag = flag;
}

const Database::parking_lot_info_t& Database::FnGetThirdParkingLot() const
{
    return thirdParkingLot_;
}

void Database::FnSetThirdParkingLotStartUpFlag(bool flag/*=true*/)
{
    thirdParkingLot_.start_up_flag = flag;
}

void Database::FnSendDBParkingLotStatusToCentral(const std::string& tableName)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    if (!session_->isConnected())
    {
        session_->reconnect();
    }

    try
    {
        std::string query = "SELECT * FROM " + tableName + " WHERE lot_in_central_sent_dt IS NULL AND lot_out_central_sent_dt IS NULL";
        Poco::Data::Statement select(*session_);
        select << query;

        select.execute();
        AppLogger::getInstance()->FnLog(select.toString());

        Poco::Data::RecordSet recordSet(select);
        if (recordSet.moveFirst())
        {
            do
            {
                std::string lot_in_image = "";
                std::string lot_out_image = "";
                Poco::Nullable<std::string> lot_in_central_sent_dt = Poco::Nullable<std::string>();
                Poco::Nullable<std::string> lot_out_central_sent_dt = Poco::Nullable<std::string>();
                std::string lot_no = recordSet["lot_no"].isEmpty() ? "" : recordSet["lot_no"].convert<std::string>();
                std::string lpn = recordSet["lpn"].isEmpty() ? "" : recordSet["lpn"].convert<std::string>();
                std::string lot_in_image_path = recordSet["lot_in_image"].isEmpty() ? "NULL" : recordSet["lot_in_image"].convert<std::string>();
                std::string lot_out_image_path = recordSet["lot_out_image"].isEmpty() ? "NULL" : recordSet["lot_out_image"].convert<std::string>();
                std::string lot_in_dt = recordSet["lot_in_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSet["lot_in_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");
                std::string lot_out_dt = recordSet["lot_out_dt"].isEmpty() ? "" : Poco::DateTimeFormatter::format(recordSet["lot_out_dt"].convert<Poco::DateTime>(), "%Y-%m-%d %H:%M:%S");

                if (!(lot_in_image_path.compare("NULL") == 0))
                {
                   lot_in_image = Common::getInstance()->FnConverImageToBase64String(lot_in_image_path);
                   lot_in_central_sent_dt = Common::getInstance()->FnCurrentFormatDateYYYY_MM_DD_HH_MM_SS();

                }

                if (!(lot_out_image_path.compare("NULL") == 0))
                {
                    lot_out_image = Common::getInstance()->FnConverImageToBase64String(lot_out_image_path);
                    lot_out_central_sent_dt = Common::getInstance()->FnCurrentFormatDateYYYY_MM_DD_HH_MM_SS();
                }

                // Log the values for each record
                std::ostringstream selectMsg;
                selectMsg << "Record: "
                    << "lot_no=" << lot_no
                    << ", lpn=" << lpn
                    << ", lot_in_image_path=" << lot_in_image_path
                    << ", lot_out_image_path=" << lot_out_image_path
                    << ", lot_in_dt=" << lot_in_dt
                    << ", lot_out_dt=" << lot_out_dt
                    << ", lot_in_central_sent_dt=" << lot_in_central_sent_dt
                    << ", lot_out_central_sent_dt=" << lot_out_central_sent_dt;
                AppLogger::getInstance()->FnLog(selectMsg.str());

                if (Central::getInstance()->FnSendParkInParkOutInfo(lot_no, lpn, lot_in_image, lot_out_image, lot_in_dt, lot_out_dt))
                {
                    std::string id = recordSet["id"].convert<std::string>();

                    // Update the record in database
                    std::string updateQuery = "UPDATE " + tableName + " SET lot_in_central_sent_dt = ?, lot_out_central_sent_dt = ? WHERE id = ?";
                    Poco::Data::Statement update(*session_);
                    update << updateQuery,
                            Poco::Data::Keywords::use(lot_in_central_sent_dt),
                            Poco::Data::Keywords::use(lot_out_central_sent_dt),
                            Poco::Data::Keywords::use(id);

                    update.execute();

                    AppLogger::getInstance()->FnLog(update.toString());
                    std::ostringstream updateMsg;
                    updateMsg << "lot_in_central_sent_dt=" << lot_in_central_sent_dt
                        << ", lot_out_central_sent_dt=" << lot_out_central_sent_dt
                        << ", id=" << id;
                    AppLogger::getInstance()->FnLog(updateMsg.str());
                }
            } while (recordSet.moveNext());
        }

    }
    catch(const Poco::Exception& ex)
    {
        std::ostringstream msg;
        msg << __func__ << " POCO Exception: " << ex.displayText() << std::endl;
        std::cerr << msg.str();
        AppLogger::getInstance()->FnLog(msg.str());
    }
}

void Database::FnInsertStatusRecord(const std::string& tableName, const std::string& carpark_code, const std::string& dev_ip, const std::string& ec)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    if (!session_->isConnected())
    {
        session_->reconnect();
    }

    try
    {
        std::string query = "INSERT INTO " + tableName + " (location_code, device_ip, error_code) VALUES(?, ?, ?)";
        Poco::Data::Statement insert(*session_);

        Poco::Nullable<std::string> location_code(carpark_code.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(carpark_code));
        Poco::Nullable<std::string> device_ip(dev_ip.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(dev_ip));
        Poco::Nullable<std::string> error_code(ec.empty() ? Poco::Nullable<std::string>() : Poco::Nullable<std::string>(ec));

        insert << query,
                Poco::Data::Keywords::use(location_code),
                Poco::Data::Keywords::use(device_ip),
                Poco::Data::Keywords::use(error_code);

        insert.execute();
        
        // Log message
        std::ostringstream msg;
        msg << insert.toString();
        msg << " ==> (" << location_code << ", ";
        msg << device_ip << ", ";
        msg << error_code << ")";
        AppLogger::getInstance()->FnLog(msg.str());
    }
    catch(const Poco::Exception& ex)
    {
        std::ostringstream msg;
        msg << __func__ << " POCO Exception: " << ex.displayText() << std::endl;
        std::cerr << msg.str();
        AppLogger::getInstance()->FnLog(msg.str());
    }
}

void Database::FnSendDBDeviceStatusToCentral(const std::string& tableName)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(databaseMutex_);

    if (!session_->isConnected())
    {
        session_->reconnect();
    }

    try
    {
        std::string query = "SELECT * FROM " + tableName + " WHERE central_sent_dt IS NULL";
        Poco::Data::Statement select(*session_);
        select << query;

        select.execute();
        AppLogger::getInstance()->FnLog(select.toString());

        Poco::Data::RecordSet recordSet(select);
        if (recordSet.moveFirst())
        {
            do
            {
                std::string location_code = recordSet["location_code"].isEmpty() ? "" : recordSet["location_code"].convert<std::string>();
                std::string device_ip = recordSet["device_ip"].isEmpty() ? "" : recordSet["device_ip"].convert<std::string>();
                std::string error_code = recordSet["error_code"].isEmpty() ? "NULL" : recordSet["error_code"].convert<std::string>();
                std::string central_sent_dt = Common::getInstance()->FnCurrentFormatDateYYYY_MM_DD_HH_MM_SS();

                // Log the values for each record
                std::ostringstream selectMsg;
                selectMsg << "Record: "
                    << "location_code=" << location_code
                    << ", device_ip=" << device_ip
                    << ", error_code=" << error_code
                    << ", central_sent_dt=" << central_sent_dt;
                AppLogger::getInstance()->FnLog(selectMsg.str());

                if (Central::getInstance()->FnSendDeviceStatusUpdate(location_code, device_ip, error_code))
                {
                    std::string id = recordSet["id"].convert<std::string>();

                    // Update the record in database
                    std::string updateQuery = "UPDATE " + tableName + " SET central_sent_dt = ? WHERE id = ?";
                    Poco::Data::Statement update(*session_);
                    update << updateQuery,
                            Poco::Data::Keywords::use(central_sent_dt),
                            Poco::Data::Keywords::use(id);

                    update.execute();

                    AppLogger::getInstance()->FnLog(update.toString());
                    std::ostringstream updateMsg;
                    updateMsg << "lot_in_central_sent_dt=" << central_sent_dt
                        << ", id=" << id;
                    AppLogger::getInstance()->FnLog(updateMsg.str());
                }
            } while (recordSet.moveNext());
        }

    }
    catch(const Poco::Exception& ex)
    {
        std::ostringstream msg;
        msg << __func__ << " POCO Exception: " << ex.displayText() << std::endl;
        std::cerr << msg.str();
        AppLogger::getInstance()->FnLog(msg.str());
    }
}