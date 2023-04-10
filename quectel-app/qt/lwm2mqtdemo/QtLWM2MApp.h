#ifndef LWM2MCLIENT_H
#define LWM2MCLIENT_H

#include <QObject>
#include <qtcpsocket.h>
#include <QString>
#include <QByteArray>
#include "ui_mainwindow.h"

class QtLWM2MApp : public QObject
{
    Q_OBJECT

public:
    explicit QtLWM2MApp(QObject *parent = nullptr);
    ~QtLWM2MApp();

public slots:
    void connectTo();
    void disconnect();
    void send(QByteArray data);
    void setUi(Ui::MainWindow *ui);

private slots:
    void readSocket();
    void handleMessage(QString msg);
    void updateDisplay(QString msg);

private:
    QTcpSocket *socket;
    Ui::MainWindow *m_Ui;
};

#endif // LWM2MCLIENT_H
