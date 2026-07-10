//
// Created by GRZ on 2026/7/9.
//

#ifndef CSVREADER_EXPERIMENTRECORDSWIDGET_H
#define CSVREADER_EXPERIMENTRECORDSWIDGET_H

#include <QDataWidgetMapper>
#include <QItemSelectionModel>
#include <QWidget>

#include <QSqlTableModel>

#include "CsvAnalyzeWidget.h"

QT_BEGIN_NAMESPACE

class CsvAnalyzeWidget;

namespace Ui {
    class ExperimentRecordsWidget;
}

QT_END_NAMESPACE

class ExperimentRecordsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ExperimentRecordsWidget(QWidget *parent = nullptr);

    ~ExperimentRecordsWidget() override;

    void setCsvAnalyzeWidget(CsvAnalyzeWidget *csvWidget);

public:
    void initTable();   // 初始化表格
    void updateStatusBar();   // 更新状态栏

private slots:
    void on_btnAutoSave_clicked();
    void on_btnImportCurrentExperiment_clicked();
    void on_btnRefreshRecord_clicked();
    void on_btnDeleteRecord_clicked();
private:
    QSqlTableModel *tableModel;
    QItemSelectionModel *selectionModel;
    QDataWidgetMapper *mapper;
    CsvAnalyzeWidget *m_csvWidget = nullptr;
private:
    Ui::ExperimentRecordsWidget *ui;
};


#endif //CSVREADER_EXPERIMENTRECORDSWIDGET_H
