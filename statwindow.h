// 统计窗口

#ifndef STATWINDOW_H
#define STATWINDOW_H

#include <QString>
#include <QVector>
#include <QWidget>
#include "civil.h"
#include "libs/qcustomplot.h"

namespace Ui
{
    class statWindow;
}

class StatWindow : public QWidget
{
    Q_OBJECT

   public:
    StatWindow(QWidget *parent = 0);
    ~StatWindow();

   private:
    Ui::statWindow *ui;
    Qt::GlobalColor pointColor;

    void paintStat();

   public slots:
    void on_pushButton_clicked();
};

#endif
