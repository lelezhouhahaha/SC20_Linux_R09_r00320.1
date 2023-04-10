#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "tel_sdk.h"



int main(int argc, char *argv[])
{
    qmlRegisterType<tel_sdk>("tel_sdk",1,0,"Tel_sdk");
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    //c++与qml交互
    QObject *pRoot = engine.rootObjects().first();
	
    //tel_sdk *test=new tel_sdk();
    start(pRoot);
    QVariant returnedValue;
    QVariant msg = "message sended from C++";
    QMetaObject::invokeMethod(pRoot, "test", Q_RETURN_ARG(QVariant, returnedValue),Q_ARG(QVariant, msg));

    return app.exec();
}

