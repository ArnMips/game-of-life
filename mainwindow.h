#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <vector>
#include <QSet>
#include <QTimer>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    struct Cell { int row; int col; };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCellChanged(int row, int column);
    void onRunButtomPressed();
    void onTimerSliderChanged();

    void selectDrawMode(const QString &drawMode);

    void drawField();
    void clearField();
    void resizeField(int coef);


private:
    void updateField2();

private:
    Ui::MainWindow *ui;
    QSet<QPoint> points_;

    QTimer timer;

    bool enableTor = true;
};
#endif // MAINWINDOW_H
