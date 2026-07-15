//
// Created by GRZ on 2026/7/9.
//

// You may need to build the project (run Qt uic code generator) to get "ui_CsvAnalyzeWidget.h" resolved

#include "CsvAnalyzeWidget.h"
#include "ui_CsvAnalyzeWidget.h"


#include <QFileDialog>
#include <QString>
#include <QColorDialog>
#include <ui_MainWindow.h>

#include "../utils/qcustomplot.h"


CsvAnalyzeWidget::CsvAnalyzeWidget(QWidget *parent) : QWidget(parent), ui(new Ui::CsvAnalyzeWidget) {
    ui->setupUi(this);

    QFile file(":/styles/CsvAnalyzeWidget.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
    }

    tableModel = new QStandardItemModel(this);
    tableSelectionModel = new QItemSelectionModel(tableModel);
    ui->tableView->setModel(tableModel);
    ui->tableView->setSelectionModel(tableSelectionModel);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 界面大小
    ui->Vsplitter->setSizes(QList<int>({700,300}));
    ui->Hsplitter->setChildrenCollapsible(false);

    ui->treeWidget->setMinimumWidth(200);
    ui->rightPanelWidget->setMinimumWidth(330);

    ui->Hsplitter->setStretchFactor(0, 0); // 文件树
    ui->Hsplitter->setStretchFactor(1, 1); // 图表区域优先伸缩
    ui->Hsplitter->setStretchFactor(2, 0); // 属性面板保持稳定

    ui->Hsplitter->setSizes({350, 950, 300});

    // 表头相关
    ui->treeWidget->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);

    // 自定义右键策略
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    // 图相关
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->legend->setBrush(QBrush(QColor(255,255,255,150)));
    ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom|QCP::iSelectPlottables);

    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scrollArea->setFrameShape(QFrame::NoFrame);

    connect(ui->treeWidget,&QTreeWidget::itemClicked,this,&CsvAnalyzeWidget::onTreeItemClicked);
    connect(ui->treeWidget,&QTreeWidget::itemDoubleClicked,this,&CsvAnalyzeWidget::onTreeItemDoubleClicked);
    connect(tableSelectionModel,&QItemSelectionModel::selectionChanged,this,
        [&](const QItemSelection& selected,const QItemSelection& deselected) {
            Q_UNUSED(deselected);
            if (!selected.isEmpty()) {
                QModelIndex index = selected.indexes().first();
                highlightTableParam(index);
            }
        });
    connect(ui->treeWidget,&QTreeWidget::itemChanged,this,&CsvAnalyzeWidget::onParmItemChanged);
    connect(ui->customPlot->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
    [=](const QCPRange &newRange) {
        if (newRange.lower < 0) {
            ui->customPlot->xAxis->setRangeLower(0);
        }
    });

    connect(ui->customPlot->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
        [=](const QCPRange &newRange) {
            if (newRange.lower < 0)
                ui->customPlot->yAxis->setRangeLower(0);
        });

    connect(ui->customPlot,&QCustomPlot::plottableClick,this,&CsvAnalyzeWidget::onCurveClicked);
    connect(ui->editName,&QLineEdit::textChanged,this,&CsvAnalyzeWidget::onEditName);
    connect(ui->spinWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &CsvAnalyzeWidget::onSpinWidth);
    connect(ui->spinAlpha, QOverload<int>::of(&QSpinBox::valueChanged), this, &CsvAnalyzeWidget::onSpinAlpha);
    connect(ui->btnColor, &QPushButton::clicked, this, &CsvAnalyzeWidget::onPropColorClicked);
    connect(ui->comboStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CsvAnalyzeWidget::onComboStyleChanged);
    connect(ui->comboScatter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CsvAnalyzeWidget::onComboScatterChanged);
    connect(ui->btnBringTop,&QPushButton::clicked,this,&CsvAnalyzeWidget::onBtnBringTopClicked);
    connect(ui->btnSendBottom,&QPushButton::clicked,this,&CsvAnalyzeWidget::onBtnSendBottomClicked);
    connect(ui->comboFont,&QFontComboBox::currentFontChanged,this,&CsvAnalyzeWidget::updateGlobalFont);
    connect(ui->spinFontSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &CsvAnalyzeWidget::updateGlobalFont);
    connect(ui->checkFontBold,&QCheckBox::toggled,this,&CsvAnalyzeWidget::updateGlobalFont);
    connect(ui->comboLegend, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CsvAnalyzeWidget::onComboLegendChanged);
    connect(ui->comboGrid, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CsvAnalyzeWidget::onComboGridChanged);
    connect(ui->editXTitle,&QLineEdit::textChanged,this,&CsvAnalyzeWidget::onAxisTitleChanged);
    connect(ui->editYTitle,&QLineEdit::textChanged,this,&CsvAnalyzeWidget::onAxisTitleChanged);
    connect(ui->spinScatterSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &CsvAnalyzeWidget::onSpinScatterSizeChanged);
    connect(ui->btnResetZoom,&QPushButton::clicked,this,&CsvAnalyzeWidget::autoRescalePlot);
    connect(ui->btnExportImg,&QPushButton::clicked,this,&CsvAnalyzeWidget::exportChart);
    connect(ui->treeWidget,&QTreeWidget::customContextMenuRequested,this,&CsvAnalyzeWidget::onTreeContextMenu);

    buildTreeHeader();
    onComboGridChanged(0);
    onAxisTitleChanged();
    initTree();
    updateGlobalFont();
}


CsvAnalyzeWidget::~CsvAnalyzeWidget() {
    delete ui;
}



void CsvAnalyzeWidget::buildTreeHeader() {
    ui->treeWidget->clear();

    QTreeWidgetItem* header = new QTreeWidgetItem();
    header->setText(CsvAnalyzeWidget::colItem,"文件名");
    header->setText(CsvAnalyzeWidget::colDate,"最后修改日期");

    header->setTextAlignment(CsvAnalyzeWidget::colItem,Qt::AlignHCenter|Qt::AlignVCenter);

    ui->treeWidget->setHeaderItem(header);
}


void CsvAnalyzeWidget::initTree() {
    QTreeWidgetItem* root = new QTreeWidgetItem(CsvAnalyzeWidget::itTopItem);
    root->setText(CsvAnalyzeWidget::colItem,"csv name: ");
    root->setText(CsvAnalyzeWidget::colDate,QDate::currentDate().toString());
    root->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->treeWidget->addTopLevelItem(root);
    ui->treeWidget->setCurrentItem(root);
    root->setExpanded(true);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->treeWidget->header()->resizeSection(1, 120);
    ui->treeWidget->header()->setStretchLastSection(false);
}


QTreeWidgetItem* CsvAnalyzeWidget::addCSVItem(QTreeWidgetItem *parItem, QString fileName) {
    QFileInfo fileInfo(fileName);
    QString lastFileName = fileInfo.fileName();
    QDateTime fileDate = fileInfo.lastModified();

    QTreeWidgetItem* item = new QTreeWidgetItem(CsvAnalyzeWidget::itCSVItem);
    item->setText(CsvAnalyzeWidget::colItem,lastFileName);
    // item->setText(CsvAnalyzeWidget::colDate,fileDate.toString("yyyy-MM-dd HH:mm"));
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    // item->setCheckState(CsvAnalyzeWidget::colItem,Qt::Checked);

    item->setData(CsvAnalyzeWidget::colItem,Qt::UserRole,QVariant(fileName));
    parItem->addChild(item);
    return item;
}


void CsvAnalyzeWidget::addParmItem(QTreeWidgetItem *parItem, QString CSVHeaderParam,int colIndex) {
    QTreeWidgetItem* item = new QTreeWidgetItem(CsvAnalyzeWidget::itParamItem);
    QString trimmedParam = CSVHeaderParam.trimmed();
    item->setText(CsvAnalyzeWidget::colItem,trimmedParam);
    item->setFirstColumnSpanned(true);
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable|Qt::ItemIsAutoTristate);
    item->setCheckState(CsvAnalyzeWidget::colItem,Qt::Unchecked);

    item->setData(CsvAnalyzeWidget::colItem,Qt::UserRole,colIndex);
    parItem->addChild(item);
}


void CsvAnalyzeWidget::loadCSVCache(QString fileName) {
    QStringList data;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            data<<line;
        }
    }
    file.close();
    if (!data.isEmpty()) {
        csvCache[fileName] = data;
        csvTableDisplay(data);
    }
}


void CsvAnalyzeWidget::highlightColumn(int columIndex) {
    if (columIndex<0||columIndex>=tableModel->columnCount())
        return;

    int rowCount = tableModel->rowCount();
    if (rowCount == 0)
        return;

    QModelIndex topLeft = tableModel->index(0, columIndex);
    QModelIndex bottomRight = tableModel->index(rowCount - 1, columIndex);

    QItemSelection selection(topLeft, bottomRight);

    tableSelectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
}


void CsvAnalyzeWidget::highlightTableParam(QModelIndex& Index) {
    if (!Index.isValid())
        return;

    int columnIndex = Index.column();

    QString paramName = tableModel->headerData(columnIndex, Qt::Horizontal).toString().trimmed();

    if (paramName.isEmpty() || currentCSVFileName.isEmpty())
        return;

    QTreeWidgetItem* csvItem = nullptr;
    QList<QTreeWidgetItem*> allItems = ui->treeWidget->findItems(
        QFileInfo(currentCSVFileName).fileName(),
        Qt::MatchExactly | Qt::MatchRecursive
    );

    for (QTreeWidgetItem* item : allItems) {
        if (item->type() == CsvAnalyzeWidget::itCSVItem) {
            QString fileName = item->data(CsvAnalyzeWidget::colItem, Qt::UserRole).toString();
            if (fileName == currentCSVFileName) {
                csvItem = item;
                break;
            }
        }
    }

    if (csvItem == nullptr)
        return;

    for (int i = 0; i < csvItem->childCount(); i++) {
        QTreeWidgetItem* child = csvItem->child(i);
        if (child->type() == CsvAnalyzeWidget::itParamItem &&
            child->data(CsvAnalyzeWidget::colItem, Qt::UserRole).toInt() == columnIndex) {
            ui->treeWidget->setCurrentItem(child);
            csvItem->setExpanded(true);

            ui->customPlot->deselectAll();
            for (int j = 0;j<ui->customPlot->graphCount();j++) {
                QCPGraph* graph = ui->customPlot->graph(j);
                if (curveToItemMap.value(graph, nullptr)== child) {
                    graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
                    curGraph = graph;

                    if (!ui->customPlot->layer("top_layer")) {
                        ui->customPlot->addLayer("top_layer", ui->customPlot->layer("main"), QCustomPlot::limAbove);
                    }
                    if (graph->layer()->name() == "top_layer") {
                        graph->setLayer("main");
                    }
                    graph->setLayer("top_layer");

                    ui->editName->blockSignals(true);
                    ui->spinWidth->blockSignals(true);
                    ui->spinAlpha->blockSignals(true);
                    ui->comboStyle->blockSignals(true);
                    ui->comboScatter->blockSignals(true);
                    ui->spinScatterSize->blockSignals(true);

                    ui->editName->setText(graph->name());
                    ui->spinWidth->setValue(graph->pen().width());
                    ui->spinAlpha->setValue(graph->pen().color().alpha() * 100 / 255);
                    ui->spinScatterSize->setValue(graph->scatterStyle().size());

                    if (graph->pen().style() == Qt::SolidLine) ui->comboStyle->setCurrentIndex(0);
                    else if (graph->pen().style() == Qt::DashLine) ui->comboStyle->setCurrentIndex(1);

                    switch(graph->scatterStyle().shape()) {
                        case QCPScatterStyle::ssNone:     ui->comboScatter->setCurrentIndex(0); break;
                        case QCPScatterStyle::ssCircle:   ui->comboScatter->setCurrentIndex(1); break;
                        case QCPScatterStyle::ssSquare:   ui->comboScatter->setCurrentIndex(2); break;
                        case QCPScatterStyle::ssDiamond:    ui->comboScatter->setCurrentIndex(3); break;
                        case QCPScatterStyle::ssTriangle: ui->comboScatter->setCurrentIndex(4); break;
                        default:                          ui->comboScatter->setCurrentIndex(0); break;
                    }

                    ui->editName->blockSignals(false);
                    ui->spinWidth->blockSignals(false);
                    ui->spinAlpha->blockSignals(false);
                    ui->comboStyle->blockSignals(false);
                    ui->comboScatter->blockSignals(false);
                    ui->spinScatterSize->blockSignals(false);

                    updateCurveStats(graph);

                    break;
                }
            }
            ui->customPlot->replot();
            return;
        }
    }
}

void CsvAnalyzeWidget::poltCsvColumn(QTreeWidgetItem* item,int columnIndex) {
    Q_UNUSED(columnIndex);
    if (!item || item->type() != CsvAnalyzeWidget::itParamItem)
        return;

    QTreeWidgetItem* parItem = item->parent();
    if (!parItem)
        return;

    currentCSVFileName = parItem->data(CsvAnalyzeWidget::colItem, Qt::UserRole).toString();
    QStringList data;
    if (csvCache.contains(currentCSVFileName)) {
        data = csvCache[currentCSVFileName];
    } else {
        return;
    }

    int valueIndex = item->data(CsvAnalyzeWidget::colItem, Qt::UserRole).toInt();

    int epochIndex = -1;
    QStringList header = data.at(0).split(",");
    for (int i = 0; i < header.size(); i++) {
        if (header.at(i).trimmed() == "epoch") {
            epochIndex = i;
            break;
        }
    }

    if (epochIndex < 0 || valueIndex < 0 || valueIndex >= header.size()) return;

    QVector<double> xData, yData;
    for (int i = 1; i < data.size(); i++) {
        QString line = data.at(i);
        QStringList fields = line.split(",");
        if (fields.size() > epochIndex && fields.size() > valueIndex) {
            bool okX, okY;
            double x = fields.at(epochIndex).trimmed().toDouble(&okX);
            double y = fields.at(valueIndex).trimmed().toDouble(&okY);
            if (okX && okY) {
                xData.append(x);
                yData.append(y);
            }
        }
    }


    QString originalHeaderName = header.at(valueIndex).trimmed();
    QString currentItemName = item->text(CsvAnalyzeWidget::colItem);
    QString pureName;
    if (currentItemName == originalHeaderName) {
        pureName = QFileInfo(currentCSVFileName).baseName() + " - " + currentItemName;
    } else {
        pureName = currentItemName;
    }

    if (!xData.isEmpty()) {
        drawOnCustomPlot(pureName, xData, yData, item);
    }
}

void CsvAnalyzeWidget::drawOnCustomPlot(QString name, QVector<double> x, QVector<double> y,QTreeWidgetItem* item) {
    if (curveToItemMap.key(item, nullptr) != nullptr) {
        return;
    }
    QCPGraph* graph = ui->customPlot->addGraph();
    curveToItemMap.insert(graph, item);

    graph->setName(name);

    graph->setData(x,y);
    QPen pen;
    pen.setColor(QColor(0,120,212));
    graph->setPen(pen);

    autoRescalePlot();

    ui->customPlot->setInteractions(QCP::Interaction::iRangeDrag|QCP::Interaction::iRangeZoom|QCP::Interaction::iSelectPlottables);
    ui->customPlot->replot();
}

void CsvAnalyzeWidget::onParmItemChanged(QTreeWidgetItem *item, int columIndex) {
    if (item->type() != CsvAnalyzeWidget::itParamItem)
        return;
    if (item->checkState(columIndex)==Qt::Checked)
        poltCsvColumn(item,columIndex);
    else
        removeCurveFromPlot(item);
}

void CsvAnalyzeWidget::removeCurveFromPlot(QTreeWidgetItem *item) {
    bool hasRemoved = false;
    for (int i = ui->customPlot->graphCount()-1;i>=0;i--) {
        QCPGraph* graph = ui->customPlot->graph(i);
        if (curveToItemMap.value(graph,nullptr)==item) {
            if (curGraph == graph)
                curGraph = nullptr;
            graph->setSelection(QCPDataSelection());
            curveToItemMap.remove(graph);
            ui->customPlot->removeGraph(graph);
            hasRemoved = true;
        }
    }
    if (hasRemoved) {
        autoRescalePlot();
        ui->customPlot->replot();
    }
}

void CsvAnalyzeWidget::updateCurveStats(QCPGraph *graph) {
    if (!graph || graph->data()->isEmpty()) {
        ui->labelMaxValue->setText("--");
        ui->labelMinValue->setText("--");
        ui->labelFinalValue->setText("--");
        return;
    }

    double maxVal = -std::numeric_limits<double>::max();
    double minVal = std::numeric_limits<double>::max();
    double finalVal = 0;

    // 遍历数据点寻找最值
    auto it = graph->data()->constBegin();
    auto itEnd = graph->data()->constEnd();
    for (; it != itEnd; ++it) {
        double val = it->mainValue();
        if (val > maxVal) maxVal = val;
        if (val < minVal) minVal = val;
    }

    finalVal = (itEnd - 1)->mainValue();

    ui->labelMaxValue->setText(QString::number(maxVal, 'f', 5));
    ui->labelMinValue->setText(QString::number(minVal, 'f', 5));
    ui->labelFinalValue->setText(QString::number(finalVal, 'f', 5));
}

void CsvAnalyzeWidget::openCsvFile() {
    QStringList data;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open CSV File"), QDir::currentPath(), tr("CSV Files (*.csv)"));
    if (!fileName.isEmpty()) {
        QTreeWidgetItem* parItem = ui->treeWidget->topLevelItem(0);
        if (!parItem)
            return;

        QTreeWidgetItem* csvItem = addCSVItem(parItem,fileName);
        parItem->setExpanded(true);

        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            QTextStream in(&file);

            if (!in.atEnd()) {
                QString headLine = in.readLine();
                QStringList headerData = headLine.split(",");

                    for (int i = 0; i < headerData.size(); i++) {
                        QString paramName = headerData.at(i).trimmed();
                        if (paramName=="epoch"||paramName=="time")
                            continue;
                        if (paramName.startsWith("x/lr") && paramName != "x/lr0")
                            continue;
                        if (paramName.startsWith("lr/pg")&&paramName != "lr/pg0")
                            continue;
                        addParmItem(csvItem,headerData.at(i),i);
                    }
                data << headLine;
            }

            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty())
                    data<<line;
            }
        }
        file.close();
        csvCache[fileName] = data;
        currentCSVFileName = fileName;  // 记录当前文件名
    }
    csvTableDisplay(data);
}

void CsvAnalyzeWidget::clearPlot() {
    curveToItemMap.clear();
    curGraph = nullptr;

    ui->customPlot->clearGraphs();
    ui->customPlot->replot();

    ui->treeWidget->blockSignals(true);
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        if ((*it)->type() == CsvAnalyzeWidget::itParamItem) {
            (*it)->setCheckState(CsvAnalyzeWidget::colItem, Qt::Unchecked);
        }
        ++it;
    }
    ui->treeWidget->blockSignals(false);
}

void CsvAnalyzeWidget::removeAllFiles() {
    clearPlot();
    csvCache.clear();
    currentCSVFileName.clear();
    for (int i = 0;i<ui->treeWidget->topLevelItemCount();i++) {
        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
        qDeleteAll(item->takeChildren());
    }
    if (tableModel)
        tableModel->clear();
}

void CsvAnalyzeWidget::openCsvDirectory() {
   QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirPath.isEmpty())
        return;

    QDirIterator it(dirPath, QStringList() << "*.csv", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString fileName = it.next();
        if (!fileName.isEmpty()) {
            QStringList data;

            QTreeWidgetItem* parItem = ui->treeWidget->topLevelItem(0);
            if (!parItem)
                return;

            QTreeWidgetItem* csvItem = addCSVItem(parItem, fileName);
            parItem->setExpanded(true);

            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);

                if (!in.atEnd()) {
                    QString headLine = in.readLine();
                    QStringList headerData = headLine.split(",");

                    for (int i = 0; i < headerData.size(); i++) {
                        QString paramName = headerData.at(i).trimmed();
                        if (paramName == "epoch" || paramName == "time")
                            continue;
                        if (paramName.startsWith("x/lr") && paramName != "x/lr0")
                            continue;
                        if (paramName.startsWith("lr/pg") && paramName != "lr/pg0")
                            continue;
                        addParmItem(csvItem, headerData.at(i), i);
                    }
                    data << headLine;
                }

                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    if (!line.isEmpty())
                        data << line;
                }
            }
            file.close();

            csvCache[fileName] = data;
            currentCSVFileName = fileName;
        }
    }

    if (!currentCSVFileName.isEmpty() && csvCache.contains(currentCSVFileName)) {
        csvTableDisplay(csvCache[currentCSVFileName]);
    }
}

void CsvAnalyzeWidget::onTreeContextMenu(const QPoint &pos) {
    QTreeWidgetItem* item = ui->treeWidget->itemAt(pos);
    if (!item)
        return;
    if (item->type() == CsvAnalyzeWidget::itCSVItem) {
        QMenu menu;
        QAction* action = menu.addAction("移除该文件");
        connect(action, &QAction::triggered, [this, item]() {removeSingleCSV(item);});
        menu.exec(ui->treeWidget->mapToGlobal(pos));
    }
}

void CsvAnalyzeWidget::deleteCsv() {
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item)
        return;
    if (item->type() == CsvAnalyzeWidget::itCSVItem)
        removeSingleCSV(item);
    else if (item->type() == CsvAnalyzeWidget::itParamItem)
        removeSingleCSV(item->parent());
}


void CsvAnalyzeWidget::setFileTreeVisible(bool checked) {
    ui->treeWidget->setVisible(checked);
}

void CsvAnalyzeWidget::setDataTableVisible(bool checked) {
    ui->tableView->setVisible(checked);
}

void CsvAnalyzeWidget::setPropertyPanelVisible(bool checked) {
    if (ui->rightPanelWidget) {
        ui->rightPanelWidget->setVisible(checked);
    }
}

void CsvAnalyzeWidget::csvTableDisplay(QStringList& data) {
    if (data.isEmpty())
        return;

    int rowCount = data.length()-1;
    int columnCount = data[0].split(",").size();

    tableModel->clear();
    tableModel->setRowCount(rowCount);
    tableModel->setColumnCount(columnCount);

    QString header = data.at(0);
    QStringList headerData = header.split(",");

    for (int i = 0; i < columnCount; i++) {
        QStandardItem* item = new QStandardItem(headerData.at(i));
        item->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        tableModel->setHorizontalHeaderItem(i,item);
    }
    tableModel->setHorizontalHeaderLabels(headerData);

    QStandardItem* item;
    for (int i = 1; i < data.length(); i++) {
        QString str = data.at(i);
        QStringList rowData = str.split(",");
        for (int j = 0; j < columnCount; j++) {
            item = new QStandardItem(rowData.at(j));
            item->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            tableModel->setItem(i-1, j, item);
        }
    }
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->resizeColumnsToContents();
}

void CsvAnalyzeWidget::onTreeItemClicked(QTreeWidgetItem *item, int column) {
    Q_UNUSED(column);
    if (item== nullptr)
        return;

    if (item->type() == CsvAnalyzeWidget::itCSVItem) {
        QString fileName = item->data(CsvAnalyzeWidget::colItem,Qt::UserRole).toString();
        currentCSVFileName = fileName;  // 记录当前文件名
        if (csvCache.contains(fileName))
            csvTableDisplay(csvCache[fileName]);
        else
            loadCSVCache(fileName);
    }else if (item->type() == CsvAnalyzeWidget::itParamItem) {
        int paramIndex = item->data(CsvAnalyzeWidget::colItem,Qt::UserRole).toInt();

        QTreeWidgetItem* parItem = item->parent();
        currentCSVFileName = parItem->data(CsvAnalyzeWidget::colItem,Qt::UserRole).toString();  // 记录当前文件名
        csvTableDisplay(csvCache[currentCSVFileName]);
        highlightColumn(paramIndex);
    }
}

void CsvAnalyzeWidget::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column) {
    if (item== nullptr)
        return;
    if (item->type()!=CsvAnalyzeWidget::itParamItem)
        return;
    if (item->checkState(CsvAnalyzeWidget::colItem)==Qt::Checked)
        item->setCheckState(column,Qt::Unchecked);
    else
        item->setCheckState(column,Qt::Checked);
}

void CsvAnalyzeWidget::onCurveClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event) {
    Q_UNUSED(dataIndex);
    Q_UNUSED(event);
    curGraph = qobject_cast<QCPGraph*>(plottable);
    if (!curGraph)
        return;
    QTreeWidgetItem* item = curveToItemMap.value(curGraph,nullptr);
    if (item) {
        ui->treeWidget->setCurrentItem(item);
        ui->treeWidget->scrollToItem(item);
        highlightColumn(item->data(CsvAnalyzeWidget::colItem,Qt::UserRole).toInt());
    }

    QString name = curGraph->name();
    QPen pen = curGraph->pen();
    QColor color = pen.color();
    int width = pen.width();
    int alpha = color.alpha()*100/255;

    ui->comboScatter->blockSignals(true);
    ui->editName->blockSignals(true);
    ui->spinWidth->blockSignals(true);
    ui->spinAlpha->blockSignals(true);
    ui->comboStyle->blockSignals(true);

    QCPScatterStyle::ScatterShape shape = curGraph->scatterStyle().shape();
    switch(shape) {
        case QCPScatterStyle::ssNone:     ui->comboScatter->setCurrentIndex(0); break;
        case QCPScatterStyle::ssCircle:   ui->comboScatter->setCurrentIndex(1); break;
        case QCPScatterStyle::ssSquare:   ui->comboScatter->setCurrentIndex(2); break;
        case QCPScatterStyle::ssCross:    ui->comboScatter->setCurrentIndex(3); break;
        case QCPScatterStyle::ssTriangle: ui->comboScatter->setCurrentIndex(4); break;
        default:                          ui->comboScatter->setCurrentIndex(0); break;
    }

    ui->editName->setText(name);
    ui->spinWidth->setValue(width);
    ui->spinAlpha->setValue(alpha);

    if (pen.style() == Qt::SolidLine)
        ui->comboStyle->setCurrentIndex(0);
    else if (pen.style() == Qt::DashLine)
        ui->comboStyle->setCurrentIndex(1);

    ui->editName->blockSignals(false);
    ui->spinWidth->blockSignals(false);
    ui->spinAlpha->blockSignals(false);
    ui->comboStyle->blockSignals(false);
    ui->comboScatter->blockSignals(false);

    updateCurveStats(curGraph);
}

void CsvAnalyzeWidget::autoRescalePlot() {
    if (ui->customPlot->graphCount() == 0) {
        ui->customPlot->xAxis->setRange(0, 1);
        ui->customPlot->yAxis->setRange(0, 1);
        ui->customPlot->replot();

        ui->editName->clear();
        ui->labelMaxValue->setText("--");
        ui->labelMinValue->setText("--");
        ui->labelFinalValue->setText("--");
        return;
    }
    ui->customPlot->rescaleAxes();

    double xUpper = ui->customPlot->xAxis->range().upper;
    double yUpper = ui->customPlot->yAxis->range().upper;

    ui->customPlot->xAxis->setRange(0, xUpper * 1.05);
    ui->customPlot->yAxis->setRange(0, yUpper * 1.05);

    ui->customPlot->replot();
}


void CsvAnalyzeWidget::onEditName(const QString &name) {
    if (curGraph==nullptr)
        return;
    curGraph->setName(name);
    QTreeWidgetItem* item = curveToItemMap.value(curGraph,nullptr);
    if (item)
        item->setText(CsvAnalyzeWidget::colItem,name);
    ui->customPlot->replot();
}

void CsvAnalyzeWidget::onSpinWidth(int width) {
    if (curGraph==nullptr)
        return;
    QPen pen = curGraph->pen();
    pen.setWidth(width);
    curGraph->setPen(pen);
    ui->customPlot->replot();
}

void CsvAnalyzeWidget::onSpinAlpha(int alpha) {
    if (curGraph==nullptr)
        return;
    QPen pen = curGraph->pen();
    QColor color = pen.color();
    color.setAlpha(alpha*255/100);
    pen.setColor(color);
    curGraph->setPen(pen);

    QCPScatterStyle scatter = curGraph->scatterStyle();
    if (scatter.shape() != QCPScatterStyle::ssNone) {
        scatter.setBrush(QBrush(color));
        curGraph->setScatterStyle(scatter);
    }

    ui->customPlot->replot();
}

void CsvAnalyzeWidget::onPropColorClicked() {
    if (curGraph== nullptr)
        return;
    QColor color = curGraph->pen().color();
    QColor newColor = QColorDialog::getColor(color,this,"选择曲线颜色");

    if (newColor.isValid()) {
        QPen pen = curGraph->pen();
        newColor.setAlpha(pen.color().alpha());
        pen.setColor(newColor);
        curGraph->setPen(pen);

        QCPScatterStyle scatter = curGraph->scatterStyle();
        if (scatter.shape() != QCPScatterStyle::ssNone) {
            scatter.setBrush(QBrush(newColor));
            curGraph->setScatterStyle(scatter);
        }

        ui->customPlot->replot();
    }
}

void CsvAnalyzeWidget::onComboStyleChanged(int index) {
    if (curGraph== nullptr)
        return;
    QPen pen = curGraph->pen();
    if (index == 0)
        pen.setStyle(Qt::SolidLine);
    else if (index == 1)
        pen.setStyle(Qt::DashLine);

    curGraph->setPen(pen);
    ui->customPlot->replot();
}

void CsvAnalyzeWidget::onComboScatterChanged(int index) {
    if (curGraph== nullptr)
        return;

    QCPScatterStyle::ScatterShape shape = QCPScatterStyle::ssNone;
    switch (index) {
        case 0:shape = QCPScatterStyle::ssNone;
            break;
        case 1:shape = QCPScatterStyle::ssCircle;
            break;
        case 2:shape = QCPScatterStyle::ssSquare;
            break;
        case 3:shape = QCPScatterStyle::ssDiamond;
            break;
        case 4:shape = QCPScatterStyle::ssTriangle;
            break;
    }
    QCPScatterStyle scatterStyle(shape,ui->spinScatterSize->value());

    if (shape!=QCPScatterStyle::ssNone) {
        scatterStyle.setBrush(curGraph->pen().color());
        scatterStyle.setPen(Qt::NoPen);
    }
    curGraph->setScatterStyle(scatterStyle);
    ui->customPlot->replot();
}

void CsvAnalyzeWidget::onBtnBringTopClicked() {
    if (curGraph== nullptr)
        return;
    if (!ui->customPlot->layer("top_layer"))
        ui->customPlot->addLayer("top_layer",ui->customPlot->layer("main"), QCustomPlot::limAbove);
    if (curGraph->layer()->name() == "top_layer")
        curGraph->setLayer("main");
    curGraph->setLayer("top_layer");
    ui->customPlot->replot();
}

void CsvAnalyzeWidget::onBtnSendBottomClicked() {
    if (curGraph== nullptr)
        return;
    if (!ui->customPlot->layer("bottom_layer"))
        ui->customPlot->addLayer("bottom_layer",ui->customPlot->layer("main"), QCustomPlot::limBelow);
    curGraph->setLayer("bottom_layer");
    QList<QCPLayerable *>items = ui->customPlot->layer("bottom_layer")->children();
    for (QCPLayerable *item:items) {
        if (item!=curGraph) {
            item->setLayer("main");
            item->setLayer("bottom_layer");
        }
    }
    ui->customPlot->replot();
}

void CsvAnalyzeWidget::onComboLegendChanged(int index) {
    if (index==2)
        ui->customPlot->legend->setVisible(false);
    else {
        ui->customPlot->legend->setVisible(true);

        if (index==1)
            ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
        else if (index==0)
            ui->customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignTop);
    }
    ui->customPlot->replot();
}

void CsvAnalyzeWidget::updateGlobalFont() {
    QFont baseFont = ui->comboFont->currentFont();
    int fontSize = ui->spinFontSize->value();
    bool isBold = ui->checkFontBold->isChecked();

    QFont labelFont = baseFont;
    labelFont.setPointSize(fontSize);
    labelFont.setBold(isBold);

    QFont secondaryFont = baseFont;
    secondaryFont.setPointSize(qMax(6, fontSize - 2)); // qMax 防止字号减成负数或太小看不清
    secondaryFont.setBold(false);

    ui->customPlot->xAxis->setLabelFont(labelFont);
    ui->customPlot->xAxis->setTickLabelFont(secondaryFont);

    ui->customPlot->yAxis->setLabelFont(labelFont);
    ui->customPlot->yAxis->setTickLabelFont(secondaryFont);

    ui->customPlot->legend->setFont(secondaryFont);

    ui->customPlot->replot();
}

void CsvAnalyzeWidget::onComboGridChanged(int index) {
    QPen gridPen(QColor(220, 220, 220), 1, Qt::DashLine);

    if (index == 0) {
        ui->customPlot->xAxis->grid()->setVisible(false);
        ui->customPlot->yAxis->grid()->setVisible(true);
        ui->customPlot->yAxis->grid()->setPen(gridPen);
    } else if (index == 1) {
        ui->customPlot->xAxis->grid()->setVisible(true);
        ui->customPlot->yAxis->grid()->setVisible(true);
        ui->customPlot->xAxis->grid()->setPen(gridPen);
        ui->customPlot->yAxis->grid()->setPen(gridPen);
    } else if (index == 2) {
        ui->customPlot->xAxis->grid()->setVisible(false);
        ui->customPlot->yAxis->grid()->setVisible(false);
    }

    ui->customPlot->replot();
}

void CsvAnalyzeWidget::exportChart() {
    QString filter = "PNG 图片 (*.png);;JPEG 图片 (*.jpg);;PDF 矢量图 (LaTeX推荐) (*.pdf)";

    QString fileName = QFileDialog::getSaveFileName(this, "导出图表", "Result_Chart", filter);

    if (fileName.isEmpty())
        return;

    int width = 0, height = 0;
    int ratioIndex = ui->comboExportRatio->currentIndex();
    if (ratioIndex==1) {
        width = 1200;
        height = 900;
    }else if (ratioIndex==2) {
        width = 2400;
        height = 1200;
    }

    if (fileName.endsWith(".png", Qt::CaseInsensitive))
        ui->customPlot->savePng(fileName, width, height, 2.0, 100);
    else if (fileName.endsWith(".jpg", Qt::CaseInsensitive))
        ui->customPlot->saveJpg(fileName, width, height, 2.0, 100);
    else if (fileName.endsWith(".pdf", Qt::CaseInsensitive))
        ui->customPlot->savePdf(fileName,width, height);
}

void CsvAnalyzeWidget::restoreLayout() {
    ui->tableView->setVisible(true);
    ui->treeWidget->setVisible(true);
    ui->rightPanelWidget->setVisible(true);

    ui->Hsplitter->setSizes(QList<int>{350, 950, 300});
    ui->Vsplitter->setSizes(QList<int>{700, 300});
}

void CsvAnalyzeWidget::onSpinScatterSizeChanged(int size) {
    if (curGraph== nullptr)
        return;
    QCPScatterStyle scatter_style = curGraph->scatterStyle();
    if (curGraph->scatterStyle().shape()!=QCPScatterStyle::ssNone) {
        scatter_style.setSize(size);
        curGraph->setScatterStyle(scatter_style);
        ui->customPlot->replot();
    }
}

void CsvAnalyzeWidget::onAxisTitleChanged() {
    ui->customPlot->xAxis->setLabel(ui->editXTitle->text());
    ui->customPlot->yAxis->setLabel(ui->editYTitle->text());
    ui->customPlot->replot();
}

void CsvAnalyzeWidget::removeSingleCSV(QTreeWidgetItem *csvItem) {
    if (!csvItem || csvItem->type() != CsvAnalyzeWidget::itCSVItem) return;
    for (int i = 0; i < csvItem->childCount(); ++i) {
        QTreeWidgetItem* paramItem = csvItem->child(i);
        removeCurveFromPlot(paramItem);
    }
    QString fileName = csvItem->data(CsvAnalyzeWidget::colItem, Qt::UserRole).toString();
    csvCache.remove(fileName);

    if (currentCSVFileName == fileName) {
        currentCSVFileName.clear();
        if (tableModel) tableModel->clear();
    }
    delete csvItem;
}

QMap<QString,QString>CsvAnalyzeWidget::csvInfo() {
    QString expName;
    QString csvPath;
    int bestEpoch;

    QMap<QString,QString> info;
    QTreeWidgetItem* currentItem = ui->treeWidget->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "提示", "请先选择一个 CSV 文件");
        return info;
    }
    QTreeWidgetItem* selectItem = nullptr;
    if (currentItem->type()== CsvAnalyzeWidget::itParamItem)
        selectItem = currentItem->parent();
    else if (currentItem->type()== CsvAnalyzeWidget::itCSVItem)
        selectItem = currentItem;
    else {
        QMessageBox::warning(this, "Warning", "没有选择 CSV 文件");
        return info;
    }

    csvPath = selectItem->data(CsvAnalyzeWidget::colItem, Qt::UserRole).toString();

    if (csvPath.isEmpty() || !csvCache.contains(csvPath)) {
        QMessageBox::warning(this, "提示", "CSV文件不存在");
        return info;
    }

    expName = QFileInfo(csvPath).completeBaseName();
    QStringList data = csvCache[csvPath];

    if (data.size()<2) {
        QMessageBox::warning(this,"提示","CSV文件中没有数据");
        return info;
    }

    QStringList headers = data[0].split(",");
    int pCol = -1, RCol = -1, map50Col = -1, map5095Col = -1;
    for (int i = 0; i < headers.size(); ++i) {
        if (headers[i].contains("precision",Qt::CaseInsensitive))
            pCol = i;
        else if (headers[i].contains("recall",Qt::CaseInsensitive))
            RCol = i;
        else if (headers[i].contains("map50-95",Qt::CaseInsensitive)||headers[i].contains("map_0.5:0.95",Qt::CaseInsensitive))
            map5095Col = i;
        else if (headers[i].contains("map50",Qt::CaseInsensitive)||headers[i].contains("mAP_0.5",Qt::CaseInsensitive))
            map50Col = i;
    }

    QVector<double>value;
    for (int i = 1; i < data.size(); ++i)
        value.append(data[i].split(",").at(pCol).trimmed().toDouble());
    double maxP = *std::max_element(value.begin(), value.end());

    value.clear();
    for (int i = 1; i < data.size(); ++i)
        value.append(data[i].split(",").at(RCol).trimmed().toDouble());
    double maxR = *std::max_element(value.begin(), value.end());

    value.clear();
    for (int i = 1; i < data.size(); ++i)
        value.append(data[i].split(",").at(map50Col).trimmed().toDouble());
    double maxMap50 = *std::max_element(value.begin(), value.end());

    value.clear();
    for (int i = 1; i < data.size(); ++i)
        value.append(data[i].split(",").at(map5095Col).trimmed().toDouble());
    double maxMap5095 = *std::max_element(value.begin(), value.end());

    value.clear();
    for (int i = 1; i < data.size(); ++i)
        value.append((data[i].split(",").at(map50Col).trimmed().toDouble())*0.1+(data[i].split(",").at(map5095Col).trimmed().toDouble())*0.9);
    bestEpoch = std::max_element(value.begin(), value.end()) - value.begin();

    info.insert("name", expName);
    info.insert("csvPath", csvPath);
    info.insert("maxP", QString::number(maxP, 'f', 3));
    info.insert("maxR", QString::number(maxR, 'f', 3));
    info.insert("maxMap50", QString::number(maxMap50, 'f', 3));
    info.insert("maxMap5095", QString::number(maxMap5095, 'f', 3));
    info.insert("bestEpoch", QString::number(bestEpoch));

    return info;
}