#include "main.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QAndroidService>
#include <QTimer>
#include <QDebug>
#include <QAndroidJniObject>
#include <QtAndroid>
#include <QThread>



FILE * debug_log_fh = nullptr;
QtMessageHandler default_msg_handler;
void msg_logger(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void background_service();

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  bool is_service = argc > 1 && strcmp(argv[1], "-service") == 0;
  
  debug_log_fh = fopen(is_service ? "service.log" : "activity.log", "w");
  default_msg_handler = qInstallMessageHandler(msg_logger);
  
  if (!is_service) {
      
    QGuiApplication app(argc, argv);
  
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {                           
      return -1;
    }
    
    QAndroidJniObject::callStaticMethod<void>(
      "com/example/MyService",
      "startMyService",
      "(Landroid/content/Context;)V",
      QtAndroid::androidActivity().object());
    new LocationUpdater(&app);
    return app.exec();
  } else { // is_service
    
    QAndroidService app(argc, argv);
    qDebug()<<"creating LocationUpdater()";
    new LocationUpdater(&app);
    QTimer::singleShot(1000, &background_service);
    return app.exec();
    
  }
}

void background_service() {
  while (1) {
    qDebug()<<"Service tick";
    QThread::sleep(10);
  }
}

LocationUpdater::LocationUpdater(QObject * parent) : QObject(parent) {
  qDebug() << "Beginning initialization of LocationUpdater";
  geo_src = QGeoPositionInfoSource::createDefaultSource(this);
  connect(geo_src, &QGeoPositionInfoSource::positionUpdated, this, &LocationUpdater::positionUpdated);
  //connect(geo_src, &QGeoPositionInfoSource::error, this, &LocationUpdater::error);
  connect(geo_src, &QGeoPositionInfoSource::updateTimeout, this, &LocationUpdater::updateTimeout);
  
  // qDebug()<<"calling geo_src->requestUpdate();";
  // geo_src->requestUpdate();
  
  geo_src->setUpdateInterval(1000 * 30);
  qDebug() << "Calling startUpdates()";
  geo_src->startUpdates();
  
  qDebug() << "Initialized LocationUpdater";
  
}

void LocationUpdater::positionUpdated(QGeoPositionInfo const & info) {
  qDebug() << "Position updated:"<<info;
}

void LocationUpdater::error(QGeoPositionInfoSource::Error positioningError) {
  qDebug() << "positioningError="<<positioningError;
}

void LocationUpdater::updateTimeout() {
  qDebug() << "updateTimeout";
}

void msg_logger(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
      case QtDebugMsg:
        //fprintf(stdout, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(debug_log_fh, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
#ifdef QT_DEBUG
        fflush(debug_log_fh);
#endif
        break;
      case QtInfoMsg:
        //fprintf(stdout, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(debug_log_fh, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
      case QtWarningMsg:
        //fprintf(stdout, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(debug_log_fh, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
      case QtCriticalMsg:
        //fprintf(stdout, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(debug_log_fh, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
      case QtFatalMsg:
        //fprintf(stdout, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fprintf(debug_log_fh, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        fflush(debug_log_fh);
        abort();
    }
    (*default_msg_handler)(type, context, msg);
}
