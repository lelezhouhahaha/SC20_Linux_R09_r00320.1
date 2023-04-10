/*
 * Copyright:  (C) 2020 quectel
 *             All rights reserved.
 *   Version:  1.0.0
 *    Author:  Fulinux/Peeta <peeta.chen@quectel.com>
 * ChangeLog:  1, Release initial version on 2020.09.28
 */
#include "mobilenetwork.h"

MobileNetwork::MobileNetwork(QObject *parent) : QObject(parent)
{
    QStringList list;
    qDebug() << "Mobile Network Start";

//    list << "start" << "end";

//    setInfoList(list);

//    appendInfoList("test");

//    connect(&m_timer, SIGNAL(timeout()), this, SLOT(addOneString()));
//    m_timer.start(3000);

    p_worker = new NetworkWorker();
    p_worker->moveToThread(&m_thread);
    connect(this, &MobileNetwork::mobileNetworkInit, p_worker, &NetworkWorker::qlrilInit);
    connect(this, &MobileNetwork::enableNetwork, p_worker, &NetworkWorker::setupDataCall);
    connect(this, &MobileNetwork::disableNetwork, p_worker, &NetworkWorker::deactivateDataCall);

    connect(p_worker, &NetworkWorker::sendMessage, this, &MobileNetwork::addMessageToList);
    connect(p_worker, &NetworkWorker::syncNetworkState, this, &MobileNetwork::syncNetworkState);

    m_thread.start();
    emit mobileNetworkInit();
}

QString MobileNetwork::getInfoAt(int index)
{
    return infoList().at(index);
}

int MobileNetwork::sendSignalFromQML(int code)
{
    qDebug() << "Receive signal code:" << code;

    switch (code) {
    case 0:
        emit mobileNetworkInit();
        break;
    case 1:
        emit enableNetwork();
        break;
    case 2:
        emit disableNetwork();
        break;
    default:
        break;
    }

    return 0;
}

void MobileNetwork::exitProgram()
{
    m_thread.exit();
    if (p_worker)
        p_worker->deleteLater();
}

int MobileNetwork::appendInfoList(QString info)
{
    if (info.isNull())
        return -1;

    if (m_infoList.size() > 15) {
        m_infoList.removeAt(0);
    }

    m_infoList.append(info);

    emit infoListChanged(infoList());

    return 0;
}

void MobileNetwork::addOneString()
{
    m_timer.stop();
    appendInfoList("haha");
    if (!m_infoList.at(0).isEmpty())
        m_infoList.removeAt(0);
    m_infoList.insert(0, "start");
    emit infoListChanged(infoList());
}

void MobileNetwork::addMessageToList(QString msg)
{
    appendInfoList(msg);
}
