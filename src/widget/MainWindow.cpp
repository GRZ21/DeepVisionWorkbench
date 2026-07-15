//
// Created by GRZ.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QString>
#include "../utils/qcustomplot.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QFile file(":/styles/MainWindow.qss");

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(QString::fromUtf8(file.readAll()));
    }

    resize(1400,750);
    setMinimumSize(1000,500);
    setWindowTitle("DeepVision Workbench");

    csvAnalyzeWidget = new CsvAnalyzeWidget(this);
    experimentRecordsWidget = new ExperimentRecordsWidget(this);
    experimentRecordsWidget->setCsvAnalyzeWidget(csvAnalyzeWidget);
    experimentRecordsWidget->initTable();
    ui->tabWidget->removeTab(0);
    ui->tabWidget->insertTab(0, csvAnalyzeWidget, "训练曲线");
    ui->tabWidget->removeTab(1);
    ui->tabWidget->insertTab(1, experimentRecordsWidget, "实验记录");
    ui->tabWidget->removeTab(2);    // 隐藏第三个界面
    ui->tabWidget->setCurrentIndex(0); // 设置默认显示第一个界面

    connect(ui->actionOpen, &QAction::triggered, csvAnalyzeWidget, &CsvAnalyzeWidget::openCsvFile);
    connect(ui->actionOpenDir, &QAction::triggered, csvAnalyzeWidget, &CsvAnalyzeWidget::openCsvDirectory);
    connect(ui->action_Clear_Plot, &QAction::triggered, csvAnalyzeWidget, &CsvAnalyzeWidget::clearPlot);
    connect(ui->action_Remove_All_Files, &QAction::triggered, csvAnalyzeWidget, &CsvAnalyzeWidget::removeAllFiles);
    connect(ui->action_Toggle_File_Tree, &QAction::triggered, csvAnalyzeWidget, &CsvAnalyzeWidget::setFileTreeVisible);
    connect(ui->action_Toggle_Data_Table, &QAction::triggered, csvAnalyzeWidget, &CsvAnalyzeWidget::setDataTableVisible);
    connect(ui->action_Toggle_Properties, &QAction::triggered, csvAnalyzeWidget, &CsvAnalyzeWidget::setPropertyPanelVisible);
    connect(ui->action_Delete_CSV, &QAction::triggered, csvAnalyzeWidget, &CsvAnalyzeWidget::deleteCsv);
    connect(ui->action_Q, &QAction::triggered, this, &MainWindow::close);
    connect(ui->action_Documentation, &QAction::triggered, this, &MainWindow::onActionGuide);
    connect(ui->action_About, &QAction::triggered, this, &MainWindow::onActionAbout);
    connect(ui->action_Export_Plot, &QAction::triggered, csvAnalyzeWidget, &CsvAnalyzeWidget::exportChart);
    connect(ui->action_Export_Xlsx,&QAction::triggered,experimentRecordsWidget,&ExperimentRecordsWidget::exportRecordAsXlsx);
    connect(ui->action_Restore_Layout, &QAction::triggered, this,
            [this] {
                resize(1400, 750);
                int index = ui->tabWidget->currentIndex();
                if (index == 0) {
                    csvAnalyzeWidget->restoreLayout();
                    ui->action_Toggle_File_Tree->setChecked(true);
                    ui->action_Toggle_Data_Table->setChecked(true);
                    ui->action_Toggle_Properties->setChecked(true);
                } else if (index == 1) {
                    experimentRecordsWidget->restoreLayout();
                }
            });

    connect(ui->action_FullScreen, &QAction::triggered, this, [this] {
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    });

    ui->action_Toggle_File_Tree->setChecked(true);
    ui->action_Toggle_Data_Table->setChecked(true);
    ui->action_Toggle_Properties->setChecked(true);

    ui->statusbar->hide();
}


MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::onActionGuide() {
    QDesktopServices::openUrl(QUrl("https://github.com/GRZ21/CSVResultAnalyzer"));
}

void MainWindow::onActionAbout() {
    QString Text = "<h3>CSV Result Analyzer</h3>"
                "<p><b>开发者：</b>GRZ</p>"
                "<p><b>联系邮箱：</b>grzgrzgrz21@foxmail.com</p>"
                "<p><b>版本：</b>v2.0beta</p>"
                "<p>本软件专为深度学习（如YOLO和RT-DETR系列）训练日志可视化设计。</p>"
                "<hr>"
                "<p>基于 Qt(C++) 构建。</p>"
                "<p><b>开源协议：</b>本项目遵循 <a href='https://opensource.org/licenses/MIT'>MIT License</a>。</p>"
                "<p><span style='font-size:10px; color:gray;'>Copyright &copy; 2026 GRZ. All rights reserved.</span></p>";
    QMessageBox box(this);

    box.setObjectName("aboutBox");
    box.setWindowTitle("关于");
    box.setIcon(QMessageBox::NoIcon);
    box.setTextFormat(Qt::RichText);
    box.setText(Text);
    box.setTextInteractionFlags(Qt::TextBrowserInteraction);
    box.setStandardButtons(QMessageBox::Ok);

    if (auto *label = box.findChild<QLabel *>("qt_msgbox_label")) {
        label->setMinimumWidth(430);
        label->setWordWrap(true);
    }

    if (auto *button = box.button(QMessageBox::Ok)) {
        button->setText("确定");
        button->setFixedWidth(88);
    }
    box.exec();
}