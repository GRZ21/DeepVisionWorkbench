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
    void on_btnImportCurrentExperiment_clicked();
    void on_btnRefreshRecord_clicked();
    void on_btnDeleteRecord_clicked();
    void on_btnSubmit_clicked();
    void on_btnRevert_clicked();
    void on_btnExportRecord_clicked();
    void on_comboSortExperiment_currentTextChanged(const QString& text);
    void on_comboBoxSortType_currentTextChanged(const QString& text);
    void on_editSearchExperiment_textChanged(const QString& text);
    void do_currentRowChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    QSqlTableModel *tableModel;
    QItemSelectionModel *selectionModel;
    QDataWidgetMapper *mapper;
    CsvAnalyzeWidget *m_csvWidget = nullptr;
    QStack<QSqlRecord> recordsStack;

    void displayRecordDetails(QSqlRecord record);

private:
    Ui::ExperimentRecordsWidget *ui;
};


#endif //CSVREADER_EXPERIMENTRECORDSWIDGET_H
