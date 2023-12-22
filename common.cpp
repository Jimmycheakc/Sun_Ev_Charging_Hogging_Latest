#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "common.h"
#include "Poco/Base64Encoder.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "log.h"

Common* Common::common_ = nullptr;

Common::Common()
{
}

Common* Common::getInstance()
{
    if (common_ == nullptr)
    {
        common_ = new Common();
    }

    return common_;
}

std::string Common::FnFormatDateYYMMDD()
{
    Poco::LocalDateTime now;
    std::string dateTimeStr(Poco::DateTimeFormatter::format(now, "%y%m%d"));
    return dateTimeStr;
}

std::string Common::FnFormatDateYYMMDD_HHMMSS()
{
    Poco::LocalDateTime now;
    std::string dateTimeStr(Poco::DateTimeFormatter::format(now, "%y%m%d_%H%M%S"));
    return dateTimeStr;
}

std::string Common::FnCurrentFormatDateYYYY_MM_DD_HH_MM_SS()
{
    Poco::LocalDateTime now;
    std::string dateTimeStr(Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%S"));
    return dateTimeStr;
}

std::string Common::FnConverImageToBase64String(const std::string& imagePath)
{
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