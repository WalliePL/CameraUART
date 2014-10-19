#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QWidget>
#include <QThread>
#include <QSerialPort>
#include <QMetaType>
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat)
#include <opencv2/highgui/highgui.hpp>  // Video write

class CameraThread : public QWidget
{
    Q_OBJECT
public:
    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;
    };
public:
    explicit CameraThread(QWidget *parent = 0);
    ~CameraThread();
    void init(int cameraID, int fps = 25, QString fileName = "movie.avi");
    void initCamera(int cameraID);
    bool isReady(void);
    void start();

signals:
    void updateFrame(cv::Mat frame);

protected:

public slots:
    void stopThread(void);
    void saveActualFrame(void);
    void setFPS(int fps);
    void readRSData(void);
    void openRS(void);
    void readRSConfig(void);

private:
    void setRSConfiguration(Settings &configuration);
    void wait(int ms);

private:
    cv::VideoCapture *_cap;
    cv::Mat *_frame;
    cv::VideoWriter *_outputVideo;
    QString _videoName;
    int _fps;
    bool _quit;
    bool _initialized;
    bool _ready;
    bool _save;
    bool _onlyCameraRun;
    QSerialPort *_serial;
    Settings _p;
    int _frameCount;
};

#endif // CAMERATHREAD_H
