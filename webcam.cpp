#include "webcam.h"
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <qdebug.h>

WebCam::WebCam(QObject *parent) : QObject(parent) {
  QCameraInfo cameraInfo = QCameraInfo::defaultCamera();
  if (cameraInfo.isNull()) {
    qWarning() << "No camera found";
  }
  if (cameraInfo.position() == QCamera::FrontFace)
    qDebug() << "The camera is on the front face of the hardware system.";
  else if (cameraInfo.position() == QCamera::BackFace)
    qDebug() << "The camera is on the back face of the hardware system.";
  qDebug() << "The camera sensor orientation is " << cameraInfo.orientation()
           << " degrees.";

  camera = new QCamera(cameraInfo);
  camera->setCaptureMode(QCamera::CaptureStillImage);
  imageCapture = new QCameraImageCapture(camera);
  if (imageCapture->isCaptureDestinationSupported(
          QCameraImageCapture::CaptureToBuffer)) {
    imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
  } else {
    imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
  }
  qDebug() << "Capture destination: " << imageCapture->captureDestination();
  connect(imageCapture, &QCameraImageCapture::imageAvailable, this,
          &WebCam::onCameraImageAvail);
  connect(imageCapture, &QCameraImageCapture::imageCaptured, this,
          &WebCam::onCameraImageCaptured);
  connect(imageCapture, &QCameraImageCapture::imageSaved, this,
          &WebCam::onCameraImageSaved);
  connect(imageCapture, &QCameraImageCapture::readyForCaptureChanged, this,
          &WebCam::onCameraReadyForCaptureChanged);
 // connect(camera, &QCamera::errorOccurred, this,
 //         &WebCam::onCameraErrorDetected);
  connect(camera, &QCamera::statusChanged, this,
          &WebCam::onCameraStatusChanged);

  connect(imageCapture,
          QOverload<int, QCameraImageCapture::Error, const QString &>::of(
              &QCameraImageCapture::error),
          [=](int id, QCameraImageCapture::Error error,
              const QString &errorString) {
            Q_UNUSED(id);
            qDebug() << "Error code: " << error << " detail: " << errorString;
          });
}

WebCam::~WebCam() {
  camera->stop();
  delete imageCapture;
  delete camera;
}

void WebCam::start() { camera->start(); }

void WebCam::stop() { camera->stop(); }

bool WebCam::capture() {
  if (imageCapture->isReadyForCapture()) {
    imageCapture->capture();
    return true;
  } else {
    return false;
  }
}

void WebCam::onCameraImageCaptured(int id, const QImage &preview) {
  qDebug() << "image captured: " << id << " size: " << preview.size();
  emit cameraImageReady(preview);
}

void WebCam::onCameraErrorDetected(QCamera::Error value) {
  qDebug() << "camera error detected:" << value;
}

void WebCam::onCameraImageSaved(int id, const QString &fileName) {
  qDebug() << "image captured: " << id << " path: " << fileName;
}

void WebCam::onCameraStatusChanged(QCamera::Status status) {
  qDebug() << "camera status changed to: " << status;
}

void WebCam::onCameraReadyForCaptureChanged(bool ready) {
  if (ready) {
    qDebug() << "camera ready for capture";
  } else {
    qDebug() << "camera not ready for capture";
  }
  emit cameraReadyChanged(ready);
}

void WebCam::onCameraImageAvail(int id, const QVideoFrame &frame) {
  qDebug() << "frame captured: " << id << " size: " << frame.size();
}
