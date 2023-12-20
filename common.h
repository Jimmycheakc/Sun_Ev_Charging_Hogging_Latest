#pragma once

#include <iostream>
#include <string>

class Common
{
public:
    static Common* getInstance();
    std::string FnFormatDateYYMMDD();
    std::string FnFormatDateYYMMDD_HHMMSS();
    std::string FnCurrentFormatDateYYYY_MM_DD_HH_MM_SS();
    std::string FnConverImageToBase64String(const std::string& imagePath);

    Common(Common& common) = delete;

    void operator=(const Common&) = delete;

private:
    static Common* common_;
    Common();
};