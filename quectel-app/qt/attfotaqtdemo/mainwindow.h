#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QtFotaApp.h"

//#define LW2_SERVER_ADDRESS  "10.88.110.119"
//#define LW2_SERVER_PORT     "8080"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_bt_connect_server_clicked();
    void on_bt_start_lw2client_clicked();
    void on_bt_disconnect_server_clicked();
    void on_bt_connect_server_2_clicked();
private:
    Ui::MainWindow *ui;
    QtFotaApp *Fotaapp;
};

#endif // MAINWINDOW_H
