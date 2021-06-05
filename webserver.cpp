#include "webserver.h"
#include <QBuffer>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <qdebug.h>

QT_USE_NAMESPACE

Webserver::Webserver(quint16 port, QObject *parent)
    : QObject(parent), m_pWebSocketServer(nullptr) {
  m_pWebSocketServer = new QWebSocketServer(QStringLiteral("Camera Server"),
                                            QWebSocketServer::SecureMode, this);
  QSslConfiguration sslConfiguration;
  QFile certFile(QStringLiteral("minica.pem"));
  QFile keyFile(QStringLiteral("minica-key.pem"));
  certFile.open(QIODevice::ReadOnly);
  keyFile.open(QIODevice::ReadOnly);
  QSslCertificate certificate(&certFile, QSsl::Pem);
  QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
  certFile.close();
  keyFile.close();
  sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
  sslConfiguration.setLocalCertificate(certificate);
  sslConfiguration.setPrivateKey(sslKey);
  sslConfiguration.setProtocol(QSsl::TlsV1SslV3);
  m_pWebSocketServer->setSslConfiguration(sslConfiguration);
  connect(m_pWebSocketServer, &QWebSocketServer::sslErrors, this,
          &Webserver::onSslErrors);
  connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this,
          &Webserver::onNewConnection);

  if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
    qDebug() << "SSL Server listening on port" << port;
  }
}

Webserver::~Webserver() {
  m_pWebSocketServer->close();
  qDeleteAll(m_clients.begin(), m_clients.end());
}

void Webserver::onCameraImageAvailable(const QImage &image) {
  QByteArray ba;
  QBuffer buffer;
  buffer.open(QIODevice::WriteOnly);
  image.save(&buffer, "JPG");
  QString message = QString(buffer.data().toBase64());

  lock.lockForRead();
  foreach (QWebSocket *client, m_clients) { client->sendTextMessage(message); }
  lock.unlock();
}

void Webserver::onNewConnection() {
  QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

  qDebug() << "Client connected:" << pSocket->peerName() << pSocket->origin();

  connect(pSocket, &QWebSocket::disconnected, this,
          &Webserver::socketDisconnected);

  connect(pSocket, &QWebSocket::textMessageReceived, this,
          &Webserver::onTextMessageReceived);

  pSocket->ignoreSslErrors();

  m_clients << pSocket;
}

void Webserver::socketDisconnected() {
  qDebug() << "Client disconnected";
  QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
  if (pClient) {
    lock.lockForWrite();
    m_clients.removeAll(pClient);
    pClient->deleteLater();
    lock.unlock();
  }
}

void Webserver::onTextMessageReceived(const QString &message) {
  qDebug() << "Message received: " << message;
  emit commandReceived(message);
}

void Webserver::onSslErrors(const QList<QSslError> &errors) {
  qDebug() << "Ssl errors occurred: " << errors;

}
