#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "common.h"
#include "Poco/Base64Encoder.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Net/NetworkInterface.h"
#include "Poco/Net/IPAddress.h"
#include "log.h"

Common* Common::common_ = nullptr;
Poco::Mutex Common::singletonCommonMutex_;

Common::Common()
{
    ipAddr_ = "127.0.0.1";
}

Common* Common::getInstance()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(singletonCommonMutex_);

    if (common_ == nullptr)
    {
        common_ = new Common();
    }

    return common_;
}

std::string Common::FnFormatDateYYMMDD()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(commonMutex_);

    Poco::LocalDateTime now;
    std::string dateTimeStr(Poco::DateTimeFormatter::format(now, "%y%m%d"));
    return dateTimeStr;
}

std::string Common::FnFormatDateYYMMDD_HHMMSS()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(commonMutex_);

    Poco::LocalDateTime now;
    std::string dateTimeStr(Poco::DateTimeFormatter::format(now, "%y%m%d_%H%M%S"));
    return dateTimeStr;
}

std::string Common::FnCurrentFormatDateYYYY_MM_DD_HH_MM_SS()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(commonMutex_);

    Poco::LocalDateTime now;
    std::string dateTimeStr(Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S"));
    return dateTimeStr;
}

std::string Common::FnConverImageToBase64String(const std::string& imagePath)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(commonMutex_);

    std::ifstream imageFile(imagePath, std::ios::binary | std::ios::ate);

    if (!imageFile.is_open())
    {
        std::ostringstream msg;
        msg << "Error opening image file: " << imagePath << std::endl;
        AppLogger::getInstance()->FnLog(msg.str());
        return "";
    }

    std::streamsize size = imageFile.tellg();
    imageFile.seekg(0, std::ios::beg);

    std::string buffer(size, ' ');
    if (!imageFile.read(&buffer[0], size))
    {
        std::ostringstream msg;
        msg << "Error reading image file: " << imagePath << std::endl;
        AppLogger::getInstance()->FnLog(msg.str());
        return "";
    }
    
    imageFile.close();

    // Use Poco's Base64Encoder to encode the binary data
    std::ostringstream encoded;
    Poco::Base64Encoder encoder(encoded);
    encoder << buffer;
    encoder.close();

    return encoded.str();
}

void Common::FnRetrieveIpAddressFromNetInterface()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(commonMutex_);

    try
    {
        Poco::Net::NetworkInterface interfaces = Poco::Net::NetworkInterface::forName("eth0", Poco::Net::NetworkInterface::IPv4_ONLY);

        ipAddr_ = interfaces.address().toString();
    }
    catch(Poco::Exception& ex)
    {
        AppLogger::getInstance()->FnLog(ex.displayText());
    }
}

std::string Common::FnGetIpAddress()
{
    return ipAddr_;
}
