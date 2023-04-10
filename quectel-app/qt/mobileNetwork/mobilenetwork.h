/*
 * Copyright:  (C) 2020 quectel
 *             All rights reserved.
 *   Version:  1.0.0
 *    Author:  Fulinux/Peeta <peeta.chen@quectel.com>
 * ChangeLog:  1, Release initial version on 2020.09.28
 */
#ifndef MOBILENETWORK_H
#define MOBILENETWORK_H

#include <QObject>
#include <QTimer>
#include <QDebug>

#include <networkworker.h>

class MobileNetwork : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList infoList READ infoList WRITE setInfoList NOTIFY infoListChanged)
public:
    explicit MobileNetwork(QObject *parent = nullptr);

    Q_INVOKABLE QString getInfoAt(int index);
    Q_INVOKABLE int sendSignalFromQML(int code);
    Q_INVOKABLE void exitProgram(void);

    int appendInfoList(QString info);

    QStringList infoList() const
    {
        return m_infoList;
    }

public slots:
    void setInfoList(QStringList infoList)
    {
        if (m_infoList == infoList)
            return;

        m_infoList = infoList;
        emit infoListChanged(m_infoList);
    }

    void addOneString();
    void addMessageToList(QString msg);

signals:
    void infoListChanged(QStringList infoList);
    void mobileNetworkInit(void);
    void enableNetwork(void);
    void disableNetwork(void);
    void syncNetworkState(int state);

private:
    QStringList m_infoList;
    QTimer m_timer;
    QThread m_thread;
    NetworkWorker *p_worker;
};

#endif // MOBILENETWORK_H
