#include <QObject>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>

class LocationUpdater: public QObject {
  Q_OBJECT
  
private:
  QGeoPositionInfoSource * geo_src;
  
public:
  LocationUpdater(QObject * parent);
  ~LocationUpdater() {}
  void request_update();


private slots:
  void positionUpdated(const QGeoPositionInfo &info);
  void error(QGeoPositionInfoSource::Error positioningError);
  void updateTimeout();

};
