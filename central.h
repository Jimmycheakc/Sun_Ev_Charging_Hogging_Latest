#pragma once

#include <iostream>
#include <string>
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Types.h"

class Central{

public:
    static const std::string ERROR_CODE_RECOVERED;
    static const std::string ERROR_CODE_CAMERA;
    static const std::string ERROR_CODE_IPC;

    const std::string username = "testuser";
    const std::string password = "testpassword";

    static Central* getInstance();
    bool FnSendHeartBeatUpdate();
    bool FnSendDeviceStatusUpdate(const std::string& deviceIP, const std::string& ec);
    bool FnSendParkInParkOutInfo(const std::string& lot_no, 
                                const std::string& lpn, 
                                const std::string& lot_in_image, 
                                const std::string& lot_out_image, 
                                const std::string& lot_in_time,
                                const std::string& lot_out_time);

    Central(Central& central) = delete;

    void operator=(const Central&) = delete;

private:
    static Central* central_;
    std::string centralServerIp;
    Poco::UInt16 centralServerPort;
    Central();
    bool doSendHeartBeatUpdate(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response);
    bool doSendDeviceStatusUpdate(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response, const std::string& deviceIP, const std::string& ec);
    bool doSendParkInParkOutInfo(Poco::Net::HTTPClientSession& session, 
                                Poco::Net::HTTPRequest& request, 
                                Poco::Net::HTTPResponse& response,
                                const std::string& lot_no, 
                                const std::string& lpn, 
                                const std::string& lot_in_image, 
                                const std::string& lot_out_image, 
                                const std::string& lot_in_time,
                                const std::string& lot_out_time);
};