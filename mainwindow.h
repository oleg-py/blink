#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class BlinkCore;
class QLineEdit;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    BlinkCore *core;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);

private:
    Ui::MainWindow *ui;
    void writePath(QLineEdit *le);

private slots:
    void onBlinkButtonPressed();
    void enableControls(bool enable);
    void purgeProgress();
    void onBlinkFinished();
    void onBlinkError(QString msg);
};

#endif // MAINWINDOW_H
