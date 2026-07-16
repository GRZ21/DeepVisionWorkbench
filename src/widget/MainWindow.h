//
// Created by GRZ.
//

#ifndef CSVREADER_MAINWINDOW_H
#define CSVREADER_MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QTreeWidgetItem>
#include <QMap>

#include "ExperimentRecordsWidget.h"
#include "CsvAnalyzeWidget.h"
#include "../utils/qcustomplot.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class CsvAnalyzeWidget;
class ExperimentRecordsWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:
    static void onActionGuide();
    void onActionAbout();
private:
    Ui::MainWindow *ui;
    CsvAnalyzeWidget* csvAnalyzeWidget = nullptr;
    ExperimentRecordsWidget* experimentRecordsWidget = nullptr;
};


#endif //CSVREADER_MAINWINDOW_H
