/*
 * Copyright:  (C) 2020 quectel
 *             All rights reserved.
 *   Version:  1.0.0
 *    Author:  Fulinux/Peeta <peeta.chen@quectel.com>
 * ChangeLog:  1, Release initial version on 2020.09.28
 */
#ifndef NETWORKWOKER_H
#define NETWORKWOKER_H

#include <QObject>
#include <QDebug>
#include <QThread>

#include <RilPublic.h>

class NetworkWorker : public QObject
{
    Q_OBJECT
public:
    explicit NetworkWorker(QObject *parent = nullptr);

    int getDataCallState(void);

public slots:
    void qlrilInit(void);
    void setupDataCall(void);
    void deactivateDataCall(void);

signals:
    void sendMessage(QString msg);
    void syncNetworkState(int state);

private:
    int m_qlrilHandle;
};

#endif // NETWORKWOKER_H
