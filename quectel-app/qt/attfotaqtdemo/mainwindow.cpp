#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpSocket>
#include <QProcess>
//#include <synchapi.h>
#include <unistd.h>
/*=========================================================================================
 * @brief constructor
 ========================================================================================*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("QtLW2Test");
    //设置屏幕尺寸
    setFixedSize(720, 1280);

    Fotaapp = new QtFotaApp();
    Fotaapp->setUi(ui);

    QProcess process;
    //process.start("/usr/bin/attfotademo", nullptr);
    //process.waitForStarted();
    Fotaapp->connectTo();
    //Sleep(5000);
}

/*=========================================================================================
 * @brief destructor
 ========================================================================================*/
MainWindow::~MainWindow()
{
    delete ui;
    delete Fotaapp;
}

/*=========================================================================================
 * @brief the clicked event when you click the connect pushbutton and connect the server
 *  by socket.
 ========================================================================================*/
void MainWindow::on_bt_connect_server_clicked()
{
    Fotaapp->connectTo();
    Fotaapp->send("fotaupgrade");
}

/*=========================================================================================
 * @brief the clicked event when you click the connect pushbutton and connect the server
 *  by socket.
 ========================================================================================*/
void MainWindow::on_bt_connect_server_2_clicked()
{
    Fotaapp->disconnect();
    this->close();
}
/*=========================================================================================
 * @brief the clicked event when you click the connect startLw2client pushbutton
 *  and disconnect the server.
 ========================================================================================*/
void MainWindow::on_bt_start_lw2client_clicked()
{
    Fotaapp->connectTo();
    Fotaapp->send("fotadownload");
}

/*=========================================================================================
 * @brief the clicked event when you click the disconnect pushbutton and start LW2Client.
 ========================================================================================*/
void MainWindow::on_bt_disconnect_server_clicked()
{
    qDebug() << "Disconnect lw2client.";
    Fotaapp->disconnect();
}

