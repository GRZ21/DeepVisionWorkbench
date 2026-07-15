//
// Created by GRZ on 2026/7/9.
//

#ifndef CSVREADER_CSVANALYZEWIDGET_H
#define CSVREADER_CSVANALYZEWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QTreeWidgetItem>
#include <QMap>
#include "../utils/qcustomplot.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class CsvAnalyzeWidget;
}

QT_END_NAMESPACE

class CsvAnalyzeWidget : public QWidget {
    Q_OBJECT

public:
    explicit CsvAnalyzeWidget(QWidget *parent = nullptr);

    ~CsvAnalyzeWidget() override;

    QMap<QString,QString>csvInfo();
private:
    enum treeItemType{itTopItem=1001,itCSVItem,itParamItem};
    enum treeColNum{colItem=0,colDate};

    void buildTreeHeader();
    void initTree();

    QTreeWidgetItem* addCSVItem(QTreeWidgetItem* parItem,QString fileName);
    void addParmItem(QTreeWidgetItem* parItem,QString CSVHeader,int colIndex);
    void loadCSVCache(QString fileName);
    void highlightColumn(int columIndex);
    void highlightTableParam(QModelIndex& columIndex);

    void poltCsvColumn(QTreeWidgetItem* item,int columIndex);
    void drawOnCustomPlot(QString name,QVector<double>x,QVector<double>y,QTreeWidgetItem* item);
    void onParmItemChanged(QTreeWidgetItem* item,int columIndex);
    void removeCurveFromPlot(QTreeWidgetItem*  item);

    void updateCurveStats(QCPGraph *graph);

    QStandardItemModel* tableModel;
    QItemSelectionModel* tableSelectionModel;
    QMap<QString,QStringList>csvCache;
    QString currentCSVFileName;  // 记录当前显示的CSV文件名
    QHash<QCPGraph*,QTreeWidgetItem*> curveToItemMap;
    QCPGraph* curGraph = nullptr;

public slots:
    void openCsvFile();
    void openCsvDirectory();
    void clearPlot();
    void removeAllFiles();
    void deleteCsv();
    void exportChart();
    void restoreLayout();

    void setFileTreeVisible(bool checked);
    void setDataTableVisible(bool checked);
    void setPropertyPanelVisible(bool checked);

private slots:
    void csvTableDisplay(QStringList&  data);
    void onTreeItemClicked(QTreeWidgetItem* item,int column);
    void onTreeItemDoubleClicked(QTreeWidgetItem* item,int column);
    void onCurveClicked(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);
    void autoRescalePlot();
    void onEditName(const QString& name);
    void onSpinWidth(int width);
    void onSpinAlpha(int alpha);
    void onPropColorClicked();
    void onComboStyleChanged(int index);
    void onComboScatterChanged(int index);
    void onBtnBringTopClicked();
    void onBtnSendBottomClicked();
    void onComboLegendChanged(int index);
    void updateGlobalFont();
    void onComboGridChanged(int index);
    void onSpinScatterSizeChanged(int size);
    void onAxisTitleChanged();
    void removeSingleCSV(QTreeWidgetItem* csvItem);

    void onTreeContextMenu(const QPoint &pos);
private:
    Ui::CsvAnalyzeWidget *ui;
};


#endif //CSVREADER_CSVANALYZEWIDGET_H
