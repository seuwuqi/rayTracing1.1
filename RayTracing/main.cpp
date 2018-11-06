#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtWebEngine/qtwebengineglobal.h>
#include "mesh.h"
#include "echoserver.h"
#include "tracer.h"
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QtWebEngine::initialize();

//    QQmlApplicationEngine engine;
//    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    EchoServer *server = new EchoServer(1234, true);

    QObject::connect(server, &EchoServer::closed, &app, &QGuiApplication::quit);


    return app.exec();
}


