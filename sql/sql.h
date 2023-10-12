#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include "log4qt/log.h"

class LoggerSql {
public:

    enum LogLevel{
        eINFO,
        eWARN,
        eERROR,
    };


    LoggerSql(const QString& dbName) {

        // 构造函数中初始化数据库连接
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbName);
        if (!db.open()) {
            LogManager::instance().getLogger()->error("Database Connection Error: " + db.lastError().text());
        }
        // 在构造函数中创建日志表
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS logs (id INTEGER PRIMARY KEY, timestamp DATETIME, level TEXT, message TEXT)");
        if (query.lastError().isValid()) {
            // 表创建失败，记录错误信息
            LogManager::instance().getLogger()->error("Table Creation Error: " + query.lastError().text());
        }
    }

    ~LoggerSql() {
        // 析构函数中关闭数据库连接
        db.close();
    }



    bool log(LogLevel level, const QString& message) {
        QSqlQuery query;
        query.prepare("INSERT INTO logs (timestamp, level, message) VALUES (?, ?, ?)");
        query.addBindValue(QDateTime::currentDateTime());
        query.addBindValue(logLevelToString(level));
        query.addBindValue(message);
        bool success = query.exec();
        if(!success) LogManager::instance().getLogger()->error("Log Insertion Error: " + query.lastError().text());
        return success;
    }

    QList<QVariantMap> getLogs(LogLevel level = LogLevel::eINFO) {
        QList<QVariantMap> logs;
        QSqlQuery query;
        QString queryString = "SELECT * FROM logs";
        if (level != LogLevel::eINFO) {
            queryString += " WHERE level = ?";
            query.prepare(queryString);
            query.addBindValue(logLevelToString(level));
        } else {
            query.exec(queryString);
        }
        if (query.exec()) {
            while (query.next()) {
                QVariantMap log;
                log["id"] = query.value(0);
                log["timestamp"] = query.value(1);
                log["level"] = query.value(2);
                log["message"] = query.value(3);
                logs.append(log);
            }
        } else {
            LogManager::instance().getLogger()->error("Log Query Error: " + query.lastError().text());
        }
        return logs;
    }

private:
    QSqlDatabase db;

    QString logLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::eINFO:
                return "INFO";
            case LogLevel::eWARN:
                return "WARN";
            case LogLevel::eERROR:
                return "ERROR";
        }
    }
};
