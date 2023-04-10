#include "QtLWM2MApp.h"
#include <QtDebug>
#include <QString>
#include <QMessageBox>
#include <QHostAddress>
#include <QScrollBar>
/*=========================================================================================
 *                          CONSTANT DEFINITION
 ========================================================================================*/
#define LW2CLIENT_SERVER_ADDRESS    "127.0.0.1"
#define LW2CLIENT_SERVER_PORT       56800

/*=========================================================================================
 * @brief constructor.
 ========================================================================================*/
QtLWM2MApp::QtLWM2MApp(QObject *parent) :
    QObject(parent)
{
    qDebug() << "LW2test Client started";
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(connected()), this, SLOT(connectTo()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnect()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
}

/*=========================================================================================
 * @brief destructor.
 ========================================================================================*/
QtLWM2MApp::~QtLWM2MApp(void)
{
    if (socket->isOpen())
    {
        socket->close();
    }

    delete m_Ui;
    delete socket;
}

/*=========================================================================================
 * @brief init Ui.
 ========================================================================================*/
void QtLWM2MApp::setUi(Ui::MainWindow *ui)
{
    m_Ui = ui;
}

/*=========================================================================================
 * @brief connect socket.
 ========================================================================================*/
void QtLWM2MApp::connectTo()
{
    qDebug() << "LW2test connectTo target server.";
//    if used local socket, use QHostAddress::LocalHost
//    instand of LW2CLIENT_SERVER_ADDRESS
    socket->connectToHost(QHostAddress::LocalHost, LW2CLIENT_SERVER_PORT);
  //  socket->connectToHost("127.0.0.1", LW2CLIENT_SERVER_PORT);

    if (socket->waitForConnected())
    {
        qDebug() << "Connect to server.";
        updateDisplay("Connect to LWClient Success !");
    }
    else
    {
        qDebug() << "Can't connect to server.";
        updateDisplay("Connect to LWClient fail !");
    }
}


/*=========================================================================================
 * @brief disconnect socket.
 ========================================================================================*/
void QtLWM2MApp::disconnect()
{
    if (socket->isOpen())
    {
        socket->close();
    }
}

/*=========================================================================================
 * @brief send data to server by socket
 * @param data which you want to send to server.
 ========================================================================================*/
void QtLWM2MApp::send(QByteArray data)
{
//    qint64 write(const char *data, qint64 len);
//    qint64 write(const char *data);
//    inline qint64 write(const QByteArray &data)
//    { return write(data.constData(), data.size()); }
    if (socket && socket->isOpen())
    {
        socket->write(data);
    }
}


/*=========================================================================================
 * @brief read data from socket
 ========================================================================================*/
void QtLWM2MApp::readSocket()
{

    QByteArray block = socket->readAll();

        QDataStream in(&block, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_5_9);

        qDebug() << "readSocket:blocklength = " << block.length();
        QString s_received;
        s_received.prepend(block);
        handleMessage(s_received);
}

/*=========================================================================================
 * @brief handle the data that get from server.
 * @param msg which received from server.
 ========================================================================================*/
void QtLWM2MApp::handleMessage(QString msg)
{
    qDebug() << "handleMessage:msg = " << msg;
    updateDisplay(msg);
}



/*=========================================================================================
 * @brief update display(Label etc.) if need.
 ========================================================================================*/
void QtLWM2MApp::updateDisplay(QString msg)
{
    if (!m_Ui)
    {
        qDebug() << "handleMessage: m_Ui is null!" ;
        return;
    }
    m_Ui->text_info_content->insertPlainText("\n");
    m_Ui->text_info_content->insertPlainText(msg);
    m_Ui->text_info_content->insertPlainText("\n");
    QScrollBar *vScrollBar = m_Ui->text_info_content->verticalScrollBar();
    if (vScrollBar != NULL)
    {
        vScrollBar->setValue(vScrollBar->maximum());
    }
}
