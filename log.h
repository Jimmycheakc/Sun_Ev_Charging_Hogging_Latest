#pragma once

#include <iostream>
#include <string>
#include "Poco/AutoPtr.h"
#include "Poco/Logger.h"
#include "Poco/File.h"
#include "Poco/Path.h"

class AppLogger
{
public:
    const std::string logFilePath = Poco::Path::home() + "Desktop/Ev_Charging_Hogging_Log";

    static AppLogger* getInstance();
    void FnLog(const std::string& msg);

    AppLogger(AppLogger& logger) = delete;

    void operator=(const AppLogger&) = delete;

private:
    static AppLogger* logger_;
    static Poco::Mutex singletonLogMutex_;
    Poco::AutoPtr<Poco::Logger> pocoLogger_;
    Poco::Mutex logMutex_;
    AppLogger();
    ~AppLogger();
    void createLogFile();
    bool isLogFileExists();
};