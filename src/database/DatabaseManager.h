//
// Created by GRZ on 2026/7/9.
//

#ifndef CSVREADER_DATAMANAGER_H
#define CSVREADER_DATAMANAGER_H
#include <QString>
#include <QSqlDatabase>


struct ExperimentRecord
{
    QString name;
    QString csvPath;

    int bestEpoch = -1;
    double precision = 0.0;
    double recall = 0.0;
    double bestMap50 = 0.0;
    double bestMap5095 = 0.0;
    double params = 0.0;
    double GFLOPs = 0.0;
    QString createdTime, updatedTime;
    QString note;
};

class DatabaseManager {
public:
    static DatabaseManager& instance();

    QString initDatabase();

    bool openDatabase();

    bool deleteExperiment(int id);
    bool insertExperiment(const ExperimentRecord& record);
    bool updateExperiment(int id, const ExperimentRecord& record);
    int experimentCount(int currentPage, int pageSize);

private:
    DatabaseManager() = default;

private:
    QSqlDatabase db;
    QString dbName;
};


#endif //CSVREADER_DATAMANAGER_H
