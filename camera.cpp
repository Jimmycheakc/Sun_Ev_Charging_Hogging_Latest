#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "camera.h"
#include "central.h"
#include "database.h"
#include "event.h"
#include "ini_parser.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPDigestCredentials.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/MultipartReader.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/NumberParser.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/NullStream.h"
#include "common.h"
#include "log.h"
#include "timer.h"

Camera* Camera::camera_ = nullptr;
Poco::Mutex Camera::singletonCameraMutex_;

Camera::Camera()
{
    cameraRecoveryFlag_ = false;
    cameraStatus_ = false;
    cameraServerIP = Iniparser::getInstance()->FnGetCameraIP();
    createImageDirectory();
}

Camera* Camera::getInstance()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(singletonCameraMutex_);

    if (camera_ == nullptr)
    {
        camera_ = new Camera();
    }

    return camera_;
}

void Camera::createImageDirectory()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(cameraMutex_);

    AppLogger::getInstance()->FnLog("Creating image directory.");
    Poco::File imageDirectory(imageDirectoryPath);

    try
    {
        if (!imageDirectory.exists())
        {
            imageDirectory.createDirectories();
        }

        imageDirectory.setWriteable(true);
    }
    catch(const Poco::Exception& ex)
    {
        std::cerr << "Error creating image directory : " << ex.displayText() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
}

bool Camera::isImageDirectoryExists()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(cameraMutex_);

    Poco::File imageDirectory(imageDirectoryPath);
    return imageDirectory.exists();
}

void Camera::FnCameraInit()
{
    cameraStatus_ = FnGetHeartBeat();
    FnSetCurrentTime();
}

void Camera::FnSetCameraStatus(bool status)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(cameraMutex_);

    cameraStatus_ = status;
}

bool Camera::FnGetCameraStatus()
{
    return cameraStatus_;
}

bool Camera::do_heartBeatRequest(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response)
{
    AppLogger::getInstance()->FnLog(request.getURI());

    session.sendRequest(request);
    std::istream& rs = session.receiveResponse(response);

    // Log the response header
    std::ostringstream msg;
    msg << "Status : " << response.getStatus() << " Reason : " << response.getReason();
    AppLogger::getInstance()->FnLog(msg.str());

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
    {
        std::ostringstream responseStream;
        Poco::StreamCopier::copyStream(rs, responseStream);
        AppLogger::getInstance()->FnLog(responseStream.str());

        return true;
    }
    else
    {
        Poco::NullOutputStream null;
        Poco::StreamCopier::copyStream(rs, null);
        return false;
    }
}

bool Camera::FnGetHeartBeat()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(cameraMutex_);

    bool ret = false;
    int retry = 0;
    int maxRetries = 3;
    const std::string uri_link= "http://" + cameraServerIP + "/cgi-bin/trafficParking.cgi?action=getAllParkingSpaceStatus";

    std::ostringstream msgReq;
    msgReq << __func__ << " : Get " << uri_link;
    AppLogger::getInstance()->FnLog(msgReq.str());

    try
    {
        Poco::URI uri(uri_link);
        std::string path(uri.getPathAndQuery());
        if (path.empty())
        {
            path = "/";
        }
        Poco::Net::HTTPDigestCredentials credentials(username, password);
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
        Poco::Net::HTTPResponse response;

        do
        {
            if (!do_heartBeatRequest(session, request, response))
            {
                credentials.authenticate(request, response);
                if (!do_heartBeatRequest(session, request, response))
                {
                    AppLogger::getInstance()->FnLog("Authentication : Failed, Invalid username or password");
                    retry++;
                }
                else
                {
                    std::ostringstream msgSuccess;
                    msgSuccess << "Get " << uri_link << " : Succeed";
                    AppLogger::getInstance()->FnLog(msgSuccess.str());
                    ret = true;
                    break;
                }
            }
            else
            {
                std::ostringstream msgFail;
                msgFail << "Get " << uri_link << " : Failed, Retries = " << retry;
                AppLogger::getInstance()->FnLog(msgFail.str());
                retry++;
            }
        } while (retry <= maxRetries);
    }
    catch (Poco::Exception& ex)
    {
        AppLogger::getInstance()->FnLog(ex.displayText());
    }

    return ret;
}

bool Camera::do_subscribeToSnapShot(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response)
{
    session.sendRequest(request);
    std::istream& rs = session.receiveResponse(response);
    
    // Log the response header
    std::ostringstream msg;
    msg << "Status : " << response.getStatus() << " Reason : " << response.getReason();
    AppLogger::getInstance()->FnLog(msg.str());
    
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
    {
        std::string contentType = response.get("Content-Type", "");
        std::string boundary;
        Poco::Net::NameValueCollection params;

        Poco::Net::MessageHeader::splitParameters(contentType.begin(), contentType.end(), params);
        boundary = params.get("boundary", "");
        Poco::Net::MultipartReader reader(rs, boundary);
        event_t event = {};

        while (reader.hasNextPart())
        {
            Poco::Net::MessageHeader partHeader;
            reader.nextPart(partHeader);

            std::string ct = partHeader.get("Content-Type", "");
            std::string cl = partHeader.get("Content-Length", "");

            if (!cl.empty() && !ct.empty())
            {
                Poco::Int64 contentLength = std::stoi(cl);
                if (ct == "text/plain") {
                    // Process text/plain part
                    std::string partData;
                    Poco::StreamCopier::copyToString(reader.stream(), partData, contentLength);
                    std::istringstream partStream(partData);
                    std::string line;
                    event = {};

                    AppLogger::getInstance()->FnLog("Camera Response : text/plain");

                    while (std::getline(partStream, line))
                    {
                        if (line.find("Heartbeat") != std::string::npos)
                        {
                            AppLogger::getInstance()->FnLog(line);
                            Camera::getInstance()->FnSetCameraStatus(true);
                            EvtTimer::getInstance()->FnRestartCameraHeartbeatTimer();
                        }
                        else if (line.find("].Channel") != std::string::npos)
                        {
                            size_t equalSignPos = line.find("=");
                            if (equalSignPos != std::string::npos)
                            {
                                event.evt_channel = line.substr(equalSignPos+1, line.length() - equalSignPos - 2);
                            }
                            AppLogger::getInstance()->FnLog(line);
                        }
                        else if (line.find("].ParkingSpaceStatus") != std::string::npos)
                        {
                            size_t equalSignPos = line.find("=");
                            if (equalSignPos != std::string::npos)
                            {
                                event.evt_parking_status = line.substr(equalSignPos+1, line.length() - equalSignPos - 2);
                            }
                            AppLogger::getInstance()->FnLog(line);
                        }
                        else if (line.find("].TrafficCar.PlateNumber") != std::string::npos)
                        {
                            size_t equalSignPos = line.find("=");
                            if (equalSignPos != std::string::npos)
                            {
                                event.evt_lpn = line.substr(equalSignPos+1, line.length() - equalSignPos - 2);
                            }
                            AppLogger::getInstance()->FnLog(line);
                        }
                        else if (line.find("].TrafficCar.UTC") != std::string::npos)
                        {
                            size_t equalSignPos = line.find("=");
                            if (equalSignPos != std::string::npos)
                            {
                                event.evt_snapshot_time = line.substr(equalSignPos+1, line.length() - equalSignPos - 2);
                            }
                            AppLogger::getInstance()->FnLog(line);
                        }
                        else if (line.find("].TrafficCar.Lane") != std::string::npos)
                        {
                            size_t equalSignPos = line.find("=");
                            if (equalSignPos != std::string::npos)
                            {
                                event.evt_lane = line.substr(equalSignPos+1, line.length() - equalSignPos - 2);
                            }
                            AppLogger::getInstance()->FnLog(line);
                        }
			            else if (line.find("].TrafficCar.CustomParkNo") != std::string::npos)
                        {
                            size_t equalSignPos = line.find("=");
                            if (equalSignPos != std::string::npos)
                            {
                                event.evt_lot_no = line.substr(equalSignPos+1, line.length() - equalSignPos - 2);
                            }
                            AppLogger::getInstance()->FnLog(line);
                        }
                        else if (line.find("].TrafficCar.Event") != std::string::npos)
                        {
                            size_t equalSignPos = line.find("=");
                            if (equalSignPos != std::string::npos)
                            {
                                event.evt_type = line.substr(equalSignPos+1, line.length() - equalSignPos - 2);
                            }
                            AppLogger::getInstance()->FnLog(line);
                        }
                    }
                }
                else if (ct == "image/jpeg")
                {
                    try
                    {
                        AppLogger::getInstance()->FnLog("Camera Response : image/jpeg");
                        // Save the image
                        std::string absImagePath;
                        std::string dateTimeImageTime;

                        if (!event.evt_snapshot_time.empty())
                        {
                            try
                            {
                                std::string UTCStr = event.evt_snapshot_time;
                                Poco::trimInPlace(UTCStr);
                                Poco::Int64 intUTC = Poco::NumberParser::parse(UTCStr);
                                Poco::DateTime dateTimeUTC(Poco::Timestamp::fromEpochTime(intUTC));
                                std::string dateTimeUTCStr(Poco::DateTimeFormatter::format(dateTimeUTC, "%y%m%d_%H%M%S"));
                                std::string dateTimeUTCStr2(Poco::DateTimeFormatter::format(dateTimeUTC, "%Y-%m-%d %H:%M:%S"));
                                dateTimeImageTime = dateTimeUTCStr2;
                                absImagePath = imageDirectoryPath + "/Img_" + dateTimeUTCStr + ".jpg";
                            }
                            catch(const Poco::Exception& ex)
                            {
                                std::cerr << "Error Parsing the string as integer : " << ex.displayText() << std::endl;
                            }
                        }
                        else
                        {
                            absImagePath = imageDirectoryPath + "/Img_" + Common::getInstance()->FnFormatDateYYMMDD_HHMMSS() + ".jpg";
                            AppLogger::getInstance()->FnLog("Date for filename is get from current time.");
                        }

                        if (!isImageDirectoryExists())
                        {
                            createImageDirectory();
                        }
                        AppLogger::getInstance()->FnLog("Image stored :" + absImagePath);
                        Poco::FileOutputStream fileStream(absImagePath);
                        Poco::StreamCopier::copyStream(reader.stream(), fileStream, contentLength);
                        fileStream.close();

                        if (event.evt_type.compare("TrafficParkingSpaceNoParking") == 0)
                        {
                            Database::parking_lot_t db_parking_lot;
                            db_parking_lot.location_code = Iniparser::getInstance()->FnGetParkingLotLocationCode();
                            db_parking_lot.lot_no = event.evt_lane;
                            db_parking_lot.custom_park_lot_no = event.evt_lot_no;
                            db_parking_lot.lpn = event.evt_lpn;
                            db_parking_lot.lot_in_image_path = "";
                            db_parking_lot.lot_out_image_path = absImagePath;
                            db_parking_lot.lot_in_dt = "";
                            db_parking_lot.lot_out_dt = dateTimeImageTime;
                            db_parking_lot.add_dt = "";
                            db_parking_lot.update_dt = "";
                            db_parking_lot.lot_in_central_sent_dt = "";
                            db_parking_lot.lot_out_central_sent_dt = "";

                            AppLogger::getInstance()->FnLog("Trigger park out event.");
                            ParkingLotOut_DBEvent* parkingLotOut_DBEvent = new ParkingLotOut_DBEvent(db_parking_lot);
                            EventManager::getInstance()->FnEnqueueEvent(parkingLotOut_DBEvent);

                        }
                        else if (event.evt_type.compare("TrafficParkingSpaceParking") == 0)
                        {
                            Database::parking_lot_t db_parking_lot;
                            db_parking_lot.location_code = Iniparser::getInstance()->FnGetParkingLotLocationCode();
                            db_parking_lot.lot_no = event.evt_lane;
                            db_parking_lot.custom_park_lot_no = event.evt_lot_no;
                            db_parking_lot.lpn = event.evt_lpn;
                            db_parking_lot.lot_in_image_path = absImagePath;
                            db_parking_lot.lot_out_image_path = "";
                            db_parking_lot.lot_in_dt = dateTimeImageTime;
                            db_parking_lot.lot_out_dt = "";
                            db_parking_lot.add_dt = "";
                            db_parking_lot.update_dt = "";
                            db_parking_lot.lot_in_central_sent_dt = "";
                            db_parking_lot.lot_out_central_sent_dt = "";

                            AppLogger::getInstance()->FnLog("Trigger park in event.");
                            ParkingLotIn_DBEvent* parkingLotIn_DBEvent = new ParkingLotIn_DBEvent(db_parking_lot);
                            EventManager::getInstance()->FnEnqueueEvent(parkingLotIn_DBEvent);
                        }

                    }
                    catch(Poco::Exception& ex)
                    {
                        std::cerr << "Error saving image: " << ex.displayText() << std::endl;
                    }
                }
            }
        }

        session.reset();
        return true;
    }
    else
    {
        Poco::NullOutputStream null;
        Poco::StreamCopier::copyStream(rs, null);
        return false;
    }
}

bool Camera::FnSubscribeToSnapShot()
{
    // Take note: Cannot use mutex here, as its function is blocking function
    // Local scope lock
    //Poco::Mutex::ScopedLock lock(cameraMutex_);

    bool ret = false;
    int retry = 0;
    int maxRetries = 3;
    const std::string uri_link= "http://" + cameraServerIP + "/cgi-bin/snapManager.cgi?action=attachFileProc&channel=1&heartbeat=60&Flags[0]=Event&Events=[TrafficParkingSpaceParking%2CTrafficParkingSpaceNoParking]";

    std::ostringstream msgReq;
    msgReq << __func__ << " : Get " << uri_link;
    AppLogger::getInstance()->FnLog(msgReq.str());

    try
    {
        Poco::URI uri(uri_link);
        std::string path(uri.getPathAndQuery());
        if (path.empty())
        {
            path = "/";
        }
        Poco::Net::HTTPDigestCredentials credentials(username, password);
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
        Poco::Net::HTTPResponse response;

        do
        {
            if (!do_subscribeToSnapShot(session, request, response))
            {
                credentials.authenticate(request, response);
                if (!do_subscribeToSnapShot(session, request, response))
                {
                    AppLogger::getInstance()->FnLog("Authentication : Failed, Invalid username or password");
                    retry++;
                }
                else
                {
                    std::ostringstream msgSuccess;
                    msgSuccess << "Get " << uri_link << " : Succeed";
                    AppLogger::getInstance()->FnLog(msgSuccess.str());
                    ret = true;
                    break;
                }
            }
            else
            {
                std::ostringstream msgFail;
                msgFail << "GET " << uri_link << " : Failed, Retries = " << retry;
                AppLogger::getInstance()->FnLog(msgFail.str());
                retry++;
            }

        } while(retry <= maxRetries);
    }
    catch (Poco::Exception& ex)
    {
        AppLogger::getInstance()->FnLog(ex.displayText());
    }

    return ret;
}

bool Camera::do_getCurrentTime(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response, std::string& dateTime)
{
    session.sendRequest(request);
    std::istream& rs = session.receiveResponse(response);
    
    // Log the response header
    std::ostringstream msg;
    msg << "Status : " << response.getStatus() << " Reason : " << response.getReason();
    AppLogger::getInstance()->FnLog(msg.str());
    
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
    {
        std::ostringstream responseStream;
        Poco::StreamCopier::copyStream(rs, responseStream);
        dateTime = responseStream.str();
        AppLogger::getInstance()->FnLog(responseStream.str());

        return true;
    }
    else
    {
        Poco::NullOutputStream null;
        Poco::StreamCopier::copyStream(rs, null);
        return false;
    }
}

bool Camera::FnGetCurrentTime(std::string& dateTime)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(cameraMutex_);

    bool ret = false;
    int retry = 0;
    int maxRetries = 3;
    const std::string uri_link= "http://" + cameraServerIP + "/cgi-bin/global.cgi?action=getCurrentTime";

    std::ostringstream msgReq;
    msgReq << __func__ << " : Get " << uri_link;
    AppLogger::getInstance()->FnLog(msgReq.str());

    try
    {
        Poco::URI uri(uri_link);
        std::string path(uri.getPathAndQuery());
        if (path.empty())
        {
            path = "/";
        }
        Poco::Net::HTTPDigestCredentials credentials(username, password);
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
        Poco::Net::HTTPResponse response;

        do
        {
            if (!do_getCurrentTime(session, request, response, dateTime))
            {
                credentials.authenticate(request, response);
                if (!do_getCurrentTime(session, request, response, dateTime))
                {
                    AppLogger::getInstance()->FnLog("Authentication : Failed, Invalid username or password");
                    retry++;
                }
                else
                {
                    std::ostringstream msgSuccess;
                    msgSuccess << "Get " << uri_link << " : Succeed";
                    AppLogger::getInstance()->FnLog(msgSuccess.str());
                    ret = true;
                    break;
                }
            }
            else
            {
                std::ostringstream msgFail;
                msgFail << "GET " << uri_link << " : Failed, Retries = " << retry;
                AppLogger::getInstance()->FnLog(msgFail.str());
                retry++;
            }

        } while(retry <= maxRetries);
    }
    catch (Poco::Exception& ex)
    {
        AppLogger::getInstance()->FnLog(ex.displayText());
    }

    return ret;
}

bool Camera::do_setCurrentTime(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response)
{
    session.sendRequest(request);
    std::istream& rs = session.receiveResponse(response);
    
    // Log the response header
    std::ostringstream msg;
    msg << "Status : " << response.getStatus() << " Reason : " << response.getReason();
    AppLogger::getInstance()->FnLog(msg.str());
    
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
    {
        std::ostringstream responseStream;
        Poco::StreamCopier::copyStream(rs, responseStream);
        AppLogger::getInstance()->FnLog(responseStream.str());
        
        if (responseStream.str().compare("OK"))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        Poco::NullOutputStream null;
        Poco::StreamCopier::copyStream(rs, null);
        return false;
    }
}

bool Camera::FnSetCurrentTime()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(cameraMutex_);

    bool ret = false;
    int retry = 0;
    int maxRetries = 3;
    Poco::LocalDateTime now;
    std::string dateTimeStr(Poco::DateTimeFormatter::format(now, "%Y-%n-%e%%20%H:%M:%S"));

    const std::string uri_link= "http://" + cameraServerIP + "/cgi-bin/global.cgi?action=setCurrentTime&time=" + dateTimeStr;
    std::ostringstream msgReq;
    msgReq << __func__ << " : Get " << uri_link;
    AppLogger::getInstance()->FnLog(msgReq.str());

    try
    {
        Poco::URI uri(uri_link);
        std::string path(uri.getPathAndQuery());
        if (path.empty())
        {
            path = "/";
        }
        Poco::Net::HTTPDigestCredentials credentials(username, password);
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
        Poco::Net::HTTPResponse response;

        do
        {
            if (!do_setCurrentTime(session, request, response))
            {
                credentials.authenticate(request, response);
                if (!do_setCurrentTime(session, request, response))
                {
                    AppLogger::getInstance()->FnLog("Authentication : Failed, Invalid username or password");
                    retry++;
                }
                else
                {
                    std::ostringstream msgSuccess;
                    msgSuccess << "Get " << uri_link << " : Succeed";
                    AppLogger::getInstance()->FnLog(msgSuccess.str());
                    ret = true;
                    break;
                }
            }
            else
            {
                std::ostringstream msgFail;
                msgFail << "GET " << uri_link << " : Failed, Retries = " << retry;
                AppLogger::getInstance()->FnLog(msgFail.str());
                retry++;
            }

        } while(retry <= maxRetries);
    }
    catch (Poco::Exception& ex)
    {
        AppLogger::getInstance()->FnLog(ex.displayText());
    }

    return ret;
}

void Camera::FnSetCameraRecoveryFlag(bool flag)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(cameraMutex_);

    cameraRecoveryFlag_ = flag;
}

bool Camera::FnGetCameraRecoveryFlag()
{
    return cameraRecoveryFlag_;
}
