#ifndef LOG_H
#define LOG_H

// log4qt
#include "log4qt/logger.h"
#include "log4qt/log4qt.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/basicconfigurator.h"
#include "log4qt/logmanager.h"

#define LOG4Qt_FILEPATH  "./log.ini"

class LogManager
{
public:
    static LogManager& instance()
    {
        static LogManager _instance;
        return _instance;
    }

    Log4Qt::Logger* getLogger()
    {
        return logger;
    }

private:
    LogManager()
    {
        Log4Qt::PropertyConfigurator::configure(LOG4Qt_FILEPATH);
        //    Log4Qt::LogManager::setHandleQtMessages(true);
        logger = Log4Qt::Logger::rootLogger();
    }

    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    Log4Qt::Logger* logger;
};

#endif // LOG_H
