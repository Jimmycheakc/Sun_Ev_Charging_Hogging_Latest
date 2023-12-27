#include "common.h"
#include "log.h"
#include "Poco/AutoPtr.h"
#include "Poco/Exception.h"
#include "Poco/File.h"
#include "Poco/FileChannel.h"
#include "Poco/FormattingChannel.h"
#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"

AppLogger* AppLogger::logger_ = nullptr;
Poco::Mutex AppLogger::singletonLogMutex_;

AppLogger::AppLogger()
{
    createLogFile();
}

AppLogger::~AppLogger()
{
    pocoLogger_->shutdown();
}

AppLogger* AppLogger::getInstance()
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(singletonLogMutex_);

    if (logger_ == nullptr)
    {
        logger_ = new AppLogger();
    }
    return logger_;
}

void AppLogger::createLogFile()
{
    Poco::File logDirectory(logFilePath);

    try
    {
        if (!logDirectory.exists())
        {
            logDirectory.createDirectories();
        }

        logDirectory.setWriteable(true);
    }
    catch(const Poco::Exception& ex)
    {
        std::cerr << "Error creating log directory : " << ex.displayText() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    Poco::AutoPtr<Poco::FileChannel> pChannel(new Poco::FileChannel);
    std::string absFilePath = logFilePath + "/ev_charging_hogging_" + Common::getInstance()->FnFormatDateYYMMDD() + ".log";
    pChannel->setProperty("path", absFilePath);
    //pChannel->setProperty("rotation", "1 M");

    Poco::AutoPtr<Poco::PatternFormatter> pPF(new Poco::PatternFormatter);
    pPF->setProperty("times", "local");
    pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S %s: %t");

    Poco::AutoPtr<Poco::FormattingChannel> pFC(new Poco::FormattingChannel(pPF, pChannel));
    pocoLogger_ = &Poco::Logger::get("EvChargingHoggingLogger");
    pocoLogger_->setChannel(pFC);

    try
    {
        pChannel->open();
    }
    catch(const Poco::Exception& ex)
    {
        std::cerr << "Error opening log file: " << ex.displayText() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
}

bool AppLogger::isLogFileExists()
{
    std::string absFilePath = logFilePath + "/ev_charging_hogging_" + Common::getInstance()->FnFormatDateYYMMDD() + ".log";
    Poco::File logFile(absFilePath);
    return logFile.exists();
}

void AppLogger::FnLog(const std::string& msg)
{
    // Local scope lock
    Poco::Mutex::ScopedLock lock(logMutex_);

    if (!isLogFileExists())
    {
        createLogFile();
        pocoLogger_->information("Recreation log file.");
    }

    pocoLogger_->information(msg);
}