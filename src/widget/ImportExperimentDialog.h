//
// Created by GRZ on 2026/7/10.
//

#ifndef CSVREADER_IMPORTEXPERIMENTDIALOG_H
#define CSVREADER_IMPORTEXPERIMENTDIALOG_H

#include <QDialog>

#include "CsvAnalyzeWidget.h"
#include "database/DatabaseManager.h"


QT_BEGIN_NAMESPACE

namespace Ui {
    class ImportExperimentDialog;
}

QT_END_NAMESPACE

class ImportExperimentDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImportExperimentDialog(CsvAnalyzeWidget *csvWidget,QWidget *parent = nullptr);

    ~ImportExperimentDialog() override;

    ExperimentRecord getRecord();

private:
    Ui::ImportExperimentDialog *ui;
};


#endif //CSVREADER_IMPORTEXPERIMENTDIALOG_H
