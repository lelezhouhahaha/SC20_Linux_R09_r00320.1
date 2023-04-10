#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpSocket>
#include <QProcess>

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

    lwm2mapp = new QtLWM2MApp();
    lwm2mapp->setUi(ui);
    lwm2mapp->connectTo();
}

/*=========================================================================================
 * @brief destructor
 ========================================================================================*/
MainWindow::~MainWindow()
{
    delete ui;
    delete lwm2mapp;
}

/*=========================================================================================
 * @brief the clicked event when you click the connect pushbutton and connect the server
 *  by socket.
 ========================================================================================*/
void MainWindow::on_bt_connect_server_clicked()
{
    qDebug() << "connect to LWClient!!!!";
    lwm2mapp->connectTo();
}

/*=========================================================================================
 * @brief the clicked event when you click the connect startLw2client pushbutton
 *  and disconnect the server.
 ========================================================================================*/
void MainWindow::on_bt_start_lw2client_clicked()
{
    lwm2mapp->disconnect();
    this->close();
 //   qDebug() << "connect to LW2Client!!!!";
   // QProcess process;
    //lw2client path.
   // process.setProgram("");
    //lw2client arguments.
   // process.setArguments(const QStringList & arguments)
//    process.start();
    //process.start("lwm2mclient");

    //wait for 30000ms
   // process.waitForStarted();
}

/*=========================================================================================
 * @brief the clicked event when you click the disconnect pushbutton and start LW2Client.
 ========================================================================================*/
void MainWindow::on_bt_disconnect_server_clicked()
{
    qDebug() << "Disconnect lw2client.";
    lwm2mapp->disconnect();
}

