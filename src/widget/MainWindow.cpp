//
// Created by GRZ.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QString>
#include <QColorDialog>

#include "../utils/qcustomplot.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 表格相关
    tableModel = new QStandardItemModel(this);
    tableSelectionModel = new QItemSelectionModel(tableModel);
    ui->tableView->setModel(tableModel);
    ui->tableView->setSelectionModel(tableSelectionModel);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 界面大小
    ui->Vsplitter->setSizes(QList<int>({700,300}));
    ui->Hsplitter->setSizes(QList<int>({200,600,200}));

    // 表头相关
    ui->treeWidget->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);

    // 自定义右键策略
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    // 图相关
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->legend->setBrush(QBrush(QColor(255,255,255,150)));
    ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom|QCP::iSelectPlottables);

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::actionOpen);
    connect(ui->treeWidget,&QTreeWidget::itemClicked,this,&MainWindow::onTreeItemClicked);
    connect(ui->treeWidget,&QTreeWidget::itemDoubleClicked,this,&MainWindow::onTreeItemDoubleClicked);
    connect(tableSelectionModel,&QItemSelectionModel::selectionChanged,this,
        [&](const QItemSelection& selected,const QItemSelection& deselected) {
            Q_UNUSED(deselected);
            if (!selected.isEmpty()) {
                QModelIndex index = selected.indexes().first();
                highlightTableParam(index);
            }
        });
    connect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainWindow::onParmItemChanged);
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

    connect(ui->customPlot,&QCustomPlot::plottableClick,this,&MainWindow::onCurveClicked);

    connect(ui->editName,&QLineEdit::textChanged,this,&MainWindow::onEditName);
    connect(ui->spinWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSpinWidth);
    connect(ui->spinAlpha, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSpinAlpha);
    connect(ui->btnColor, &QPushButton::clicked, this, &MainWindow::onPropColorClicked);
    connect(ui->comboStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboStyleChanged);
    connect(ui->comboScatter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboScatterChanged);
    connect(ui->btnBringTop,&QPushButton::clicked,this,&MainWindow::onBtnBringTopClicked);
    connect(ui->btnSendBottom,&QPushButton::clicked,this,&MainWindow::onBtnSendBottomClicked);
    connect(ui->comboFont,&QFontComboBox::currentFontChanged,this,&MainWindow::updateGlobalFont);
    connect(ui->spinFontSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateGlobalFont);
    connect(ui->checkFontBold,&QCheckBox::toggled,this,&MainWindow::updateGlobalFont);
    connect(ui->comboLegend, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboLegendChanged);
    connect(ui->comboGrid, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboGridChanged);
    connect(ui->editXTitle,&QLineEdit::textChanged,this,&MainWindow::onAxisTitleChanged);
    connect(ui->editYTitle,&QLineEdit::textChanged,this,&MainWindow::onAxisTitleChanged);
    connect(ui->spinScatterSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSpinScatterSizeChanged);
    connect(ui->btnResetZoom,&QPushButton::clicked,this,&MainWindow::autoRescalePlot);
    connect(ui->btnExportImg,&QPushButton::clicked,this,&MainWindow::onBtnExportClicked);

    connect(ui->actionOpenDir, &QAction::triggered, this, &MainWindow::onActionOpenDir);
    connect(ui->action_Clear_Plot, &QAction::triggered, this, &MainWindow::onActionClearPlot);
    connect(ui->action_Remove_All_Files, &QAction::triggered, this, &MainWindow::onActionRemoveAllFiles);
    connect(ui->action_Toggle_File_Tree, &QAction::triggered, this, &MainWindow::onActionToggleTree);
    connect(ui->action_Toggle_Data_Table, &QAction::triggered, this, &MainWindow::onActionToggleTable);
    connect(ui->action_Toggle_Properties, &QAction::triggered, this, &MainWindow::onActionToggleProps);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::onBtnExportClicked);
    connect(ui->action_Delete_CSV, &QAction::triggered, this, &MainWindow::onActionDeleteCSV);
    connect(ui->treeWidget,&QTreeWidget::customContextMenuRequested,this,&MainWindow::onTreeContextMenu);
    connect(ui->action_Q, &QAction::triggered, this, &MainWindow::close);
    connect(ui->action_Documentation, &QAction::triggered, this, &MainWindow::onActionGuide);
    connect(ui->action_About, &QAction::triggered, this, &MainWindow::onActionAbout);


    ui->action_Toggle_File_Tree->setChecked(true);
    ui->action_Toggle_Data_Table->setChecked(true);
    ui->action_Toggle_Properties->setChecked(true);

    buildTreeHeader();
    onComboGridChanged(0);
    onAxisTitleChanged();
    initTree();
    updateGlobalFont();
}


MainWindow::~MainWindow() {
    delete ui;
}


// 构建树形结构的表头
void MainWindow::buildTreeHeader() {
    ui->treeWidget->clear();

    QTreeWidgetItem* header = new QTreeWidgetItem();  // 创建表头项
    header->setText(MainWindow::colItem,"文件名");  // 设置表头项的文本
    header->setText(MainWindow::colDate,"最后修改日期");  // 设置表头项的文本

    // 设置表头项的对齐方式
    header->setTextAlignment(MainWindow::colItem,Qt::AlignHCenter|Qt::AlignVCenter);

    // 设置表头项
    ui->treeWidget->setHeaderItem(header);
}

// 初始化树形结构
void MainWindow::initTree() {
    // 创建根项
    QTreeWidgetItem* root = new QTreeWidgetItem(MainWindow::itTopItem);  // 创建根项
    root->setText(MainWindow::colItem,"csv name: ");  // 设置根项的文本
    root->setText(MainWindow::colDate,QDate::currentDate().toString());  // 设置根项的文本
    root->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable|Qt::ItemIsAutoTristate);  // 设置根项的标志
    root->setCheckState(MainWindow::colItem,Qt::Checked);  // 设置根项的选中状态
    ui->treeWidget->addTopLevelItem(root);  // 添加根项到树形结构中
}

// 添加CSV文件节点
QTreeWidgetItem* MainWindow::addCSVItem(QTreeWidgetItem *parItem, QString fileName) {
    QFileInfo fileInfo(fileName);  // 获取文件信息
    QString lastFileName = fileInfo.fileName();  // 获取文件名
    QDateTime fileDate = fileInfo.lastModified();  // 获取文件修改时间

    QTreeWidgetItem* item = new QTreeWidgetItem(MainWindow::itCSVItem);  // 创建CSV文件节点
    item->setText(MainWindow::colItem,lastFileName);  // 设置节点对应列的文本
    item->setText(MainWindow::colDate,fileDate.toString());  // 设置节点对应列的文本
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);  // 设置节点的标志

    item->setData(MainWindow::colItem,Qt::UserRole,QVariant(fileName));
    parItem->addChild(item);   // 将该csv节点添加到对应的根节点中
    return item;
}

// 从CSV表格文件头中添加子节点
void MainWindow::addParmItem(QTreeWidgetItem *parItem, QString CSVHeaderParam,int colIndex) {
    QTreeWidgetItem* item = new QTreeWidgetItem(MainWindow::itParamItem);  // 创建参数节点
    QString trimmedParam = CSVHeaderParam.trimmed();  // 去除参数名前后的空格
    item->setText(MainWindow::colItem,trimmedParam);  // 设置节点对应列的文本
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable|Qt::ItemIsAutoTristate);  // 设置节点的标志
    item->setCheckState(MainWindow::colItem,Qt::Unchecked);  // 设置节点的选中状态

    item->setData(MainWindow::colItem,Qt::UserRole,colIndex);
    parItem->addChild(item);  // 将该参数节点添加到对应的csv父节点中
}

// 从CSV文件中加载数据并缓存
void MainWindow::loadCSVCache(QString fileName) {
    QStringList data;   // 用于存储CSV文件数据
    QFile file(fileName);  // 创建文件对象
    if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {  // 打开文件
        QTextStream in(&file);  // 创建文本流对象
        while (!in.atEnd()) {   // 读取文件的每一行
            QString line = in.readLine().trimmed();  // 读取并去除行首行尾的空格
            data<<line;  // 将行数据添加到QStringList中
        }
    }
    file.close();  // 关闭文件
    if (!data.isEmpty()) {
        csvCache[fileName] = data;   // 在QMap缓存中添加数据<QString，QStringList>
        csvTableDisplay(data);  // 显示CSV数据表
    }
}

// 表格，树和图的联动，高亮显示对应的列
void MainWindow::highlightColumn(int columIndex) {
    if (columIndex<0||columIndex>=tableModel->columnCount())
        return;
    
    int rowCount = tableModel->rowCount();
    if (rowCount == 0)
        return;

    // 获取指定列的QModelIndex范围，行号不同，列号相同
    QModelIndex topLeft = tableModel->index(0, columIndex);
    QModelIndex bottomRight = tableModel->index(rowCount - 1, columIndex);

    // 创建QItemSelection对象
    QItemSelection selection(topLeft, bottomRight);

    // 选择指定列
    tableSelectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
}

// 表格，树和图的联动，高亮显示对应的树的节点以及图中对应的曲线
void MainWindow::highlightTableParam(QModelIndex& Index) {
    if (!Index.isValid())
        return;
    
    int columnIndex = Index.column();   // 获取列索引

    // 从选中的列索引获取参数名
    QString paramName = tableModel->headerData(columnIndex, Qt::Horizontal).toString().trimmed();
    
    if (paramName.isEmpty() || currentCSVFileName.isEmpty())
        return;


    QTreeWidgetItem* csvItem = nullptr;
    // 从树形结构中找到对应的CSV文件节点，使用文件名进行匹配，先找出所有名称对应的节点
    QList<QTreeWidgetItem*> allItems = ui->treeWidget->findItems(
        QFileInfo(currentCSVFileName).fileName(), // 提取纯文件名
        Qt::MatchExactly | Qt::MatchRecursive
    );
    // 遍历找到的名称相同的节点，找到对应的CSV文件节点
    for (QTreeWidgetItem* item : allItems) {
        if (item->type() == MainWindow::itCSVItem) {
            QString fileName = item->data(MainWindow::colItem, Qt::UserRole).toString();
            // 通过 Qt::UserRole 中存储的完整路径做精确匹配，区分同名但不同路径的文件
            if (fileName == currentCSVFileName) {
                csvItem = item;
                break;
            }
        }
    }
    
    if (csvItem == nullptr)
        return;

    // 在树中找到对应的参数子节点
    for (int i = 0; i < csvItem->childCount(); i++) {
        QTreeWidgetItem* child = csvItem->child(i);
        if (child->type() == MainWindow::itParamItem && 
            child->data(MainWindow::colItem, Qt::UserRole).toInt() == columnIndex) {
            ui->treeWidget->setCurrentItem(child);   // 选中对应的参数子节点
            csvItem->setExpanded(true);   // 展开对应的CSV文件父节点

            // 选择该参数节点对应的曲线
            ui->customPlot->deselectAll();
            for (int j = 0;j<ui->customPlot->graphCount();j++) {
                QCPGraph* graph = ui->customPlot->graph(j);
                if (curveToItemMap.value(graph, nullptr)== child) {
                    // 找到与该参数节点关联的曲线
                    graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
                    curGraph = graph;

                    // 将曲线移动到顶层显示
                    if (!ui->customPlot->layer("top_layer")) {
                        ui->customPlot->addLayer("top_layer", ui->customPlot->layer("main"), QCustomPlot::limAbove);
                    }
                    if (graph->layer()->name() == "top_layer") {
                        graph->setLayer("main");
                    }
                    graph->setLayer("top_layer");

                    // 阻塞信号，防止属性面板的修改影响曲线属性
                    ui->editName->blockSignals(true);
                    ui->spinWidth->blockSignals(true);
                    ui->spinAlpha->blockSignals(true);
                    ui->comboStyle->blockSignals(true);
                    ui->comboScatter->blockSignals(true);
                    ui->spinScatterSize->blockSignals(true);

                    // 更新右侧的曲线属性面板
                    ui->editName->setText(graph->name());
                    ui->spinWidth->setValue(graph->pen().width());
                    ui->spinAlpha->setValue(graph->pen().color().alpha() * 100 / 255);
                    ui->spinScatterSize->setValue(graph->scatterStyle().size());

                    if (graph->pen().style() == Qt::SolidLine) ui->comboStyle->setCurrentIndex(0);
                    else if (graph->pen().style() == Qt::DashLine) ui->comboStyle->setCurrentIndex(1);

                    // 根据散点形状更新下拉框
                    switch(graph->scatterStyle().shape()) {
                        case QCPScatterStyle::ssNone:     ui->comboScatter->setCurrentIndex(0); break;
                        case QCPScatterStyle::ssCircle:   ui->comboScatter->setCurrentIndex(1); break;
                        case QCPScatterStyle::ssSquare:   ui->comboScatter->setCurrentIndex(2); break;
                        case QCPScatterStyle::ssDiamond:  ui->comboScatter->setCurrentIndex(3); break;
                        case QCPScatterStyle::ssTriangle: ui->comboScatter->setCurrentIndex(4); break;
                        default:                          ui->comboScatter->setCurrentIndex(0); break;
                    }

                    // 解除信号阻塞
                    ui->editName->blockSignals(false);
                    ui->spinWidth->blockSignals(false);
                    ui->spinAlpha->blockSignals(false);
                    ui->comboStyle->blockSignals(false);
                    ui->comboScatter->blockSignals(false);
                    ui->spinScatterSize->blockSignals(false);

                    // 更新曲线统计信息
                    updateCurveStats(graph);

                    break;
                }
            }
            ui->customPlot->replot();
            return;
        }
    }
}

// 绘制CSV文件的指定列数据
void MainWindow::poltCsvColumn(QTreeWidgetItem* item,int columnIndex) {
    Q_UNUSED(columnIndex);
    if (!item || item->type() != MainWindow::itParamItem)
        return;

    QTreeWidgetItem* parItem = item->parent();
    if (!parItem)
        return;

    // 获取CSV文件名
    currentCSVFileName = parItem->data(MainWindow::colItem, Qt::UserRole).toString();
    QStringList data;
    if (csvCache.contains(currentCSVFileName)) {   // 从缓存中获取对应的数据
        data = csvCache[currentCSVFileName];
    } else {
        return;
    }

    // 从子节点中获取列号，作为索引使用
    int valueIndex = item->data(MainWindow::colItem, Qt::UserRole).toInt();

    // 获取CSV文件的列标题，找到epoch列的索引
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
    for (int i = 1; i < data.size(); i++) {   // 跳过表头，从表的第一行开始
        QString line = data.at(i);
        QStringList fields = line.split(",");

        // 将行和列的数据全部存入vector中
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
    QString currentItemName = item->text(MainWindow::colItem);
    QString pureName;
    if (currentItemName == originalHeaderName) {
        pureName = QFileInfo(currentCSVFileName).baseName() + " - " + currentItemName;
    } else {
        pureName = currentItemName;
    }

    if (!xData.isEmpty()) {
        drawOnCustomPlot(pureName, xData, yData, item);  // 绘制数据
    }
}

// poltCsvColumn调用该函数绘制曲线
void MainWindow::drawOnCustomPlot(QString name, QVector<double> x, QVector<double> y,QTreeWidgetItem* item) {
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

// 在树中勾选与取消参数节点前复选框时对图中曲线的变化
void MainWindow::onParmItemChanged(QTreeWidgetItem *item, int columIndex) {
    if (item->type() != MainWindow::itParamItem)   // 只处理参数节点
        return;
    if (item->checkState(columIndex)==Qt::Checked)  // 如果勾选了参数节点
        poltCsvColumn(item,columIndex);   // 绘制曲线
    else  // 如果取消勾选了参数节点
        removeCurveFromPlot(item);  // 移除曲线
}

// 从图中移除与参数节点对应的曲线
void MainWindow::removeCurveFromPlot(QTreeWidgetItem *item) {
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

// 更新曲线统计信息
void MainWindow::updateCurveStats(QCPGraph *graph) {
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

// 打开CSV文件
void MainWindow::actionOpen() {
    QStringList data;

    // 打开文件选择器对话框
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open CSV File"), QDir::currentPath(), tr("CSV Files (*.csv)"));
    if (!fileName.isEmpty()) {
        QTreeWidgetItem* parItem,*item;
        item = ui->treeWidget->currentItem();

        if (item != nullptr) {
            if (item->type() == MainWindow::itCSVItem || item->type() == MainWindow::itParamItem) {  // 如果当前节点是CSV节点或参数节点
                while (item->parent() != nullptr && item->type() != MainWindow::itTopItem) {  // 向上遍历，找到当前节点的根节点（顶级节点）
                    item = item->parent();   // 找到根节点（顶级节点）
                }
            }
            parItem = item;  // 当前节点的根节点（顶级节点）
        } else {
            ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(MainWindow::itTopItem));   // 添加一个顶级节点

            // 将这个新添加的顶级节点赋值给parItem（索引为topLevelItemCount() - 1），作为csv节点的父节点
            parItem = ui->treeWidget->topLevelItem(ui->treeWidget->topLevelItemCount() - 1);
        }

        // 在顶级节点下添加CSV文件节点
        QTreeWidgetItem* csvItem = addCSVItem(parItem,fileName);
        parItem->setExpanded(true);

        // 按行读取CSV文件的数据，并且过滤掉无效行
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
            QTextStream in(&file);

            if (!in.atEnd()) {
                QString headLine = in.readLine();
                QStringList headerData = headLine.split(",");

                // 过滤掉无效行
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
        csvCache[fileName] = data;  // 将读取的数据缓存到QMap
        currentCSVFileName = fileName;  // 记录当前文件名
    }
    csvTableDisplay(data);
}

// 清除图中所有曲线
void MainWindow::onActionClearPlot() {
    curveToItemMap.clear();
    curGraph = nullptr;

    ui->customPlot->clearGraphs();
    ui->customPlot->replot();

    ui->treeWidget->blockSignals(true);
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        if ((*it)->type() == MainWindow::itParamItem) {
            (*it)->setCheckState(MainWindow::colItem, Qt::Unchecked);
        }
        ++it;
    }
    ui->treeWidget->blockSignals(false);
}

// 从树中移除所有文件
void MainWindow::onActionRemoveAllFiles() {
    onActionClearPlot();
    csvCache.clear();
    currentCSVFileName.clear();
    for (int i = 0;i<ui->treeWidget->topLevelItemCount();i++) {
        QTreeWidgetItem* item = ui->treeWidget->topLevelItem(i);
        qDeleteAll(item->takeChildren());
    }
    if (tableModel)
        tableModel->clear();
}

// 打开文件夹并加载所有CSV文件
void MainWindow::onActionOpenDir() {
   QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirPath.isEmpty())
        return;

    QDirIterator it(dirPath, QStringList() << "*.csv", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString fileName = it.next();
        if (!fileName.isEmpty()) {
            QStringList data;

            QTreeWidgetItem* parItem, *item;
            item = ui->treeWidget->currentItem();

            if (item != nullptr) {
                if (item->type() == MainWindow::itCSVItem || item->type() == MainWindow::itParamItem) {
                    while (item->parent() != nullptr && item->type() != MainWindow::itTopItem) {
                        item = item->parent();
                    }
                }
                parItem = item;
            } else {
                ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(MainWindow::itTopItem));
                parItem = ui->treeWidget->topLevelItem(ui->treeWidget->topLevelItemCount() - 1);
            }

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

// 文件树右键菜单
void MainWindow::onTreeContextMenu(const QPoint &pos) {
    QTreeWidgetItem* item = ui->treeWidget->itemAt(pos);  // 获取被点击的树节点
    if (!item)
        return;
    if (item->type() == MainWindow::itCSVItem) {
        QMenu menu;
        QAction* action = menu.addAction("移除该文件");  // 添加菜单项
        // 绑定菜单项的触发信号到移除CSV文件的槽函数
        connect(action, &QAction::triggered, [this, item]() {removeSingleCSV(item);});
        menu.exec(ui->treeWidget->mapToGlobal(pos));
    }
}

// 从树中移除单个CSV文件节点，同时移除图表中的曲线,由在CSV节点上右键菜单触发后调用
void MainWindow::removeSingleCSV(QTreeWidgetItem *csvItem) {
    if (!csvItem || csvItem->type() != MainWindow::itCSVItem) return;
    for (int i = 0; i < csvItem->childCount(); ++i) {
        QTreeWidgetItem* paramItem = csvItem->child(i);
        removeCurveFromPlot(paramItem);
    }
    QString fileName = csvItem->data(MainWindow::colItem, Qt::UserRole).toString();
    csvCache.remove(fileName);

    if (currentCSVFileName == fileName) {
        currentCSVFileName.clear();
        if (tableModel) tableModel->clear();
    }
    delete csvItem;
}

// 移除CSV文件
void MainWindow::onActionDeleteCSV() {
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if (!item)
        return;
    if (item->type() == MainWindow::itCSVItem)
        removeSingleCSV(item);
    else if (item->type() == MainWindow::itParamItem)
        removeSingleCSV(item->parent());
}

// 切换树形控件的可见性
void MainWindow::onActionToggleTree(bool checked) {
    ui->treeWidget->setVisible(checked);
}

// 切换数据表的可见性
void MainWindow::onActionToggleTable(bool checked) {
    ui->tableView->setVisible(checked);
}

// 切换属性面板的可见性
void MainWindow::onActionToggleProps(bool checked) {
    if (ui->rightPanelWidget) {
        ui->rightPanelWidget->setVisible(checked);
    }
}

// 显示CSV数据表
void MainWindow::csvTableDisplay(QStringList& data) {
    if (data.isEmpty())
        return;

    int rowCount = data.length()-1;
    int columnCount = data[0].split(",").size();

    tableModel->clear();
    tableModel->setRowCount(rowCount);
    tableModel->setColumnCount(columnCount);

    QString header = data.at(0);
    QStringList headerData = header.split(",");

    // 设置表头
    for (int i = 0; i < columnCount; i++) {
        QStandardItem* item = new QStandardItem(headerData.at(i));
        item->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        tableModel->setHorizontalHeaderItem(i,item);
    }

    // 填充数据
    // 遍历数据并填充表格
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
    ui->tableView->horizontalHeader()->setVisible(true); // 保持表头可见
    ui->tableView->resizeColumnsToContents(); // 调整列宽
}

// 单击CSV文件树，显示对应文件数据
void MainWindow::onTreeItemClicked(QTreeWidgetItem *item, int column) {
    Q_UNUSED(column)
    if (item== nullptr)
        return;

    if (item->type() == MainWindow::itCSVItem) {  // 如果是CSV文件节点
        QString fileName = item->data(MainWindow::colItem,Qt::UserRole).toString();
        currentCSVFileName = fileName;  // 记录当前文件名
        if (csvCache.contains(fileName))  // 如果QMap缓存中包含该文件数据
            csvTableDisplay(csvCache[fileName]);  // 显示CSV数据表
        else
            loadCSVCache(fileName);  // 加载CSV缓存
    } else if (item->type() == MainWindow::itParamItem) {  // 如果是参数节点
        int paramIndex = item->data(MainWindow::colItem,Qt::UserRole).toInt();

        QTreeWidgetItem* parItem = item->parent();  // 找到父节点（CSV节点）
        currentCSVFileName = parItem->data(MainWindow::colItem,Qt::UserRole).toString();  // 记录当前文件名
        csvTableDisplay(csvCache[currentCSVFileName]);  // 显示CSV数据表
        highlightColumn(paramIndex);  // 高亮显示对应参数列
    }
}

// 双击CSV文件树，切换对应参数列的勾选状态
void MainWindow::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column) {
    if (item== nullptr)
        return;
    if (item->type()!=MainWindow::itParamItem)
        return;
    if (item->checkState(MainWindow::colItem)==Qt::Checked)
        item->setCheckState(column,Qt::Unchecked);
    else
        item->setCheckState(column,Qt::Checked);
}

// 曲线点击时更新树形控件和图表属性
void MainWindow::onCurveClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event) {
    Q_UNUSED(dataIndex);
    Q_UNUSED(event);
    curGraph = qobject_cast<QCPGraph*>(plottable);
    if (!curGraph)
        return;
    QTreeWidgetItem* item = curveToItemMap.value(curGraph,nullptr);
    if (item) {
        ui->treeWidget->setCurrentItem(item);
        ui->treeWidget->scrollToItem(item);
        highlightColumn(item->data(MainWindow::colItem,Qt::UserRole).toInt());
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

// 自动缩放图表
void MainWindow::autoRescalePlot() {
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

// 曲线名称改变时更新图表
void MainWindow::onEditName(const QString &name) {
    if (curGraph==nullptr)
        return;
    curGraph->setName(name);
    QTreeWidgetItem* item = curveToItemMap.value(curGraph,nullptr);
    if (item)
        item->setText(MainWindow::colItem,name);
    ui->customPlot->replot();
}

// 线条宽度改变时更新图表
void MainWindow::onSpinWidth(int width) {
    if (curGraph==nullptr)
        return;
    QPen pen = curGraph->pen();
    pen.setWidth(width);
    curGraph->setPen(pen);
    ui->customPlot->replot();
}

// 透明度改变时更新图表
void MainWindow::onSpinAlpha(int alpha) {
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

// 曲线颜色改变时更新图表
void MainWindow::onPropColorClicked() {
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

// 线条样式改变时更新图表
void MainWindow::onComboStyleChanged(int index) {
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

// 散点样式改变时更新图表
void MainWindow::onComboScatterChanged(int index) {
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

// 图层改变时更新图表，将该曲线置顶
void MainWindow::onBtnBringTopClicked() {
    if (curGraph== nullptr)
        return;
    if (!ui->customPlot->layer("top_layer"))
        ui->customPlot->addLayer("top_layer",ui->customPlot->layer("main"), QCustomPlot::limAbove);
    if (curGraph->layer()->name() == "top_layer")
        curGraph->setLayer("main");
    curGraph->setLayer("top_layer");
    ui->customPlot->replot();
}

// 图层改变时更新图表，将该曲线置底
void MainWindow::onBtnSendBottomClicked() {
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

// 图例位置改变时更新图表
void MainWindow::onComboLegendChanged(int index) {
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

// 更新全局字体设置
void MainWindow::updateGlobalFont() {
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

// 图表背景网格设置改变时更新图表
void MainWindow::onComboGridChanged(int index) {
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

// 导出图表
void MainWindow::onBtnExportClicked() {
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

// 散点大小改变时更新图表
void MainWindow::onSpinScatterSizeChanged(int size) {
    if (curGraph== nullptr)
        return;
    QCPScatterStyle scatter_style = curGraph->scatterStyle();
    if (curGraph->scatterStyle().shape()!=QCPScatterStyle::ssNone) {
        scatter_style.setSize(size);
        curGraph->setScatterStyle(scatter_style);
        ui->customPlot->replot();
    }
}

// 轴标题改变时更新图表
void MainWindow::onAxisTitleChanged() {
    ui->customPlot->xAxis->setLabel(ui->editXTitle->text());
    ui->customPlot->yAxis->setLabel(ui->editYTitle->text());
    ui->customPlot->replot();
}

void MainWindow::onActionGuide() {
    QDesktopServices::openUrl(QUrl("https://github.com/GRZ21/CSVResultAnalyzer"));
}

void MainWindow::onActionAbout() {
    QString Text = "<h3>CSV Result Analyzer</h3>"
                "<p><b>开发者：</b>GRZ</p>"
                "<p><b>联系邮箱：</b>grzgrzgrz21@foxmail.com</p>"
                "<p><b>版本：</b>v1.0</p>"
                "<p>本软件专为深度学习（如YOLO和RT-DETR系列）训练日志可视化设计。</p>"
                "<hr>"
                "<p>基于 Qt(C++) 构建。</p>"
                "<p><b>开源协议：</b>本项目遵循 <a href='https://opensource.org/licenses/MIT'>MIT License</a>。</p>"
                "<p><span style='font-size:10px; color:gray;'>Copyright &copy; 2026 GRZ. All rights reserved.</span></p>";
    QMessageBox::about(this, "关于", Text);
}