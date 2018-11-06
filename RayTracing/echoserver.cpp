#include "echoserver.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include "mesh.h"
#include "echoserver.h"
#include "tracer.h"
QT_USE_NAMESPACE

//! [constructor]
EchoServer::EchoServer(quint16 port, bool debug, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Echo Server"),
                                            QWebSocketServer::NonSecureMode, this)),
    m_debug(debug)
{
    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        if (m_debug)
            qDebug() << "Echoserver listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &EchoServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &EchoServer::closed);
    }
    updatemapMap();
}
//! [constructor]

EchoServer::~EchoServer()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

//! [onNewConnection]
void EchoServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &EchoServer::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &EchoServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &EchoServer::socketDisconnected);

    m_clients << pSocket;
}
//! [onNewConnection]

//! [processTextMessage]
void EchoServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Message received:" << message;
    if (pClient) {
        if(message == "1"){
            QJsonObject qjss=QJsonObject::fromVariantMap(mapMap);
            QJsonDocument doc(qjss);
            QString strJson(doc.toJson(QJsonDocument::Compact));
            pClient->sendTextMessage(strJson);
            //QString bound = '';
        }else if(message == "2"){
            QString data = rayTracing();

            pClient->sendTextMessage(data);
        }

    }
}
//! [processTextMessage]

//! [processBinaryMessage]
void EchoServer::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "Binary Message received:" << message;
    if (pClient) {
        pClient->sendBinaryMessage(message);
    }
}
//! [processBinaryMessage]

//! [socketDisconnected]
void EchoServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
//! [socketDisconnected]

QString EchoServer::rayTracing(){
    FilePoint *filePoint = new FilePoint();
    FileManager *fileManager = new FileManager();
    fileManager->readDbfFile(filePoint);
    fileManager->readShpFile(filePoint);
    qDebug() <<"xmax:"<<filePoint->Xmax;
    qDebug() <<"xmin"<<filePoint->Xmin;
    qDebug() <<"ymax"<<filePoint->Ymax;
    qDebug() <<"ymin"<<filePoint->Ymin;
    filePoint->uniformlize();
    Scene* scene = new Scene(filePoint->allPointList, filePoint->index);
    Node* rx = new Node(13.5, 6.2, 0.0, true);
    Mesh* mesh = new Mesh(30, scene, rx);
    Tracer*  tracer = new Tracer(mesh);
    Node* source = nullptr;
    //source = new  Node(14.5, 7.5, 16);
    //tracer->traceAll(source);
    for (double i = 0; i < 360;i+= 2)
    {
        source = new Node(14.5, 7.5,i);
        tracer->srcList.push_back(source);
        source->vSpread[0] = -90 * 3.14 / 180;
        source->vSpread[1] = 90 * 3.14 / 180;
        source->type = Tx;
        source->z = 0.5;
        source->range[0] = source->range[1] = 0.2;
        tracer->traceAll(source);
        //qDebug() << "i:" << i ;
    }

    QMap<QString,QVariant> data;
    for(int i = 0; i < tracer->allPath.size(); i++){
        Path* path = tracer->allPath[i];
        list<Node*> nodes = path->nodes;
        list<Node*>:: iterator iter;
        QList<QVariant> list;
        for(iter = nodes.begin(); iter != nodes.end(); iter++){
            QMap<QString,QVariant> map;
            Node* node = *iter;
            map.insert("x", (node->x / mesh->size - 0.5) * 50);
            map.insert("y", (node->y /mesh->size- 0.5) * 50);
            map.insert("z", (node->z /mesh->size) * 30);
            list.append(QVariant(map));
        }
        data.insert(QString::number(i,10),QVariant(list));
    }
    QJsonObject qjss=QJsonObject::fromVariantMap(data);
    QJsonDocument doc(qjss);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}


void EchoServer::updatemapMap(){
    FilePoint *filePoint = new FilePoint();
    FileManager *fileManager = new FileManager();
    fileManager->readDbfFile(filePoint);
    fileManager->readShpFile(filePoint);
    qDebug() <<"xmax:"<<filePoint->Xmax;
    qDebug() <<"xmin"<<filePoint->Xmin;
    qDebug() <<"ymax"<<filePoint->Ymax;
    qDebug() <<"ymin"<<filePoint->Ymin;
    QMap<QString,QVariant> boundMap;
    xmax =filePoint->Xmax;
    xmin =filePoint->Xmin;
    ymax =filePoint->Ymax;
    ymin =filePoint->Ymin;
    filePoint->uniformlize();//(0,1)
    Scene *scene = new Scene(filePoint->allPointList, filePoint->index);
    QJsonObject qjs;
    for(int i = 0; i < scene->objList.size(); i++){
        QMap<QString,QVariant> myMap;
        QList<QVariant> posList;
        for(int j=0;j<scene->objList[i]->pointList.size();j++){
            QMap<QString,QVariant> tempMap;
            tempMap.insert(QString("x"),QVariant((scene->objList[i]->pointList[j]->x - 0.5)*50));
            tempMap.insert(QString("y"),QVariant((scene->objList[i]->pointList[j]->y - 0.5)*50));
            tempMap.insert(QString("z"),QVariant(scene->objList[i]->pointList[j]->z *15) );
            posList.append(QVariant(tempMap));
        }

        mapMap.insert(QString::number(i,10),QVariant(posList));
    }

}
