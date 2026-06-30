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

#include "../utils/qcustomplot.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    // 枚举类型，用于标识树形结构的节点类型（顶层节点1001，CSV文件节点1002，参数节点1003）
    enum treeItemType{itTopItem=1001,itCSVItem,itParamItem};

    // 枚举类型，用于标识树形结构的列索引（节点名称0，日期1）
    enum treeColNum{colItem=0,colDate};

    // 构建树形结构的表头
    void buildTreeHeader();

    // 初始化树形结构
    void initTree();

    // 添加CSV文件节点
    QTreeWidgetItem* addCSVItem(QTreeWidgetItem* parItem,QString fileName);

    // 添加参数节点
    void addParmItem(QTreeWidgetItem* parItem,QString CSVHeader,int colIndex);
    void loadCSVCache(QString fileName);

    // 表格，树和图的联动，高亮显示对应的列
    void highlightColumn(int columIndex);

    // 表格，树和图的联动，高亮显示对应的树的节点以及图中对应的曲线
    void highlightTableParam(QModelIndex& columIndex);

    // 绘制CSV文件的指定列数据
    void poltCsvColumn(QTreeWidgetItem* item,int columIndex);

    // poltCsvColumn调用该函数绘制曲线
    void drawOnCustomPlot(QString name,QVector<double>x,QVector<double>y,QTreeWidgetItem* item);

    // 在树中勾选与取消参数节点前复选框时对图中曲线的变化
    void onParmItemChanged(QTreeWidgetItem* item,int columIndex);

    // 从图中移除对应的曲线
    void removeCurveFromPlot(QTreeWidgetItem*  item);

    // 更新曲线统计信息
    void updateCurveStats(QCPGraph *graph);

    QStandardItemModel* tableModel;
    QItemSelectionModel* tableSelectionModel;
    QMap<QString,QStringList>csvCache;  // 用于存储CSV文件的记录缓存（文件名对应的stringList）
    QString currentCSVFileName;  // 记录当前显示的CSV文件名（完整路径）
    QHash<QCPGraph*,QTreeWidgetItem*> curveToItemMap;  // 用于存储曲线和对应树节点的映射
    QCPGraph* curGraph = nullptr;

private slots:
    void csvTableDisplay(QStringList&  data);   // 显示CSV数据表
    void onTreeItemClicked(QTreeWidgetItem* item,int column);   // 单击CSV文件树，显示对应文件数据
    void onTreeItemDoubleClicked(QTreeWidgetItem* item,int column);     // 双击CSV文件树，切换对应参数列的勾选状态
    void onCurveClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);  // 曲线点击时更新树形控件和图表属性
    void autoRescalePlot();     // 自动缩放图表
    void onEditName(const QString& name);   // 曲线名称改变时更新图表
    void onSpinWidth(int width);   // 线条宽度改变时更新图表
    void onSpinAlpha(int alpha);    // 透明度改变时更新图表
    void onPropColorClicked();  // 曲线颜色改变时更新图表
    void onComboStyleChanged(int index);    // 线条样式改变时更新图表
    void onComboScatterChanged(int index);  // 散点样式改变时更新图表
    void onBtnBringTopClicked();    // 图层改变时更新图表，将该曲线置顶
    void onBtnSendBottomClicked();    // 图层改变时更新图表，将该曲线置底
    void onComboLegendChanged(int index);   // 图例位置改变时更新图表
    void updateGlobalFont();    // 更新全局字体设置
    void onComboGridChanged(int index);     // 图表背景网格设置改变时更新图表
    void onBtnExportClicked();  // 导出图表
    void onSpinScatterSizeChanged(int size);    // 散点大小改变时更新图表
    void onAxisTitleChanged();  // 轴标题改变时更新图表
    void removeSingleCSV(QTreeWidgetItem* csvItem);  // 从树中移除单个CSV文件节点，同时移除图表中的曲线,由在CSV节点上右键菜单触发后调用

    // 打开文件和目录
    void actionOpen();
    void onActionOpenDir();

    // 全局清理与数据操作
    void onActionClearPlot();
    void onActionRemoveAllFiles();

    // 树形结构右键菜单
    void onTreeContextMenu(const QPoint &pos);
    void onActionDeleteCSV();   // 从树中移除单个CSV文件节点，同时移除图表中的曲线,由在CSV节点上右键菜单触发后调用

    // 视图控制 (接收 action 的 Check 状态)
    void onActionToggleTree(bool checked);   // 切换树形结构的显示状态
    void onActionToggleTable(bool checked);   // 切换表格的显示状态
    void onActionToggleProps(bool checked);   // 切换属性控件的显示状态

    // 帮助菜单
    void onActionGuide();   // 显示用户指南
    void onActionAbout();   // 显示关于信息
private:
    Ui::MainWindow *ui;
};


#endif //CSVREADER_MAINWINDOW_H
