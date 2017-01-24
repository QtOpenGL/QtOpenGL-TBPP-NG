// 统计窗口

#include "statwindow.h"
#include "ui_statwindow.h"

const int MAX_DATA_FUNC = 12;

const function<double(int)> dataFunc[MAX_DATA_FUNC] = {
    [](int i) -> double { return double(i); },
    [](int i) -> double { return civils[i].birthTime; },
    [](int i) -> double {
        return civils[i].deathTime >= 0
                   ? civils[i].deathTime - civils[i].birthTime
                   : space.clock - civils[i].birthTime;
    },
    [](int i) -> double { return civils[i].tech; },
    [](int i) -> double { return civils[i].timeScale; },
    [](int i) -> double { return civils[i].childCivilCount; },
    [](int i) -> double {
        civils[i].normalizeRate();
        return abs(civils[i].rateDev);
    },
    [](int i) -> double {
        civils[i].normalizeRate();
        return abs(civils[i].rateAtk);
    },
    [](int i) -> double {
        civils[i].normalizeRate();
        return abs(civils[i].rateCoop);
    },
    [](int i) -> double {
        return double(civils[i].devCount) /
               double(civils[i].devCount + civils[i].atkCount +
                      civils[i].coopCount);
    },
    [](int i) -> double {
        return double(civils[i].atkCount) /
               double(civils[i].devCount + civils[i].atkCount +
                      civils[i].coopCount);
    },
    [](int i) -> double {
        return double(civils[i].coopCount) /
               double(civils[i].devCount + civils[i].atkCount +
                      civils[i].coopCount);
    },
};

const QString dataLabel[MAX_DATA_FUNC] = {
    "文明编号", "诞生时间", "存活时间", "科技水平", "时间曲率", "子文明数",
    "发展倾向", "攻击倾向", "合作倾向", "发展比例", "攻击比例", "合作比例"};

StatWindow::StatWindow(QWidget* parent)
    : QWidget(parent), ui(new Ui::statWindow), pointColor(Qt::red)
{
    ui->setupUi(this);
    for (int i = 0; i < MAX_DATA_FUNC; ++i)
    {
        ui->comboBox->addItem(dataLabel[i]);
        ui->comboBox_2->addItem(dataLabel[i]);
    }
    ui->comboBox->setCurrentIndex(0);
    ui->comboBox_2->setCurrentIndex(3);
}

StatWindow::~StatWindow()
{
    delete ui;
}

void StatWindow::paintStat()
{
    // 调节取点的数量和间隔
    // 取点不超过10000个
    int maxSample = 10000;
    double step;
    if (size_t(maxSample) > civils.size())
    {
        maxSample = civils.size();
        step = 1.0;
    }
    else
    {
        step = double(civils.size()) / double(maxSample);
    }

    QVector<double> x(maxSample), y(maxSample);
    double minX, minY, maxX, maxY;
    minX = minY = numeric_limits<double>::infinity();
    maxX = maxY = 0.0;
    for (int i = 0; i < maxSample; ++i)
    {
        x[i] = dataFunc[ui->comboBox->currentIndex()](floor(i * step));
        if (ui->checkBox->checkState() == Qt::Checked) x[i] = log(x[i] + 1.0);
        y[i] = dataFunc[ui->comboBox_2->currentIndex()](floor(i * step));
        if (ui->checkBox_2->checkState() == Qt::Checked) y[i] = log(y[i] + 1.0);
        minX = min(minX, x[i]);
        minY = min(minY, y[i]);
        maxX = max(maxX, x[i]);
        maxY = max(maxY, y[i]);
    }

    ui->qCustomPlot->addGraph();
    ui->qCustomPlot->graph(0)->setData(x, y);
    ui->qCustomPlot->graph(0)->setPen(QPen(pointColor));
    ui->qCustomPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->qCustomPlot->graph(0)->setScatterStyle(
        QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    ui->qCustomPlot->xAxis->setLabel(dataLabel[ui->comboBox->currentIndex()]);
    ui->qCustomPlot->yAxis->setLabel(dataLabel[ui->comboBox_2->currentIndex()]);
    ui->qCustomPlot->xAxis->setRange(minX, maxX);
    ui->qCustomPlot->yAxis->setRange(minY, maxY);

    ui->qCustomPlot->replot();
}

void StatWindow::on_pushButton_clicked()
{
    paintStat();
}
