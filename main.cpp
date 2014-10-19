#include <QApplication>
#include <QtCore>
#include <QCamera>
#include "camerathread.h"
#include "globals.h"

int main(int argc, char *argv[])
{
    QApplication a(argc,argv);

    QApplication::setApplicationName("Recorder");
    QApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));
    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

    // A boolean option with a single name (-l, -list)
    QCommandLineOption listOption(QStringList() << "l" << "list", QCoreApplication::translate("main", "List avaiable cameras"));
    parser.addOption(listOption);

    // A boolean option with a single name (-l, -list)
    QCommandLineOption testOption(QStringList() << "t" << "test", QCoreApplication::translate("main", "Test run - shows camera input, not save to file"));
    parser.addOption(testOption);

    // An option with a value
    QCommandLineOption cameraIdOption(QStringList() << "c" << "camera",
                                             QCoreApplication::translate("main", "Set camera with <id>."),
                                             QCoreApplication::translate("main", "id"));
    parser.addOption(cameraIdOption);

    // An option with a value
    QCommandLineOption fileNameOption(QStringList() << "f" << "file",
                                      QCoreApplication::translate("main", "Set file name as <name>."),
                                      QCoreApplication::translate("main", "name"),
                                      QLatin1String("movie.avi"));
    parser.addOption(fileNameOption);

    // An option with a value
    QCommandLineOption fpsOption(QStringList() << "fps" ,
                                      QCoreApplication::translate("main", "Set fps for file as <fps>."),
                                      QCoreApplication::translate("main", "fps"),
                                      QLatin1String("25"));
    parser.addOption(fpsOption);
    
    // Process the actual command line arguments given by the user
    parser.process(a);

    const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)

    bool list = parser.isSet(listOption);
    bool test = parser.isSet(testOption);
    QString cameraID = parser.value(cameraIdOption);
    QString fileName = parser.value(fileNameOption);
    QString fpsValue = parser.value(fpsOption);

    quint8 status = 0;

    if (test)
    {
        status |= 1<<0;
    }

    if (list)
    {
        status |= 1<<1;
    }

    switch (status)
    {
        case PS_NORMAL:
        {
            if (!cameraID.isEmpty())
            {
                bool ok = false;
                int id = cameraID.toInt(&ok);
                if (!ok)
                {
                    qWarning() << __FILE__ << __LINE__ << "Bad camera id";
                }

                int fps = fpsValue.toInt(&ok);
                if (!ok)
                {
                    qWarning() << __FILE__ << __LINE__ << "Bad fps value";
                }

                qDebug() << "Camera id: " << id;
                qDebug() << "File name: " << fileName;
                qDebug() << "FPS : " << fps;

                CameraThread camera;
                camera.readRSConfig();
                camera.init(id, fps, fileName);

                camera.start();
                return a.exec();

            }
            else
            {
                qWarning() << __FILE__ << __LINE__ << "Missing parameter";
            }
        }
        break;

        case PS_LIST:
        {
            qDebug() << " Avaiable cameras : ";
            int i = 0;

            //Camera devices:
            foreach(const QByteArray &deviceName, QCamera::availableDevices())
            {
                QString description = QCamera::deviceDescription(deviceName);
                qDebug() << i << " - " << description;
            }
        }
        break;

        case PS_TEST:
        {
            if (!cameraID.isEmpty())
            {
                qDebug() << "-- TEST RUN -- ";
                bool ok = false;
                int id = cameraID.toInt(&ok);
                if (!ok)
                {
                    qWarning() << __FILE__ << __LINE__ << "Bad camera id";
                }
                qDebug() << "Camera id: " << id;
                CameraThread camera;
                camera.initCamera(id);

                camera.start();
            }
            else
            {
                qWarning() << __FILE__ << __LINE__ << "Missing parameter";
            }

        }
        break;

        default :
        {

        }
        break;
    }

//    return a.exec();
}
