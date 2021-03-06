/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#ifndef CWLINEPLOTGEOMETRYTASK_H
#define CWLINEPLOTGEOMETRYTASK_H

//Our includes
#include "cwTask.h"
#include "cwStation.h"
class cwCavingRegion;
class cwCave;


//Qt includes
#include <QVector>
#include <QVector3D>
#include <QWeakPointer>
#include <QMap>

/**
  \brief This class isn't thread safe!
  */
class cwLinePlotGeometryTask : public cwTask
{
    Q_OBJECT

public:

    class LengthAndDepth {
    public:
        LengthAndDepth() : Depth(-1), Length(-1) { }
        LengthAndDepth(double length, double depth) : Depth(depth), Length(length) {}

        double length() const { return Length; }
        double depth() const { return Depth; }

    private:
        double Depth;
        double Length;
    };

    explicit cwLinePlotGeometryTask(QObject *parent = 0);

    //Inputs
    void setRegion(cwCavingRegion* region);

    //Outputs
    QVector<QVector3D> pointData() const;
    QVector<unsigned int> indexData() const;
    QVector<LengthAndDepth> cavesLengthAndDepths() const;

protected:
    void runTask();

signals:

public slots:

private:
    //Inputs
    cwCavingRegion* Region;

    //Outputs
    QVector<QVector3D> PointData;
    QVector<unsigned int> IndexData;
    QVector<LengthAndDepth> CavesLengthAndDepths;

    //Lookup to look up the station and get it's index
    QMap< QString, unsigned int > StationIndexLookup;

    void addStationPositions(int caveIndex);
    void addShotLines(int caveIndex);

    QString fullStationName(int caveIndex, QString caveName, QString stationName) const;
};

/**
  \brief Sets the region that the task will generate the geometry for

  \param region - should be valid
  */
inline void cwLinePlotGeometryTask::setRegion(cwCavingRegion* region) {
    Region = region;
}

/**
  \brief Gets the point geometry for all the stations
  */
inline QVector<QVector3D> cwLinePlotGeometryTask::pointData() const {
    return PointData;
}

/**
  \brief Gets all the index data for creating the lines in opengl

  This index can be drawn in opengl using draw elements
  */
inline QVector<unsigned int> cwLinePlotGeometryTask::indexData() const {
    return IndexData;
}

/**
 * @brief cwLinePlotGeometryTask::cavesLengthAndDepths
 * @return Get all the cave's lengths and depths
 */
inline QVector<cwLinePlotGeometryTask::LengthAndDepth> cwLinePlotGeometryTask::cavesLengthAndDepths() const
{
    return CavesLengthAndDepths;
}

/**
  Creates a full station name based on the caveIndex, the cave name and the station name.

  This should produces a unique name for the station for the whole region
  */
inline QString cwLinePlotGeometryTask::fullStationName(int caveIndex, QString caveName, QString stationName) const {
    return QString::number(caveIndex) + "-" + caveName + "." + stationName.toLower();
}

#endif // CWLINEPLOTGEOMETRYTASK_H
