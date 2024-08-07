#include <iostream>
#include <string>
#include <sstream>
#include "central.h"
#include "common.h"
#include "ini_parser.h"
#include "log.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Exception.h"
#include "Poco/URI.h"
#include "Poco/StreamCopier.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/StreamCopier.h"
#include "Poco/NullStream.h"

Central* Central::central_ = nullptr;
Poco::Mutex Central::singletonCentralMutex_;
const std::string Central::ERROR_CODE_RECOVERED = "0";
const std::string Central::ERROR_CODE_CAMERA = "1";
const std::string Central::ERROR_CODE_IPC = "2";

Central::Central()
{
    centralServerIp = Iniparser::getInstance()->FnGetCentralIP();
    centralServerPort = Iniparser::getInstance()->FnGetCentralServerPort();
    centralStatus_ = false;
}

Central* Central::getInstance()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(singletonCentralMutex_);

    if (central_ == nullptr)
    {
        central_ = new Central();
    }
    return central_;
}

bool Central::doSendHeartBeatUpdate(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response)
{
    AppLogger::getInstance()->FnLog(request.getURI());

    request.setContentType("application/json");

    Poco::JSON::Object::Ptr jsonObject = new Poco::JSON::Object;
    jsonObject->set("username", username);
    jsonObject->set("password", password);
    jsonObject->set("carpark_code", Iniparser::getInstance()->FnGetParkingLotLocationCode());
    jsonObject->set("device_ip", Common::getInstance()->FnGetIpAddress());
    jsonObject->set("heartbeat_dt", Common::getInstance()->FnCurrentFormatDateYYYY_MM_DD_HH_MM_SS());
    jsonObject->set("msg", "Hearbeat Update");

    std::ostringstream jsonStringStream;
    jsonObject->stringify(jsonStringStream);
    request.setContentLength(jsonStringStream.str().length());

    // Send the request
    std::ostream& requestStream = session.sendRequest(request);
    requestStream << jsonStringStream.str();

    // Receive the response
    std::istream& responseStream = session.receiveResponse(response);

    // Log the response header
    std::ostringstream msg;
    msg << "Status : " << response.getStatus() << " Reason : " << response.getReason();
    AppLogger::getInstance()->FnLog(msg.str());

    if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK)
    {
        std::ostringstream rs;
        Poco::StreamCopier::copyStream(responseStream, rs);
        AppLogger::getInstance()->FnLog(rs.str());

        return true;
    }
    else
    {
        Poco::NullOutputStream null;
        Poco::StreamCopier::copyStream(responseStream, null);

        return false;
    }
}

bool Central::FnSendHeartBeatUpdate()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(centralMutex_);

    const std::string uri_link = "http://" + centralServerIp;
    bool ret = false;

    try
    {
        Poco::URI uri(uri_link);
        std::string path(uri.getPathAndQuery());
        if (path.empty())
        {
            path = "/";
        }
        
        Poco::Net::HTTPClientSession session(uri.getHost(), centralServerPort);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/HeartBeat", Poco::Net::HTTPMessage::HTTP_1_1);
        Poco::Net::HTTPResponse response;

        int retry = 0;

        if (!doSendHeartBeatUpdate(session, request, response))
        {
            retry = 3;

            while (retry > 0)
            {
                if (doSendHeartBeatUpdate(session, request, response))
                {
                    ret = true;
                    break;
                }
                else
                {
                    retry--;
                }
            }
        }
        else
        {
            ret = true;
        }

    }
    catch (Poco::Exception& ex)
    {
        AppLogger::getInstance()->FnLog(ex.displayText());
        ret = false;
    }
    
    return ret;
}

bool Central::doSendDeviceStatusUpdate(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response, const std::string& location_code, const std::string& deviceIP, const std::string& ec)
{
    AppLogger ::getInstance()->FnLog(request.getURI());

    request.setContentType("application/json");

    Poco::JSON::Object::Ptr jsonObject = new Poco::JSON::Object;
    jsonObject->set("username", username);
    jsonObject->set("password", password);
    jsonObject->set("carpark_code", location_code);
    jsonObject->set("device_ip", deviceIP);
    jsonObject->set("error_code", ec);
    
    std::ostringstream jsonStringStream;
    jsonObject->stringify(jsonStringStream);
    request.setContentLength(jsonStringStream.str().length());

    // Send the request
    std::ostream& requestStream = session.sendRequest(request);
    requestStream << jsonStringStream.str();

    // Receive the response
    std::istream& responseStream = session.receiveResponse(response);

    // Log the response header
    std::ostringstream msg;
    msg << "Status : " << response.getStatus() << " Reason : " << response.getReason();
    AppLogger::getInstance()->FnLog(msg.str());

    if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK)
    {
        std::ostringstream rs;
        Poco::StreamCopier::copyStream(responseStream, rs);
        AppLogger::getInstance()->FnLog(rs.str());

        return true;
    }
    else
    {
        Poco::NullOutputStream null;
        Poco::StreamCopier::copyStream(responseStream, null);

        return false;
    }
}

bool Central::FnSendDeviceStatusUpdate(const std::string& location_code, const std::string& deviceIP, const std::string& ec)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(centralMutex_);

    if (!centralStatus_)
    {
        std::ostringstream msgFail;
        msgFail << __func__ << " Failed, Central Status : " << centralStatus_;
        AppLogger::getInstance()->FnLog(msgFail.str());
        return false;
    }

    const std::string uri_link = "http://" + centralServerIp;
    bool ret = false;

    try
    {
        Poco::URI uri(uri_link);
        std::string path(uri.getPathAndQuery());
        if (path.empty())
        {
            path = "/";
        }
        
        Poco::Net::HTTPClientSession session(uri.getHost(), centralServerPort);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/DeviceStatus", Poco::Net::HTTPMessage::HTTP_1_1);
        Poco::Net::HTTPResponse response;

        int retry = 0;
        
        if (!doSendDeviceStatusUpdate(session, request, response, location_code, deviceIP, ec))
        {
            retry = 3;

            while (retry > 0)
            {
                if (doSendDeviceStatusUpdate(session, request, response, location_code, deviceIP, ec))
                {
                    ret = true;
                    break;
                }
                else
                {
                    retry--;
                }
            }
        }
        else
        {
            ret = true;
        }
    }
    catch (Poco::Exception& ex)
    {
        AppLogger::getInstance()->FnLog(ex.displayText());
        ret = false;
    }
    
    return ret;
}

bool Central::doSendParkInParkOutInfo(Poco::Net::HTTPClientSession& session, 
                                Poco::Net::HTTPRequest& request, 
                                Poco::Net::HTTPResponse& response,
                                const std::string& lot_no, 
                                const std::string& lpn, 
                                const std::string& lot_in_image, 
                                const std::string& lot_out_image, 
                                const std::string& lot_in_time,
                                const std::string& lot_out_time)
{
    AppLogger::getInstance()->FnLog(request.getURI());

    request.setContentType("application/json");

    auto jsonObject = std::make_unique<Poco::JSON::Object>();
    jsonObject->set("username", username);
    jsonObject->set("password", password);
    jsonObject->set("carpark_code", Iniparser::getInstance()->FnGetParkingLotLocationCode());
    // Temp: Need to revisit
    jsonObject->set("lot_no", lot_no);
    jsonObject->set("lpn", lpn);
    jsonObject->set("lot_in_image", lot_in_image);
    jsonObject->set("lot_out_image", lot_out_image);
    jsonObject->set("lot_in_time", lot_in_time);
    jsonObject->set("lot_out_time", lot_out_time);

    std::ostringstream jsonStringStream;
    jsonObject->stringify(jsonStringStream);
    request.setContentLength(jsonStringStream.str().length());

    // Send the request
    std::ostream& requestStream = session.sendRequest(request);
    requestStream << jsonStringStream.str();

    try
    {
        // Receive the response
        std::istream& responseStream = session.receiveResponse(response);

        // Log the response header
        std::ostringstream msg;
        msg << "Status : " << response.getStatus() << " Reason : " << response.getReason();
        AppLogger::getInstance()->FnLog(msg.str());

        if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK)
        {
            std::ostringstream rs;
            Poco::StreamCopier::copyStream(responseStream, rs);
            AppLogger::getInstance()->FnLog(rs.str());

            return true;
        }
        else
        {
            Poco::NullOutputStream null;
            Poco::StreamCopier::copyStream(responseStream, null);

            return false;
        }
    }
    catch (Poco::TimeoutException& timeoutEx)
    {
        // Log timeout error
        AppLogger::getInstance()->FnLog("Timeout occurred while waiting for response.");

        // Handle timeout error here, such as retrying the request or returning false
        return false;
    }
}

bool Central::FnSendParkInParkOutInfo(const std::string& lot_no, 
                                const std::string& lpn, 
                                const std::string& lot_in_image, 
                                const std::string& lot_out_image, 
                                const std::string& lot_in_time,
                                const std::string& lot_out_time)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(centralMutex_);

    if (!centralStatus_)
    {
        std::ostringstream msgFail;
        msgFail << __func__ << " Failed, Central Status : " << centralStatus_;
        AppLogger::getInstance()->FnLog(msgFail.str());
        return false;
    }

    const std::string uri_link = "http://" + centralServerIp;
    bool ret = false;

    try
    {
        Poco::URI uri(uri_link);
        std::string path(uri.getPathAndQuery());
        if (path.empty())
        {
            path = "/";
        }
        
        Poco::Net::HTTPClientSession session(uri.getHost(), centralServerPort);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/ParkInOut", Poco::Net::HTTPMessage::HTTP_1_1);
        Poco::Net::HTTPResponse response;

        int retry = 0;
        
        if (!doSendParkInParkOutInfo(session, request, response, lot_no, lpn, lot_in_image, lot_out_image, lot_in_time, lot_out_time))
        {
            retry = 3;

            while (retry > 0)
            {
                if (doSendParkInParkOutInfo(session, request, response, lot_no, lpn, lot_in_image, lot_out_image, lot_in_time, lot_out_time))
                {
                    ret = true;
                    break;
                }
                else
                {
                    retry--;
                }
            }
        }
        else
        {
            ret = true;
        }

    }
    catch (Poco::Exception& ex)
    {
        AppLogger::getInstance()->FnLog(ex.displayText());
        ret = false;
    }
    
    return ret;
}

void Central::FnSetCentralStatus(bool status)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(centralMutex_);

    centralStatus_ = status;
}

bool Central::FnGetCentralStatus()
{
    return centralStatus_;
}
