#pragma once

#include <iostream>
#include <memory>
#include "Poco/Timer.h"
#include "Poco/Thread.h"

class EvtTimer
{

public:
    static EvtTimer* getInstance();
    void FnStartFilterTimer();
    void FnStopFilterTimer();

    EvtTimer(EvtTimer& evtTimer) = delete;

    void operator=(const EvtTimer&) = delete;

private:
    static EvtTimer* evtTimer_;
    std::unique_ptr<Poco::Timer> pFilterTimer_;
    std::unique_ptr<Poco::Timer> pStartUpProcessTimer_;
    EvtTimer();
    void onFilterTimerTimeout(Poco::Timer& timer);
};