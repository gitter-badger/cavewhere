//Our includes
#include "cwStationReferance.h"
#include "cwCave.h"

cwStationReference::cwStationReference(QObject *parent) :
    QObject(parent),
    Cave(NULL),
    SharedStation(new cwStation())
{
    ConnectStation();
}

/**
  \brief Sets the cave for this referance

  If the station already exist in the cave, then the data will
  be over written with new data
  */
void cwStationReference::setCave(cwCave* cave) {
    if(Cave == cave) { return; }

    //Sets the cave for this referance
    if(Cave != NULL) {
        disconnect(Cave, 0, this, 0);
    }

    Cave = cave;

    connect(Cave, SIGNAL(destroyed()), SLOT(caveDestroyed()));

    //The cave already has a station named this
    if(Cave->hasStation(name())) {
        //Overwrite that station
        QSharedPointer<cwStation> caveStation = cave->station(name()).toStrongRef();

        //Copy the data from this object
        caveStation->setDown(SharedStation->down());
        caveStation->setLeft(SharedStation->left());
        caveStation->setPosition(SharedStation->position());
        caveStation->setRight(SharedStation->right());
        caveStation->setUp(SharedStation->up());

        DisconnectStation();

        //Set that station to this object
        SharedStation = caveStation;

        ConnectStation();
    } else {
        //Add this station to the cave
        Cave->addStation(SharedStation);
    }
}

/**
  \brief Updates the name

  If the name has changed, this will lookup the station.  If the station exist, then
  this station will be update with the existing station's data.  If the station doesn't exist
  then this station will be renamed.
  */
void cwStationReference::setName(QString newName) {
    if(newName == name()) { return; }

    if(Cave == NULL) {
        //We have cave
        if(Cave->hasStation(newName)) {
            DisconnectStation();

            SharedStation = Cave->station(newName);

            ConnectStation();
            emit reset();
        } else {
            //Remove it from the cave

            //Make the temperary point to the data
            QString oldName = SharedStation->name();

            //Copy the old data, this is simlar to detaching the data
            cwStation* oldStationData = SharedStation.data();
            cwStation* newStation = new cwStation(*oldStationData);

            DisconnectStation();

            //Reassign the station
            SharedStation = QSharedPointer<cwStation>(newStation);

            ConnectStation();

            //Try to remove the old station
            Cave->removeStation(oldName);

            //Try to add the new station
            Cave->addStation(SharedStation);

            SharedStation->setName(newName);
        }
    } else {
        //No cave attached, so just set the sharedStation
        SharedStation->setName(newName);
    }
}

/**
  \brief Connects all the station's signals to this object
  */
void cwStationReference::ConnectStation() {
    cwStation* station = SharedStation.data();
    connect(station, SIGNAL(nameChanged()), SIGNAL(nameChanged()));
    connect(station, SIGNAL(downChanged()), SIGNAL(downChanged()));
    connect(station, SIGNAL(leftChanged()), SIGNAL(leftChanged()));
    connect(station, SIGNAL(positionChanged()), SIGNAL(positionChanged()));
    connect(station, SIGNAL(rightChanged()), SIGNAL(upChanged()));
}


