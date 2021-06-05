#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QImage>
#include <QObject>
#include <QReadWriteLock>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtNetwork/QSslError>
#include <QBuffer>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class Webserver : public QObject {

  Q_OBJECT
public:
  explicit Webserver(quint16 port, QObject *parent = nullptr);
  ~Webserver();

Q_SIGNALS:
    void commandReceived(const QString& command);

public Q_SLOTS:
  void onCameraImageAvailable(const QImage &image);

private Q_SLOTS:
  void onNewConnection();
  void socketDisconnected();
  void onTextMessageReceived(const QString &message);
  void onSslErrors(const QList<QSslError> &errors);
private:
  QWebSocketServer *m_pWebSocketServer;
  QList<QWebSocket *> m_clients;
  QReadWriteLock lock;

};

#endif // WEBSERVER_H
