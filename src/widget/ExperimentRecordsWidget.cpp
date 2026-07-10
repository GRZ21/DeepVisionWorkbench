//
// Created by GRZ on 2026/7/9.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ExperimentLibraryWidget.h" resolved

#include "ExperimentRecordsWidget.h"
#include "ui_ExperimentRecordsWidget.h"

#include <QFileDialog>
#include <QDataWidgetMapper>
#include <QSqlError>
#include <ui_ImportExperimentDialog.h>
#include <QSqlRecord>
#include "CsvAnalyzeWidget.h"
#include "ImportExperimentDialog.h"
#include "database/DatabaseManager.h"

ExperimentRecordsWidget::ExperimentRecordsWidget(QWidget *parent) : QWidget(parent),
                                                                    ui(new Ui::ExperimentRecordsWidget) {
    ui->setupUi(this);

    ui->editDetailCsvPath->setReadOnly(true);
    ui->editBestEpoch->setReadOnly(true);
    ui->editBestMap50->setReadOnly(true);
    ui->editBestMap5095->setReadOnly(true);
    ui->editCreateTime->setReadOnly(true);
    ui->editUpdateTime->setReadOnly(true);

    ui->tableExperimentRecords->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableExperimentRecords->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableExperimentRecords->setAlternatingRowColors(true);
}

ExperimentRecordsWidget::~ExperimentRecordsWidget() {
    delete ui;
}

void ExperimentRecordsWidget::initTable() {
    tableModel = new QSqlTableModel(this);
    tableModel->setTable("experiments");
    tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    tableModel->setSort(tableModel->fieldIndex("id"), Qt::AscendingOrder);
    if (tableModel->select()) {
        qDebug()<<"Open experiment records table successfully";
    }else {
        qDebug()<<"Open experiment records table failed"<<tableModel->lastError().text();
        return;
    }

    tableModel->setHeaderData(tableModel->fieldIndex("id"), Qt::Horizontal, "ID");
    tableModel->setHeaderData(tableModel->fieldIndex("name"), Qt::Horizontal, "实验名称");
    tableModel->setHeaderData(tableModel->fieldIndex("csv_path"), Qt::Horizontal, "csv路径");
    tableModel->setHeaderData(tableModel->fieldIndex("best_epoch"), Qt::Horizontal, "最高轮次");
    tableModel->setHeaderData(tableModel->fieldIndex("precision_value"), Qt::Horizontal, "precision");
    tableModel->setHeaderData(tableModel->fieldIndex("recall_value"), Qt::Horizontal, "recall");
    tableModel->setHeaderData(tableModel->fieldIndex("best_map50"), Qt::Horizontal, "最高mAP@50");
    tableModel->setHeaderData(tableModel->fieldIndex("best_map5095"), Qt::Horizontal, "最高mAP@50-95");
    tableModel->setHeaderData(tableModel->fieldIndex("params"), Qt::Horizontal, "参数量");
    tableModel->setHeaderData(tableModel->fieldIndex("GFlops"), Qt::Horizontal, "计算量");
    tableModel->setHeaderData(tableModel->fieldIndex("created_time"), Qt::Horizontal, "创建时间");
    tableModel->setHeaderData(tableModel->fieldIndex("updated_time"), Qt::Horizontal, "更新时间");
    tableModel->setHeaderData(tableModel->fieldIndex("note"), Qt::Horizontal, "备注");

    ui->tableExperimentRecords->setModel(tableModel);

    selectionModel = new QItemSelectionModel(tableModel, this);
    ui->tableExperimentRecords->setSelectionModel(selectionModel);

    mapper = new QDataWidgetMapper(this);
    mapper->setModel(tableModel);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->addMapping(ui->editDetailName, tableModel->fieldIndex("name"));
    mapper->addMapping(ui->editDetailCsvPath, tableModel->fieldIndex("csv_path"));
    mapper->addMapping(ui->editBestEpoch, tableModel->fieldIndex("best_epoch"));
    mapper->addMapping(ui->editBestMap5095, tableModel->fieldIndex("best_map5095"));
    mapper->addMapping(ui->editCreateTime, tableModel->fieldIndex("created_time"));
    mapper->addMapping(ui->editUpdateTime, tableModel->fieldIndex("updated_time"));
    mapper->addMapping(ui->plainNote, tableModel->fieldIndex("note"));

    ui->btnDeleteRecord->setEnabled(true);
    ui->btnEditNote->setEnabled(true);
    ui->btnUpdateRecord->setEnabled(true);

    updateStatusBar();
}

void ExperimentRecordsWidget::setCsvAnalyzeWidget(CsvAnalyzeWidget *csvWidget) {
    m_csvWidget = csvWidget;
}

void ExperimentRecordsWidget::updateStatusBar() {
    ui->labelConnectStatus->setText("已连接");
    ui->labelRecordStatus->setText("记录数：" + QString::number(tableModel->rowCount()));
}

// 自动保存
void ExperimentRecordsWidget::on_btnAutoSave_clicked() {
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    ui->btnAutoSave->setEnabled(false);
}

// 导入当前实验
void ExperimentRecordsWidget::on_btnImportCurrentExperiment_clicked() {
    if (!m_csvWidget) {
        QMessageBox::warning(this, "提示", "无法获取 CSV 分析器");
        return;
    }
    ImportExperimentDialog *dialog = new ImportExperimentDialog(m_csvWidget,this);
    int result = dialog->exec();
    if (result==QDialog::Accepted) {
        ExperimentRecord record = dialog->getRecord();
        DatabaseManager& db = DatabaseManager::instance();
        db.insertExperiment(record);
        tableModel->select();
    }
    delete dialog;
    updateStatusBar();
}

// 刷新记录
void ExperimentRecordsWidget::on_btnRefreshRecord_clicked() {
    DatabaseManager& db = DatabaseManager::instance();
    db.selectAllExperiments();
    tableModel->select();
    updateStatusBar();
}

// 删除记录
void ExperimentRecordsWidget::on_btnDeleteRecord_clicked() {
    int row = selectionModel->currentIndex().row();
    QSqlRecord record = tableModel->record(row);
    int id = record.value("id").toInt();
    if (QMessageBox::question(this, "提示", "是否删除该记录？") == QMessageBox::Yes) {
        qDebug() << "Deleting record with id:" << id;
        DatabaseManager& db = DatabaseManager::instance();
        db.deleteExperiment(id);
        tableModel->select();
    }
    updateStatusBar();
}



