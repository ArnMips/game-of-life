#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <omp.h>

namespace constants {
const char START_BTM_TEXT_RUN[] = "Run";
const char START_BTM_TEXT_STOP[] = "Stop";

}

std::string row_col_to_str(int row, int col){
    return std::to_string(row).append(",").append(std::to_string(col));
}

std::string row_col_to_str(const MainWindow::Cell& cell){
    return row_col_to_str(cell.row, cell.col);
}

QPoint norm_point(const QPoint& p, const QSize& bounds) {
    int newX = p.x() % bounds.width();
    int newY = p.y() % bounds.height();
    if (newX < 0)
        newX += bounds.width();
    if (newY < 0)
        newY += bounds.height();

    return QPoint(newX, newY);
}

int count_neighbours(const QSize& bounds, const QSet<QPoint> & points, const QPoint& p) {
    int neighbours = 0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            QPoint neighbourPoint = norm_point(p + QPoint(x, y), bounds);
            if (neighbourPoint == p)
                continue;

            if(points.find(neighbourPoint) != points.end())
                ++neighbours;
        }
    }
    return neighbours;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->runButton->setText(constants::START_BTM_TEXT_RUN);

    connect(&timer, &QTimer::timeout, this, &MainWindow::updateField2);
    connect(ui->timerSlider, &QSlider::valueChanged, this, &MainWindow::onTimerSliderChanged);
    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &MainWindow::onCellChanged);
    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::onRunButtomPressed);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::clearField);
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::resizeField);
    connect(ui->drawModeSelector, &QComboBox::currentTextChanged, this, &MainWindow::selectDrawMode);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onCellChanged(int row, int column)
{
    if(auto* oldCell = ui->tableWidget->takeItem(row, column)){
    }
//    if(auto* oldCell = ui->tableWidget->item(row, column)){
//        if(oldCell->background().color() == Qt::black) {
//            oldCell->setBackground(Qt::white);
//        } else {
//            oldCell->setBackground(Qt::black);
//        }
//    }
    else {
        auto* newCell = new QTableWidgetItem();
        newCell->setBackground(Qt::black);
        ui->tableWidget->setItem(row, column, newCell);
    }
}

void MainWindow::onRunButtomPressed()
{
    if(ui->runButton->isChecked()) {
        points_.clear();
        for(const auto* item : ui->tableWidget->findItems("*", Qt::MatchWildcard)){
            if (item) {
                int x = item->column();
                int y = item->row();
                points_.insert({x, y});
            }
        }
        ui->tableWidget->setEnabled(false);
        ui->runButton->setText(constants::START_BTM_TEXT_STOP);
        ui->runButton->setChecked(true);

        timer.start(ui->timerSlider->value());
    } else {
        ui->tableWidget->setEnabled(true);
        ui->runButton->setText(constants::START_BTM_TEXT_RUN);
        ui->runButton->setChecked(false);
        timer.stop();
    }
}

void MainWindow::onTimerSliderChanged()
{
    timer.setInterval(ui->timerSlider->value());
}

void MainWindow::selectDrawMode(const QString& drawMode)
{
 if (drawMode == "Pointer") {
    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &MainWindow::onCellChanged);
    disconnect(ui->tableWidget, &QTableWidget::cellEntered, this, &MainWindow::onCellChanged);
 } else if (drawMode == "Brush") {
    disconnect(ui->tableWidget, &QTableWidget::cellClicked, this, &MainWindow::onCellChanged);
    connect(ui->tableWidget, &QTableWidget::cellEntered, this, &MainWindow::onCellChanged);
 }
}

void MainWindow::drawField()
{
    ui->tableWidget->clear();
    for(const auto& point : points_){
        auto* newCell = new QTableWidgetItem();
        newCell->setBackground(Qt::black);
        ui->tableWidget->setItem(point.y(), point.x(), newCell);
    }
    ui->itemLabelNum->setText(QString::number(points_.size()));
}

void MainWindow::clearField()
{
    if(ui->runButton->isChecked()) {
        ui->runButton->click();
    }

    points_.clear();
    ui->tableWidget->clear();
    ui->itemLabelNum->setText(QString::number(0));
}

void MainWindow::resizeField(int coef)
{
    const int cellSize =  std::min(ui->tableWidget->width() / coef, ui->tableWidget->height() / coef);

    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(cellSize);
    ui->tableWidget->horizontalHeader()->setMinimumSectionSize(cellSize);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(cellSize);
    ui->tableWidget->verticalHeader()->setMinimumSectionSize(cellSize);

    ui->tableWidget->setRowCount(ui->tableWidget->height() / cellSize);
    ui->tableWidget->setColumnCount(ui->tableWidget->width() / cellSize);
}

void MainWindow::updateField2()
{
    decltype (points_) newLivingCells;

    const auto xCount = ui->tableWidget->columnCount();
    const auto yCount = ui->tableWidget->rowCount();
    const QSize bounds(xCount, yCount);

    #pragma omp parallel for shared(newLivingCells, bounds, points_)
    for (int i = 0; i < points_.size(); ++i) {
        auto it = points_.begin();
        for (int ik = 0; ik < i; ++ik) {
            ++it;
        }

        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {

                QPoint newPoint = norm_point(*it + QPoint(x, y), bounds);

                int neighbours = count_neighbours(bounds, points_, newPoint);
                bool hasLife = points_.find(newPoint) != points_.end();

                if ((hasLife && neighbours == 2) || neighbours == 3) {
                    #pragma omp critical
                    {
                        newLivingCells.insert(newPoint);
                    }
                }
            }
        }
    }
    points_ = newLivingCells;

    drawField();
}
