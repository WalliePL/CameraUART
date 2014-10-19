#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QTime>
#include <QDir>
#include <QApplication>
#include <qxmlstream.h>
#include "camerathread.h"

//!
//! \brief Object constructor
//! \param parent Represents parent of object.
//!
CameraThread::CameraThread(QWidget *parent) :
    QWidget(parent),
    _quit(false),
    _initialized(false),
    _ready(false),
    _save(false),
    _onlyCameraRun(false),
    _serial(nullptr),
    _frameCount(0)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");

    this->_p.name = "COM1";
    this->_p.baudRate = QSerialPort::Baud115200;
    this->_p.dataBits = QSerialPort::Data8;
    this->_p.parity = QSerialPort::NoParity;
    this->_p.stopBits = QSerialPort::OneStop;
    this->_p.flowControl = QSerialPort::NoFlowControl;
}

//!
//! \brief Object desctuctor.
//!
CameraThread::~CameraThread()
{
    if (this->_serial != nullptr)
    {
        this->_serial->close();
        delete this->_serial;

    }
    this->_cap->release();
    delete this->_cap;
    delete this->_frame;

    if (this->_outputVideo != nullptr)
    {
        this->_outputVideo->release();
        delete this->_outputVideo;
    }
}

//!
//! \brief Method inits all needed pointers and data.
//! \param cameraID Represents camera id.
//! \param fps Represetns new fps value.
//! \param fileName Represents file name where data will be saved.
//!
void CameraThread::init(int cameraID, int fps, QString fileName)
{
    if (!this->_initialized)
    {
        this->_cap = new cv::VideoCapture(cameraID);

        if(!this->_cap->isOpened()) // check if we succeeded
        {
            qWarning() << __FILE__ << __LINE__ << "Cannot open camera file";
            return;
        }

        this->wait(1);

        cv::Size S = cv::Size((int) this->_cap->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                         (int) this->_cap->get(CV_CAP_PROP_FRAME_HEIGHT));

        this->_frame = new cv::Mat();
        this->_fps = fps;
        this->_videoName = fileName;
        this->_outputVideo = new cv::VideoWriter(this->_videoName.toStdString(), -1 , double(fps), S, true);

        qDebug() << __FILE__ << S.height << S.width << fps;

        if (!this->_outputVideo->isOpened())
        {
            qWarning() << __FILE__ << __LINE__ << "Could not open the output video for write: " << this->_videoName;
            return;
        }

        this->_serial = new QSerialPort(this);
        connect(this->_serial, SIGNAL(readyRead()), this, SLOT(readRSData()));
        this->openRS();

        this->_initialized = true;
    }
    else
    {
        qWarning() << __FILE__ << __LINE__ << "Camera already initialized";
    }
}

//!
//! \brief Method inits only camera.
//! \param cameraID represents camera id.
//!
void CameraThread::initCamera(int cameraID)
{
    if (!this->_initialized)
    {
        this->_cap = new cv::VideoCapture(cameraID);

        if(!this->_cap->isOpened()) // check if we succeeded
        {
            qWarning() << __FILE__ << __LINE__ << "Cannot open camera file";
            return;
        }
        this->_frame = new cv::Mat();

        this->_outputVideo = nullptr;
        this->_serial = nullptr;
        this->_fps = 25;
        this->_onlyCameraRun = true;

        this->_initialized = true;
        this->wait(1);
    }
    else
    {
        qWarning() << __FILE__ << __LINE__ << "Camera already initialized";
    }
}

//!
//! \brief Getter for ready value. Is true after initialization.
//! \return Returns true if thread starts to run otherwise false.
//!
bool CameraThread::isReady(void)
{
    return this->_ready;
}

//!
//! \brief Overloaded method.
//!
void CameraThread::start(void)
{
    if (!this->_initialized)
    {
        qWarning() << __FILE__ << __LINE__ << "Camera not intialized.";
        return;
    }

    this->_ready = true;

    if (this->_onlyCameraRun)
    {
        int key = 0;
        if (this->_cap == nullptr)
        {
            qWarning() << __FILE__ << __LINE__ << "Camera not initialize!";
            return;
        }

        if (this->_frame == nullptr)
        {
            qWarning() << __FILE__ << __LINE__ << "Frame container not initalized!";
            return;
        }

        while (!this->_quit)
        {
            this->_cap->read(*(this->_frame)); // get a new frame from camera

            if (!this->_frame->empty())
            {
                imshow("frame", *(this->_frame));
            }

            key = cv::waitKey(30);
            if (key >= 0)
            {
                qDebug() << __FILE__ << "stop";
                this->_quit = true;
            }
        }
    }
}

//!
//! \brief Method stops thread.
//!
void CameraThread::stopThread(void)
{
    qApp->quit();
//    this->_quit = true;
}

//!
//! \brief Setter for save value. Sets it to true value to save actual frame.
//!
void CameraThread::saveActualFrame(void)
{
    this->_cap->read(*(this->_frame)); // get a new frame from camera
    this->_outputVideo->write(*(this->_frame));
    imshow("frame", *(this->_frame));
    qDebug() << __FILE__ << __LINE__ << "save frame:" << ++this->_frameCount << QTime::currentTime().toString("hh:mm:ss:zzz");
//    this->_save = true;
}

//!
//! \brief Setter method for fpr value
//! \param fps Represents new fps value
//!
void CameraThread::setFPS(int fps)
{
    if (this->_fps != fps)
    {
        this->_fps = fps;
    }
}

//!
//! \brief Method called when new data from com arrives.
//!
void CameraThread::readRSData(void)
{
    QByteArray data = this->_serial->readAll();
    QString text = QString(data);
    if (text.contains('a'))
    {
        this->saveActualFrame();
    }
    else if (text.contains('q'))
    {
        this->stopThread();
    }
}

//!
//! \brief Method opens com connection.
//!
void CameraThread::openRS(void)
{
    this->_serial->setPortName(this->_p.name);
    this->_serial->setBaudRate(this->_p.baudRate);
    this->_serial->setDataBits(this->_p.dataBits);
    this->_serial->setParity(this->_p.parity);
    this->_serial->setStopBits(this->_p.stopBits);
    this->_serial->setFlowControl(this->_p.flowControl);
    if (this->_serial->open(QIODevice::ReadWrite))
    {
            qDebug() << __FILE__ << __LINE__ << QString("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(this->_p.name).arg(this->_p.stringBaudRate).arg(this->_p.stringDataBits)
                                       .arg(this->_p.stringParity).arg(this->_p.stringStopBits).arg(this->_p.stringFlowControl);
    }
    else
    {
        qWarning() << __FILE__ << __LINE__ << "Error" <<  this->_serial->errorString();
    }
}

//!
//! \brief Setter for rs configuration data.
//! \param configuration Represents reference to new settings.
//!
void CameraThread::setRSConfiguration(Settings &configuration)
{
    this->_p.name = configuration.name;
    this->_p.baudRate = configuration.baudRate;
    this->_p.dataBits = configuration.dataBits;
    this->_p.parity = configuration.parity;
    this->_p.stopBits = configuration.stopBits;
    this->_p.flowControl = configuration.flowControl;

    qDebug() << "-- New com port configuration -- ";
    qDebug() << this->_p.name;
    qDebug() << this->_p.baudRate;
    qDebug() << this->_p.dataBits;
    qDebug() << this->_p.parity;
    qDebug() << this->_p.stopBits;
    qDebug() << this->_p.flowControl;
}

//!
//! \brief Method waits given time
//! \param ms Represents ms to wait.
//!
void CameraThread::wait(int ms)
{
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(ms); // 1 second
    loop.exec();
}

//!
//! \brief Method reads com setting from xml and sets them to object.
//!
void CameraThread::readRSConfig(void)
{
    Settings p;
    // set def
    p.name = "COM1";
    p.baudRate = QSerialPort::Baud1200;
    p.dataBits = QSerialPort::Data8;
    p.parity = QSerialPort::NoParity;
    p.stopBits = QSerialPort::OneStop;
    p.flowControl = QSerialPort::NoFlowControl;

    QFile* file = new QFile(QDir::toNativeSeparators(QApplication::applicationDirPath() + QDir::separator() +  "comConfig.xml"));
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << __FILE__ << __LINE__ << "QXSRExample::parseXML" <<
                      "Couldn't open comConfig.xml";
        return;
    }

    QXmlStreamReader xml(file);

    /* We'll parse the XML until we reach end of it.*/
    while(!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        // If token is just StartDocument, we'll go to next.
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }

        // If token is StartElement, we'll see if we can read it.
        if(token == QXmlStreamReader::StartElement)
        {
            //If it's named persons, we'll go to the next/
            if(xml.name() == "uart")
            {
                continue;
            }
            else if(xml.name() == "portName")
            {
                xml.readNext();
                QString elementName = xml.text().toString();
                p.name = elementName;
            }
            else if(xml.name() == "baudRate")
            {
                xml.readNext();
                int elementName = xml.text().toInt();
                p.baudRate = elementName;
            }
            else if(xml.name() == "dataBits")
            {
                xml.readNext();
                int elementName = xml.text().toInt();
                switch (elementName)
                {
                    case 5:
                    {
                        p.dataBits = QSerialPort::Data5;
                    }
                    break;

                    case 6:
                    {
                        p.dataBits = QSerialPort::Data6;
                    }
                    break;

                    case 7:
                    {
                        p.dataBits = QSerialPort::Data7;
                    }
                    break;

                    case 8:
                    {
                        p.dataBits = QSerialPort::Data8;
                    }
                    break;

                    default:
                    {
                        p.dataBits = QSerialPort::UnknownDataBits;
                    }
                    break;
                }
            }
            else if(xml.name() == "parity")
            {
                xml.readNext();
                QString elementName = xml.text().toString();
                if (elementName.contains("NoParity"))
                {
                    p.parity = QSerialPort::NoParity;
                }
                else if (elementName.contains("EvenParity"))
                {
                    p.parity = QSerialPort::EvenParity;
                }
                else if (elementName.contains("OddParity"))
                {
                    p.parity = QSerialPort::OddParity;
                }
                else if (elementName.contains("SpaceParity"))
                {
                    p.parity = QSerialPort::SpaceParity;
                }
                else if (elementName.contains("MarkParity"))
                {
                    p.parity = QSerialPort::MarkParity;
                }
                else
                {
                    p.parity = QSerialPort::UnknownParity;
                }
            }
            else if(xml.name() == "stopBits")
            {
                xml.readNext();
                int elementName = xml.text().toInt();
                switch (elementName)
                {
                    case 1:
                    {
                        p.stopBits = QSerialPort::OneStop;
                    }
                    break;

                    case 2:
                    {
                        p.stopBits = QSerialPort::TwoStop;

                    }
                    break;

                    case 3:
                    {
                        p.stopBits = QSerialPort::OneAndHalfStop;
                    }
                    break;

                    default:
                    {
                        p.stopBits = QSerialPort::UnknownStopBits;
                    }
                    break;
                }
            }
            else if(xml.name() == "flowControl")
            {
                xml.readNext();
                QString elementName = xml.text().toString();
                if (elementName.contains("NoFlowControl"))
                {
                    p.flowControl = QSerialPort::NoFlowControl;
                }
                else if (elementName.contains("HardwareControl"))
                {
                    p.flowControl = QSerialPort::HardwareControl;
                }
                else if (elementName.contains("SoftwareControl"))
                {
                    p.flowControl = QSerialPort::SoftwareControl;
                }
                else
                {
                    p.flowControl = QSerialPort::UnknownFlowControl;
                }
            }

            xml.readNext();
        }
    }
    // Error handling

    if(xml.hasError())
    {
        qWarning() << __FILE__ << __LINE__ << "QXSRExample::parseXML : " <<  xml.errorString();
    }

    this->setRSConfiguration(p);
    xml.clear();
}
