#include "webcam.h"
#include "webserver.h"
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <atomic>
#include <qdebug.h>

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  WebCam camera;
  Webserver server(443);
  std::atomic<bool> isCameraReady{false};

  QObject::connect(&camera, &WebCam::cameraImageReady, &server,
                   &Webserver::onCameraImageAvailable);
  QObject::connect(&camera, &WebCam::cameraReadyChanged,
                   [&isCameraReady](bool ready) {
                     qDebug() << "Camera ready - " << ready;
                     isCameraReady = ready;
                   });

  QObject::connect(&server, &Webserver::commandReceived,
                   [&camera, &isCameraReady](const QString &command) {
                     qDebug() << "Command is: " << command;
                     if (command.contains("start")) {
                       camera.start();
                     }

                     if (command.contains("stop")) {
                       camera.stop();
                     }

                     if (command.contains("capture")) {
                       if (isCameraReady)
                         camera.capture();
                     }
                   });

  camera.start();

  return a.exec();
}
