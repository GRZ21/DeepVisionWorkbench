//
// Created by GRZ on 2026/7/10.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ImportExperimentDialog.h" resolved

#include "ImportExperimentDialog.h"

#include "CsvAnalyzeWidget.h"
#include "ui_ImportExperimentDialog.h"
#include "database/DatabaseManager.h"
#include <QFile>

ImportExperimentDialog::ImportExperimentDialog(CsvAnalyzeWidget *csvWidget,QWidget *parent) : QDialog(parent), ui(new Ui::ImportExperimentDialog) {
    ui->setupUi(this);

    QFile file(":/styles/ImportExperimentDialog.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
    }

    QMap<QString, QString> expRecord = csvWidget->csvInfo();
    ui->editExperimentName->setText(expRecord.value("name"));
    ui->editCsvPath->setText(expRecord.value("csvPath"));
    ui->spinBestEpoch->setValue(expRecord.value("bestEpoch").toInt());
    ui->spinPrecision->setValue(expRecord.value("maxP").toDouble());
    ui->spinRecall->setValue(expRecord.value("maxR").toDouble());
    ui->spinMap50->setValue(expRecord.value("maxMap50").toDouble());
    ui->spinMap5095->setValue(expRecord.value("maxMap5095").toDouble());
}

ImportExperimentDialog::~ImportExperimentDialog() {
    delete ui;
}

ExperimentRecord ImportExperimentDialog::getRecord() {
    ExperimentRecord record;
    record.name = ui->editExperimentName->text();
    record.csvPath = ui->editCsvPath->text();
    record.bestEpoch = ui->spinBestEpoch->value();
    record.precision = ui->spinPrecision->value();
    record.recall = ui->spinRecall->value();
    record.bestMap50 = ui->spinMap50->value();
    record.bestMap5095 = ui->spinMap5095->value();
    record.params = ui->spinParams->value();
    record.GFLOPs = ui->spinGflops->value();
    record.note = ui->plainTextNote->toPlainText();
    record.createdTime = QFileInfo(record.csvPath).birthTime().toString("yyyy-MM-dd hh:mm:ss");
    record.updatedTime = QFileInfo(record.csvPath).lastModified().toString("yyyy-MM-dd hh:mm:ss");
    return record;
}
