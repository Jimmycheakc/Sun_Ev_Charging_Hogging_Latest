#include <iostream>
#include <string.h>
#include <sstream>
#include "database.h"
#include "ini_parser.h"
#include "camera.h"
#include "central.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPDigestCredentials.h"
#include "Poco/Path.h"
#include "Poco/StreamCopier.h"
#include "Poco/StringTokenizer.h"
#include "Poco/FileStream.h"
#include "log.h"
#include "timer.h"

using namespace Poco::Net;
using namespace Poco;

int main(int argc, char* agrv[])
{
    std::ostringstream info;
    info << "start " << agrv[0] << " , version: 0.0.1 build:" << __DATE__ << " " << __TIME__;
    AppLogger::getInstance()->FnLog(info.str());
    
    //std::cout<< "Result:" << Camera::getInstance()->FnGetHeartBeat() << std::endl;
    //std::cout<< "Result:" << Camera::getInstance()->FnGetSnapShot() << std::endl;
    //Camera::getInstance()->FnSubscribeToSnapShot();
    
    //Camera::getInstance()->FnSetCurrentTime();
    //std::string time;
    //Camera::getInstance()->FnGetCurrentTime(time);
    //std::cout << time << std::endl;

    /*
    Database::getInstance()->FnDatabaseInit();
    Database::getInstance()->FnRemoveAllRecord("tbl_ev_lot_trans");
    Database::parking_lot_t lot = 
    {"1", 
     "1",
     "ABC 0000",
     "/usr/local/image/in",
     "",
     "2023-12-13 01:59:59",
     "",
     "",
     "",
     "",
     ""
     };
     Database::parking_lot_t lot1 = 
    {"1", 
     "1",
     "ABC 0000",
     "/usr/local/image/in",
     "",
     "2023-12-13 02:59:59",
     "",
     "",
     "",
     "",
     ""
     };
     Database::parking_lot_t lot2 = 
    {"1", 
     "1",
     "ABC 0000",
     "",
     "/usr/local/image/out",
     "",
     "2023-12-13 04:59:59",
     "",
     "",
     "",
     ""
     };
     Database::parking_lot_t lot3 = 
    {"1", 
     "2",
     "ABC 0000",
     "",
     "/usr/local/image/out",
     "",
     "2023-12-13 02:59:59",
     "",
     "",
     "",
     ""
     };
     Database::parking_lot_t lot4 = 
    {"1", 
     "2",
     "ABC 0000",
     "/usr/local/image/in",
     "",
     "2023-12-13 03:59:59",
     "",
     "",
     "",
     "",
     ""
     };
     Database::parking_lot_t lot5 = 
    {"1", 
     "2",
     "ABC 0000",
     "/usr/local/image/in",
     "",
     "2023-12-13 05:59:59",
     "",
     "",
     "",
     "",
     ""
     };
     Database::parking_lot_t lot6 = 
    {"1", 
     "3",
     "ABC 0000",
     "/usr/local/image/in",
     "",
     "2023-12-13 05:59:59",
     "",
     "",
     "",
     "",
     ""
     };
     Database::parking_lot_t lot7 = 
    {"1", 
     "3",
     "ABC 0000",
     "/usr/local/image/in",
     "",
     "2023-12-13 02:59:59",
     "",
     "",
     "",
     "",
     ""
     };
     Database::parking_lot_t lot8 = 
    {"1", 
     "3",
     "ABC 0000",
     "/usr/local/image/in",
     "",
     "2023-12-13 01:59:59",
     "",
     "",
     "",
     "",
     ""
     };
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", lot);
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", lot1);
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", lot2);
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", lot3);
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", lot4);
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", lot5);
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", lot6);
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", lot7);
    Database::getInstance()->FnInsertRecord("tbl_ev_lot_trans", lot8);

    Database::getInstance()->FnUpdateThreeLotParkingStatus("tbl_ev_lot_trans");
    std::cout << "location_code : " << Database::getInstance()->FnGetFirstParkingLot().location_code << std::endl;
    std::cout << "lot_no : " << Database::getInstance()->FnGetFirstParkingLot().lot_no << std::endl;
    std::cout << "lpn : " << Database::getInstance()->FnGetFirstParkingLot().lpn << std::endl;
    std::cout << "lot_in_image_path : " << Database::getInstance()->FnGetFirstParkingLot().lot_in_image_path << std::endl;
    std::cout << "lot_out_image_path : " << Database::getInstance()->FnGetFirstParkingLot().lot_out_image_path << std::endl;
    std::cout << "lot_in_dt : " << Database::getInstance()->FnGetFirstParkingLot().lot_in_dt << std::endl;
    std::cout << "lot_out_dt : " << Database::getInstance()->FnGetFirstParkingLot().lot_out_dt << std::endl;
    std::cout << "add_dt : " << Database::getInstance()->FnGetFirstParkingLot().add_dt << std::endl;
    std::cout << "update_dt : " << Database::getInstance()->FnGetFirstParkingLot().update_dt << std::endl;
    std::cout << "lot_in_central_sent_dt : " << Database::getInstance()->FnGetFirstParkingLot().lot_in_central_sent_dt << std::endl;
    std::cout << "lot_out_central_sent_dt : " << Database::getInstance()->FnGetFirstParkingLot().lot_out_central_sent_dt << std::endl;

    std:: cout << std::endl << std::endl;
    std::cout << "location_code : " << Database::getInstance()->FnGetSecondParkingLot().location_code << std::endl;
    std::cout << "lot_no : " << Database::getInstance()->FnGetSecondParkingLot().lot_no << std::endl;
    std::cout << "lpn : " << Database::getInstance()->FnGetSecondParkingLot().lpn << std::endl;
    std::cout << "lot_in_image_path : " << Database::getInstance()->FnGetSecondParkingLot().lot_in_image_path << std::endl;
    std::cout << "lot_out_image_path : " << Database::getInstance()->FnGetSecondParkingLot().lot_out_image_path << std::endl;
    std::cout << "lot_in_dt : " << Database::getInstance()->FnGetSecondParkingLot().lot_in_dt << std::endl;
    std::cout << "lot_out_dt : " << Database::getInstance()->FnGetSecondParkingLot().lot_out_dt << std::endl;
    std::cout << "add_dt : " << Database::getInstance()->FnGetSecondParkingLot().add_dt << std::endl;
    std::cout << "update_dt : " << Database::getInstance()->FnGetSecondParkingLot().update_dt << std::endl;
    std::cout << "lot_in_central_sent_dt : " << Database::getInstance()->FnGetSecondParkingLot().lot_in_central_sent_dt << std::endl;
    std::cout << "lot_out_central_sent_dt : " << Database::getInstance()->FnGetSecondParkingLot().lot_out_central_sent_dt << std::endl;

    std:: cout << std::endl << std::endl;
    std::cout << "location_code : " << Database::getInstance()->FnGetThirdParkingLot().location_code << std::endl;
    std::cout << "lot_no : " << Database::getInstance()->FnGetThirdParkingLot().lot_no << std::endl;
    std::cout << "lpn : " << Database::getInstance()->FnGetThirdParkingLot().lpn << std::endl;
    std::cout << "lot_in_image_path : " << Database::getInstance()->FnGetThirdParkingLot().lot_in_image_path << std::endl;
    std::cout << "lot_out_image_path : " << Database::getInstance()->FnGetThirdParkingLot().lot_out_image_path << std::endl;
    std::cout << "lot_in_dt : " << Database::getInstance()->FnGetThirdParkingLot().lot_in_dt << std::endl;
    std::cout << "lot_out_dt : " << Database::getInstance()->FnGetThirdParkingLot().lot_out_dt << std::endl;
    std::cout << "add_dt : " << Database::getInstance()->FnGetThirdParkingLot().add_dt << std::endl;
    std::cout << "update_dt : " << Database::getInstance()->FnGetThirdParkingLot().update_dt << std::endl;
    std::cout << "lot_in_central_sent_dt : " << Database::getInstance()->FnGetThirdParkingLot().lot_in_central_sent_dt << std::endl;
    std::cout << "lot_out_central_sent_dt : " << Database::getInstance()->FnGetThirdParkingLot().lot_out_central_sent_dt << std::endl;
    */

    /*
    Database::getInstance()->FnDatabaseInit();
    std::vector<Database::parking_lot_t> lots;
    Database::getInstance()->FnSelectAllRecord("tbl_ev_lot_trans", lots);

    for (auto& parking_lot : lots)
    {
        std::cout << "parking_lot.location_code : " << parking_lot.location_code << std::endl;
        std::cout << "parking_lot.lot_no : " << parking_lot.lot_no << std::endl;
        std::cout << "parking_lot.lpn : " << parking_lot.lpn << std::endl;
        std::cout << "parking_lot.lot_in_image : " << parking_lot.lot_in_image_path << std::endl;
        std::cout << "parking_lot.lot_out_image : " << parking_lot.lot_out_image_path << std::endl;
        std::cout << "parking_lot.lot_in_dt : " << parking_lot.lot_in_dt << std::endl;
        std::cout << "parking_lot.lot_out_dt : " << parking_lot.lot_out_dt << std::endl;
        std::cout << "parking_lot.add_dt : " << parking_lot.add_dt << std::endl;
        std::cout << "parking_lot.update_dt : " << parking_lot.update_dt << std::endl;
        std::cout << "parking_lot.lot_in_central_sent_dt : " << parking_lot.lot_in_central_sent_dt << std::endl;
        std::cout << "parking_lot.lot_out_central_sent_dt : " << parking_lot.lot_out_central_sent_dt << std::endl;
    }

    std::cout << "Result : " << Database::getInstance()->FnIsTableEmpty("tbl_ev_lot_trans") << std::endl;
    Database::getInstance()->FnRemoveAllRecord("tbl_ev_lot_trans");
    std::cout << "Result : " << Database::getInstance()->FnIsTableEmpty("tbl_ev_lot_trans") << std::endl;
    */

    //Iniparser::getInstance()->FnIniParserInit();
    //Database::getInstance()->FnDatabaseInit();
    //Database::getInstance()->FnRemoveAllRecord("tbl_ev_lot_trans");
    //EvtTimer::getInstance()->FnStartFilterTimer();
    //Camera::getInstance()->FnSubscribeToSnapShot();


    //Iniparser::getInstance()->FnIniParserInit();
    //std::cout << Iniparser::getInstance()->FnGetParkingLotLocationCode() << std::endl;
    //std::cout << Iniparser::getInstance()->FnGetTimerForFilteringSnapShot() << std::endl;

    //std::cout << "Result : " << Central::getInstance()->FnSendHeartBeatUpdate() << std::endl;
    //std::cout << "Result : " << Central::getInstance()->FnSendDeviceStatusUpdate(Iniparser::getInstance()->FnGetCameraIP(), Central::ERROR_CODE_CAMERA) << std::endl;
    //std::cout << "Result : " << Central::getInstance()->FnSendParkInParkOutInfo() << std::endl;

    /*
    Iniparser::getInstance()->FnIniParserInit();
    EvtTimer::getInstance()->FnStartFilterTimer();
    Thread::sleep(20000);
    std::cout << "Stop Timer" << std::endl;
    EvtTimer::getInstance()->FnStopFilterTimer();
    Thread::sleep(20000);
    */

    /*
    EvtTimer::getInstance()->FnStartStartUpTimer();
    Thread::sleep(20000);
    std::cout << "Stop Timer" << std::endl;
    EvtTimer::getInstance()->FnStopStartUpTimer();
    Thread::sleep(20000);
    */

    Iniparser::getInstance()->FnIniParserInit();
    Database::getInstance()->FnDatabaseInit();
    Database::getInstance()->FnRemoveAllRecord("tbl_ev_lot_trans");
    Database::getInstance()->FnRemoveAllRecord("tbl_ev_lot_trans_temp");
    //Central::getInstance()->FnSendParkInParkOutInfo();
    EvtTimer::getInstance()->FnStartStartUpTimer();
    Camera::getInstance()->FnSubscribeToSnapShot();

    return 0;
}