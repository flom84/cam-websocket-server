#ifndef WEBCAM_H
#define WEBCAM_H

#include <QObject>
#include <QCamera>
#include <QVideoFrame>
#include <QImage>
#include <QString>
QT_FORWARD_DECLARE_CLASS(QCameraImageCapture)

class WebCam : public QObject {
  Q_OBJECT
public:
  explicit WebCam(QObject *parent = nullptr);
  ~WebCam();
   void start();
   void stop();
   bool capture();

signals:
    void cameraImageReady(const QImage& image);
    void cameraReadyChanged(bool ready);

private:
  QCamera *camera;
  QCameraImageCapture *imageCapture;
private slots:
  void onCameraImageAvail(int id, const QVideoFrame &frame);
  void onCameraImageCaptured(int id, const QImage &preview);
  void onCameraErrorDetected(QCamera::Error value);
  void onCameraImageSaved(int id, const QString &fileName);
  void onCameraStatusChanged(QCamera::Status status);
  void onCameraReadyForCaptureChanged(bool ready);

};

#endif // WEBCAM_H
