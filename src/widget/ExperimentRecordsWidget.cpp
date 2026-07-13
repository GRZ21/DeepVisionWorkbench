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
#include "xlsxdocument.h"
#include "xlsxformat.h"

ExperimentRecordsWidget::ExperimentRecordsWidget(QWidget *parent) : QWidget(parent),
                                                                    ui(new Ui::ExperimentRecordsWidget) {
    ui->setupUi(this);

    ui->editDetailName->setReadOnly(false);
    ui->editDetailCsvPath->setReadOnly(true);
    ui->editBestEpoch->setReadOnly(true);
    ui->editBestMap50->setReadOnly(true);
    ui->editBestMap5095->setReadOnly(true);
    ui->editParams->setReadOnly(false);
    ui->editGFLOPs->setReadOnly(false);
    ui->editCreateTime->setReadOnly(true);
    ui->editUpdateTime->setReadOnly(true);
    ui->plainNote->setReadOnly(false);

    ui->tableExperimentRecords->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableExperimentRecords->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableExperimentRecords->setAlternatingRowColors(true);
    ui->tableExperimentRecords->horizontalHeader()->setStretchLastSection(true);

    ui->splitter->setSizes(QList<int>({600,200}));

    QIcon icon(":/icons/sqlite.svg");
    ui->iconDatabaseLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->iconDatabaseLabel->setPixmap(icon.pixmap(16,16));

    QIcon icon2(":/icons/status.svg");
    ui->iconDatabaseStatus->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->iconDatabaseStatus->setPixmap(icon2.pixmap(16,16));

    QIcon icon3(":/icons/search.svg");
    ui->iconSearch->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->iconSearch->setPixmap(icon3.pixmap(24,24));
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
    tableModel->setHeaderData(tableModel->fieldIndex("best_epoch"), Qt::Horizontal, "最佳轮次");
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
    mapper->toFirst();

    ui->tableExperimentRecords->setColumnHidden(tableModel->fieldIndex("id"), true);
    ui->tableExperimentRecords->setColumnHidden(tableModel->fieldIndex("created_time"), true);
    ui->tableExperimentRecords->setColumnHidden(tableModel->fieldIndex("updated_time"), true);

    connect(ui->tableExperimentRecords->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ExperimentRecordsWidget::do_currentRowChanged);

    ui->btnSubmit->setEnabled(selectionModel->hasSelection());
    ui->btnRevert->setEnabled(selectionModel->hasSelection());

    updateStatusBar();
}

void ExperimentRecordsWidget::setCsvAnalyzeWidget(CsvAnalyzeWidget *csvWidget) {
    m_csvWidget = csvWidget;
}

void ExperimentRecordsWidget::updateStatusBar() {
    ui->labelConnectStatus->setText("状态：已连接");
    ui->labelRecordStatus->setText("记录数：" + QString::number(tableModel->rowCount()));
}

// 导入当前实验
void ExperimentRecordsWidget::on_btnImportCurrentExperiment_clicked() {
    if (!m_csvWidget) {
        QMessageBox::warning(this, "提示", "无法获取 CSV 分析器");
        return;
    }
    QMap<QString,QString> info = m_csvWidget->csvInfo();
    if (info.value("csvPath").isEmpty())    // 检查 CSV 路径是否为空
        return;
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

// 提交本次修改
void ExperimentRecordsWidget::on_btnSubmit_clicked() {
    QSqlRecord record = tableModel->record(selectionModel->currentIndex().row());
    recordsStack.push(record);
    ExperimentRecord experimentRecord;
    experimentRecord.name = ui->editDetailName->text();
    experimentRecord.params = ui->editParams->text().toDouble();
    experimentRecord.GFLOPs = ui->editGFLOPs->text().toDouble();
    experimentRecord.note = ui->plainNote->toPlainText();

    if (record.value("name").toString()==experimentRecord.name&&
        record.value("params").toDouble()==experimentRecord.params&&
        record.value("GFLOPs").toDouble()==experimentRecord.GFLOPs&&
        record.value("note").toString()==experimentRecord.note) {
        recordsStack.pop();
        QMessageBox::warning(this, "提示", "没有进行任何修改");
    }

    DatabaseManager& db = DatabaseManager::instance();
    db.updateExperiment(record.value("id").toInt(), experimentRecord);
    tableModel->select();
    ui->tableExperimentRecords->selectRow(selectionModel->currentIndex().row());
}

void ExperimentRecordsWidget::on_btnRevert_clicked() {
    QSqlRecord record;
    if (!recordsStack.isEmpty()) {
        record = recordsStack.pop();
    }else {
        QMessageBox::warning(this, "提示", "没有可撤销的记录");
        return;
    }
    ExperimentRecord experimentRecord;
    experimentRecord.name = record.value("name").toString();
    experimentRecord.params = record.value("params").toDouble();
    experimentRecord.GFLOPs = record.value("GFLOPs").toDouble();
    experimentRecord.note = record.value("note").toString();
    DatabaseManager& db = DatabaseManager::instance();
    db.updateExperiment(record.value("id").toInt(), experimentRecord);
    tableModel->select();
    for (int i = 0; i < tableModel->rowCount(); i++) {
        if (tableModel->record(i).value("id").toInt() == record.value("id").toInt()) {
            ui->tableExperimentRecords->selectRow(i);
            break;
        }
    }
}

void ExperimentRecordsWidget::on_btnExportRecord_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "保存文件", "experiment_records.xlsx", "Xlsx Files (*.xlsx);");
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".xlsx", Qt::CaseInsensitive))
        fileName += ".xlsx";

    if (tableModel->rowCount() == 0) {
        QMessageBox::information(this, "提示", "当前没有可导出的记录");
        return;
    }

    QXlsx::Document xlsx;
    QSqlRecord record = tableModel->record();
    for (int i = 0;i<record.count();i++) {
        if (record.value(i)=="ID")
            continue;
        xlsx.write(1, i+1, tableModel->headerData(i, Qt::Horizontal).toString());
    }

    for (int i = 0;i<tableModel->rowCount();i++) {
        record = tableModel->record(i);
        for (int j = 0;j<record.count();j++) {
            if (j==0)
                continue;
            xlsx.write(i+2, j+1, record.value(j));
        }
    }
    if (xlsx.saveAs(fileName))
        QMessageBox::information(this, "提示", "保存成功");
    else
        QMessageBox::warning(this, "提示", "保存失败");
}


void ExperimentRecordsWidget::on_comboSortExperiment_currentTextChanged(const QString &text) {
    QString sortKey;
    QString sortType = ui->comboBoxSortType->currentText();

    if (text.contains("95"))
        sortKey = "best_map5095";
    else if (text.contains("mAP@50"))
        sortKey = "best_map50";
    else if (text.contains("实验名称"))
        sortKey = "name";
    else if (text.contains("更新时间"))
        sortKey = "updated_time";

    if (sortType == "降序")
        tableModel->setSort(tableModel->fieldIndex(sortKey), Qt::DescendingOrder);
    else if (sortType == "升序")
        tableModel->setSort(tableModel->fieldIndex(sortKey), Qt::AscendingOrder);
    tableModel->select();
}

void ExperimentRecordsWidget::on_comboBoxSortType_currentTextChanged(const QString &text) {
    QString sortKey = ui->comboSortExperiment->currentText();
    QString sortType = text;

    if (sortKey.contains("95"))
        sortKey = "best_map5095";
    else if (sortKey.contains("mAP@50"))
        sortKey = "best_map50";
    else if (sortKey.contains("实验名称"))
        sortKey = "name";
    else if (sortKey.contains("更新时间"))
        sortKey = "updated_time";

    if (sortType == "降序")
        tableModel->setSort(tableModel->fieldIndex(sortKey), Qt::DescendingOrder);
    else if (sortType == "升序")
        tableModel->setSort(tableModel->fieldIndex(sortKey), Qt::AscendingOrder);
    tableModel->select();
}

void ExperimentRecordsWidget::on_editSearchExperiment_textChanged(const QString &text) {
    QString keyword = text;
    keyword.replace("'", "''");

    tableModel->setFilter(
        QString("name LIKE '%%1%' OR csv_path LIKE '%%1%' OR note LIKE '%%1%'")
            .arg(keyword)
    );
    tableModel->select();
}

// 当前行改变时显示记录详情
void ExperimentRecordsWidget::do_currentRowChanged(const QModelIndex &current, const QModelIndex &previous) {
    Q_UNUSED(previous);
    QSqlRecord record = tableModel->record(current.row());
    displayRecordDetails(record);
    ui->btnSubmit->setEnabled(true);
    ui->btnRevert->setEnabled(true);
}

// 显示记录详情
void ExperimentRecordsWidget::displayRecordDetails(QSqlRecord record) {
    ui->editDetailName->setText(record.value("name").toString());
    ui->editDetailCsvPath->setText(record.value("csv_path").toString());
    ui->editBestEpoch->setText(record.value("best_epoch").toString());
    ui->editBestMap50->setText(record.value("best_map50").toString());
    ui->editBestMap5095->setText(record.value("best_map5095").toString());
    ui->editParams->setText(record.value("params").toString());
    ui->editGFLOPs->setText(record.value("GFlops").toString());
    ui->editCreateTime->setText(record.value("created_time").toString());
    ui->editUpdateTime->setText(record.value("updated_time").toString());
    ui->plainNote->setPlainText(record.value("note").toString());
}



