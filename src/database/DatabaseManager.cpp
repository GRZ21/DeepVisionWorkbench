//
// Created by GRZ on 2026/7/9.
//

#include "DatabaseManager.h"
#include <QFileDialog>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

QString DatabaseManager::initDatabase() {
    QString dirPath = QDir::currentPath();
    QStringList filters;
    filters << "*.db3"<<"*.db";
    bool existingDB = QDir(dirPath).entryList(filters).isEmpty();
    if (existingDB) {
        dbName = QFileDialog::getOpenFileName(nullptr, "Open Database", "", "SQLite Database (*.db3 *.db)");
    }else {
        db = QSqlDatabase::addDatabase("QSQLITE");
        dbName = "experiments.db";
        db.setDatabaseName(dbName);
        if (!db.open()) {
            qDebug() << "Failed to open database:" << db.lastError().text();
            dbName = "";
            return dbName;
        }
        QSqlQuery query(db);
        bool ok = query.exec(R"(
        CREATE TABLE IF NOT EXISTS experiments (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            csv_path TEXT NOT NULL UNIQUE,
            best_epoch INTEGER,
            precision_value REAL,
            recall_value REAL,
            best_map50 REAL,
            best_map5095 REAL,
            params REAL,
            GFLOPs REAL,
            created_time TEXT DEFAULT (datetime('now', 'localtime')),
            updated_time TEXT DEFAULT (datetime('now', 'localtime')),
            note TEXT
        )
    )");
        if (ok) {
            qDebug() << "Table created successfully";
            dbName = "experiments.db";
        }
        else {
            qDebug() << "Failed to create table";
            dbName = "";
        }
    }
    return dbName;
}

bool DatabaseManager::openDatabase() {
    if (dbName.isEmpty())
        return false;
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbName);
    }
    if (!db.open())
        return false;
    return true;
}

bool DatabaseManager::deleteExperiment(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM experiments WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Failed to delete experiment:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::insertExperiment(const ExperimentRecord &record) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO experiments "
                            "(name, csv_path, best_epoch, "
                            "precision_value, recall_value, best_map50, "
                            "best_map5095, params, GFLOPs, "
                            "created_time, updated_time, note) "
                            "VALUES (:name, :csv_path, :best_epoch,"
                            " :precision_value, :recall_value, :best_map50,"
                            " :best_map5095, :params, :GFLOPs, "
                            ":created_time, :updated_time, :note)");
    query.bindValue(":name", record.name);
    query.bindValue(":csv_path", record.csvPath);
    query.bindValue(":best_epoch", record.bestEpoch);
    query.bindValue(":precision_value", record.precision);
    query.bindValue(":recall_value", record.recall);
    query.bindValue(":best_map50", record.bestMap50);
    query.bindValue(":best_map5095", record.bestMap5095);
    query.bindValue(":params", record.params);
    query.bindValue(":GFLOPs", record.GFLOPs);
    query.bindValue(":created_time", record.createdTime);
    query.bindValue(":updated_time", record.updatedTime);
    query.bindValue(":note", record.note);
    if (!query.exec()) {
        qDebug() << "Failed to insert experiment:" << query.lastError().text();
        return false;
    }else {
        query.exec("SELECT * FROM experiments");
    }
    return true;
}

bool DatabaseManager::selectAllExperiments() {
    QSqlQuery query(db);
    bool ok = query.exec("SELECT * FROM experiments");
    if (ok) {
        qDebug() << "Selected experiments successfully";
        return true;
    }
    qDebug() << "Failed to select experiments";
    return false;
}


