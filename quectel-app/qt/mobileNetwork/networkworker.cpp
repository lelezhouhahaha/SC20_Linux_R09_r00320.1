#include "networkworker.h"

NetworkWorker::NetworkWorker(QObject *parent) : QObject(parent)
{
    m_qlrilHandle = 0;
}

void NetworkWorker::qlrilInit()
{
    int i = 0;
    int ret = 0;
    int handle = 0;
    char **resp = nullptr;
    QString message;

    if (m_qlrilHandle == 0) {
        qDebug() << "QLRIL_Init start...";
        for (i = 0; i < 5; i++) {
            ret = QLRIL_Init(&handle);
            if (ret == 0) {
                break;
            }
            QThread::sleep(1);
        }

        if (i >= 5 || handle == 0) {
            qDebug() << "QLRIL_Init failure";
            return;
        }

        m_qlrilHandle = handle;
        qDebug() << "QLRIL_Init success";
        emit sendMessage("QLRIL_Init success");
    }

    if (m_qlrilHandle != 0) {
        ret = QLRIL_GetDataRegistrationState(&handle, &resp);
        if (ret < 0) {
            qDebug() << "QLRIL_GetDataRegistrationState failed with return:" << ret;
        } else {
            /* "response[0] is registration state 0-5,"
            "e.g 1:Registered, 2: Not registered\n"
            "response[1] is LAC if registered or NULL if not\n"
            "response[2] is CID if registered or NULL if not\n"
            "response[3] indicates the available data radio technology, e.g 3:UMTS, 14:LTE\n"
            "valid values as defined by RIL_RadioTechnology.\n"
            "response[5] The maximum number of simultaneous Data Calls that can be established\n"
            "For more information refer to RIL_REQUEST_DATA_REGISTRATION_STATE in ril.h" */
            for (i = 0; i < ret && i < 14; i++) {
                if (resp[i] == nullptr)
                    continue;

                if (i == 0) {
                    message = (QString(resp[i]).compare("1") == 0)?
                                QString("Network registration"):QString("Network not registration");
                    emit sendMessage(message);
                } else if (i == 2) {
                    message = QString("Cell Id:") + QString(resp[i]);
                    emit sendMessage(message);
                } else if (i == 3) {
                    if (QString(resp[i]).compare("14") == 0) {
                        message = QString("Network type: 4G");
                    } else if (QString(resp[i]).compare("3") == 0)
                        message = QString("Network type: 3G");
                    else
                        message = QString("Network type: ") + QString(resp[i]);
                    emit sendMessage(message);
                }
                free(resp[i]);
            }

            if (resp) {
                free(resp);
                resp = nullptr;
            }
        }

        ret = getDataCallState();
        if (ret <= 0)
            emit syncNetworkState(0);
        else
            emit syncNetworkState(1);
    }

    return;
}

void NetworkWorker::setupDataCall()
{
    int ret = 0;
    int handle = 0;
    char *str = nullptr;
    char dnses[200] = {0};
    RIL_Data_Call_Response_v11 *dataCall = NULL;

    if (m_qlrilHandle == 0)
        return;

    handle = m_qlrilHandle;
    ret = getDataCallState();
    if (ret > 0) {
        emit sendMessage("Mobile network is active");
        return;
    }

    emit sendMessage("Network data call start...");

    ret = QLRIL_SetupDataCall(&handle, 14, 0, "", "", "", 0, "IP", (void **)&dataCall);
    if (ret > 0) {
        if (dataCall->active == 0) {
            qDebug("Network is not active, may be you can try 'others' mode");
        }

        emit sendMessage("Mobile network connect");

        if (dataCall->dnses != NULL && strlen(dataCall->dnses) > 0) {//FIXME maybe
            str = dataCall->dnses;
            qDebug() << "DNS string:" << str;
        } else {
            ret = snprintf(dnses, sizeof(dnses), "%s", "8.8.8.8");//for example
            ret += snprintf(dnses + ret, sizeof(dnses), " %s", "4.2.2.2");
            ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200::1");
            ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200:baba::1");
            str = dnses;
        }

        emit sendMessage(QString("DNS:") + QString::fromStdString(str));

        ret = QLRIL_SetDNSForIPV4V6(str);
        if (ret <= 0) {
            qDebug() << "QLRIL_SetDNSForIPV4V6 failure return:" << ret;
            emit sendMessage("Set DNS failure:" + QString(ret));
        } else {
            qDebug() << "QLRIL_SetDNSForIPV4V6 success";
        }

        if (dataCall->ifname != NULL && strlen(dataCall->ifname) > 0) {
            ret = QLRIL_SetRouteForIPV4V6(dataCall->ifname);
            if (ret < 0)
                qDebug() << "QLRIL_SetRouteForIPV4V6 failed with return:" << ret;
            else {
                qDebug() << "QLRIL_SetRouteForIPV4V6 success";
                emit sendMessage("IP route applied successfully");
            }
        } else
            qDebug() << "QLRIL_SetupDataCall called failed without interface name";

        if (dataCall->type)
            free(dataCall->type);
        if (dataCall->ifname) {
            emit sendMessage(QString("ifname:") +
                             QString::fromStdString(dataCall->ifname));
            free(dataCall->ifname);
        }
        if (dataCall->addresses) {
            emit sendMessage(QString("IP addresses:") +
                             QString::fromStdString(dataCall->addresses));
            free(dataCall->addresses);
        }
        if (dataCall->dnses)
            free(dataCall->dnses);
        if (dataCall->gateways) {
            emit sendMessage(QString("gateways:") +
                             QString::fromStdString(dataCall->gateways));
            free(dataCall->gateways);
        }
        if (dataCall->pcscf)
            free(dataCall->pcscf);

        if (dataCall) {
            free(dataCall);
            dataCall = NULL;
        }
    } else {
        qDebug() << "QLRIL_SetupDataCall failed with return:" << ret;
        emit sendMessage("setup data call failed:" + QString(ret));
    }

    emit sendMessage("Network data call started");

    return;
}

void NetworkWorker::deactivateDataCall()
{
    int i = 0;
    int ret = 0;
    int num = 0;
    int callId = 0;
    int handle = 0;
    RIL_Data_Call_Response_v11 *dataCallList = NULL;

    if (m_qlrilHandle == 0)
        return;

    handle = m_qlrilHandle;

    ret = QLRIL_GetDataCallList(&handle, (void **)&dataCallList);
    if (ret > 0) {
        if (ret >= 20) {
            qDebug("Data Call list items is too many "
                    "you shuld not run QLRIL_SetupDataCall() too many times "
                    "May be you shuld run QLRIL_DeactivateDataCall() to release some items");
        }

        if (dataCallList == NULL) {
            qDebug() << "QLRIL_GetDataCallList pointer is NULL";
            return;
        }

        num = ret;
        for (i = 0; i < num; i++) {
            callId = dataCallList[i].cid;
            ret = QLRIL_DeactivateDataCall(&handle, callId, 0);
            if (ret < 0) {
                qDebug() << "QLRIL_DeactivateDataCall failure:" << ret;
                emit sendMessage("Deactivated mobile network failure:" + QString::number(callId));
            }

            emit sendMessage("Deactivated mobile network:" + QString::number(callId));

            if (dataCallList[i].type)
                free(dataCallList[i].type);
            if (dataCallList[i].ifname)
                free(dataCallList[i].ifname);
            if (dataCallList[i].addresses)
                free(dataCallList[i].addresses);
            if (dataCallList[i].dnses)
                free(dataCallList[i].dnses);
            if (dataCallList[i].gateways)
                free(dataCallList[i].gateways);
            if (dataCallList[i].pcscf)
                free(dataCallList[i].pcscf);
        }
    } else if (ret == 0) {
        qDebug() << "No Data Call list items, May be you shuld run QLRIL_SetupDataCall()";
    } else {
        /*
         * -1047: 47 - RIL_E_INVALID_CALL_ID (Received invalid call id in request)
         */
        qDebug() << "QLRIL_GetDataCallList failed with return:" << ret;
    }

    if (dataCallList) {
        free(dataCallList);
        dataCallList = NULL;
    }

    return;
}

int NetworkWorker::getDataCallState()
{
    int i = 0;
    int ret = 0;
    int state = 0;
    int callId = 0;
    int handle = 0;
    RIL_Data_Call_Response_v11 *dataCallList = nullptr;

    if (m_qlrilHandle == 0)
        return -1;

    handle = m_qlrilHandle;

    ret = QLRIL_GetDataCallList(&handle, (void **)&dataCallList);
    if (ret > 0) {
        if (ret >= 20) {
            qDebug("Data Call list items is too many"
                   " you shuld not run QLRIL_SetupDataCall() too many times May be"
                   " you shuld run QLRIL_DeactivateDataCall() to release some items");
        }

        if (dataCallList == nullptr) {
            qDebug() << "QLRIL_GetDataCallList pointer is NULL";
            return -2;
        }

        for (i = ret - 1; i >= 0; i--) {
            if (dataCallList[i].active > 0) {
                state += 1;
                emit sendMessage(QString("Network is active:") + QString::number(state));
            }

            if (dataCallList[i].type)
                free(dataCallList[i].type);
            if (dataCallList[i].ifname) {
                emit sendMessage(QString("ifname: ") +
                                 QString::fromStdString(dataCallList[i].ifname));
                free(dataCallList[i].ifname);
            }
            if (dataCallList[i].addresses) {
                emit sendMessage(QString("IP address:") +
                                 QString::fromStdString(dataCallList[i].addresses));
                free(dataCallList[i].addresses);
            }
            if (dataCallList[i].dnses) {
                emit sendMessage(QString("DNS:") +
                                 QString::fromStdString(dataCallList[i].dnses));
                free(dataCallList[i].dnses);
            }
            if (dataCallList[i].gateways) {
                emit sendMessage(QString("gateways:") +
                                 QString::fromStdString(dataCallList[i].gateways));
                free(dataCallList[i].gateways);
            }
            if (dataCallList[i].pcscf)
                free(dataCallList[i].pcscf);
        }

        if (dataCallList != nullptr)
            free(dataCallList);
        dataCallList = NULL;
    } else if (ret == 0) {
        qDebug() << "No Data Call list items, May be you shuld run QLRIL_SetupDataCall()";
    } else {
        /*
         * -1047: 47 - RIL_E_INVALID_CALL_ID (Received invalid call id in request)
         */
        qDebug() << "QLRIL_GetDataCallList failed with return:" << ret;
    }

    if (state > 0) {
        return state;
    }

    return -3;
}
