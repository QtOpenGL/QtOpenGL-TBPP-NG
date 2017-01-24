// 主窗口

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>
#include <QString>
#include <QThread>
#include <QTimer>
#include "backend.h"
#include "libs/jsoncpp.h"
#include "statwindow.h"

const int AUTO_SAVE_INTERVAL = 60000;  // 自动保存的间隔（毫秒）

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

   public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

   private:
    Ui::MainWindow *ui;
    StatWindow statWindow;
    QThread backendThread;
    QTimer *timer;
    QString fileName;  // 当前打开的数据路径
    int lastAutoSaveTime;
    bool inited;  // 是否初始化

    void loadData();
    void saveData();

   public slots:
    void animate();
    void showMsg(QString s);
    void on_actionInit_triggered();                // 初始化
    void on_actionOpen_triggered();                // 打开数据
    void on_actionSave_triggered();                // 保存数据
    void on_actionSaveAs_triggered();              // 另存为数据
    void on_actionImportStg_triggered();           // 导入策略数据
    void on_actionExportStg_triggered();           // 导出策略数据
    void on_actionSpot_triggered();                // 查看选中的星球
    void on_actionReset_triggered();               // 查看中央位置
    void on_actionPause_toggled(bool b);           // 开始/暂停模拟
    void on_actionSlow_toggled(bool b);            // 慢速模拟
    void on_actionShowFriendship_toggled(bool b);  // 显示外交参数
    void on_actionShowParent_toggled(bool b);      // 显示母星
    void on_actionShowFleet_toggled(bool b);       // 显示舰队
    void on_actionStat_triggered();                // 显示统计图
    void on_actionAbout_triggered();               // 版权信息

   signals:
    backendWork();
public slots:
    void initComplete();
};

#endif
