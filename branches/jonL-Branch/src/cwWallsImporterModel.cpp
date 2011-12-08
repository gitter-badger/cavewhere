//Our includes
#include "cwWallsImporterModel.h"
#include "cwWallsGlobalData.h"
#include "cwWallsBlockData.h"
#include "cwSurveyChunk.h"
#include "cwShot.h"
#include "cwStationReference.h"
#include "cwGlobalIcons.h"

//Qt includes
#include <QPixmapCache>
#include <QDebug>

cwWallsImporterModel::cwWallsImporterModel(QObject *parent) :
    QAbstractItemModel(parent),
    GlobalData(NULL)
{
}

/**
  \brief Get's the number of columns of the model
  Always 1
  */
 int cwWallsImporterModel::columnCount ( const QModelIndex & /*parent*/) const {
     if(GlobalData == NULL) { return 0; }
     return NumberOfColumns;
 }

 /**
   \brief Get's the data at index
   */
 QVariant cwWallsImporterModel::data ( const QModelIndex & index, int role) const {
     if(GlobalData == NULL) { return QVariant(); }
     if(!index.isValid()) { return QVariant(); }

     return NameColumnData(index, role);
 }

 /**
   \brief Get's the name's column data
   */
 QVariant cwWallsImporterModel::NameColumnData(const QModelIndex & index, int role) const {

     switch(role) {
     case Qt::DisplayRole:
         return NameColumnDisplayData(index);
     case Qt::DecorationRole:
         return NameColumnIconData(index);
     default:
         return QVariant();
     }

     return QVariant();
 }

 /**
   \brief Gets the dislay data for the index

   This return the name of the index
   */
 QVariant cwWallsImporterModel::NameColumnDisplayData(const QModelIndex& index) const {
     //This index is a shot
     cwShot* shot = toShot(index); //qobject_cast<cwShot*>(static_cast<QObject*>(index.internalPointer()));
     if(shot != NULL) {
         cwStationReference fromStation = shot->fromStation();
         cwStationReference toStation = shot->toStation();

         QString fromStationName;
         QString toStationName;
         QString errorStationName = "(Error - Unknown staton name)";

         if(!fromStation.isValid()) {
             fromStationName = errorStationName;
         } else {
             fromStationName = fromStation.name();
         }

         if(!toStation.isValid()) {
             toStationName = errorStationName;
         } else {
             toStationName = toStation.name();
         }

         QString displayText = QString("%1 %2 %3").arg(fromStationName).arg(QString(QChar(0x2192))).arg(toStationName);

         return QVariant(displayText);
     }

     //This index is a block
     cwWallsBlockData* block = toBlockData(index); //qobject_cast<cwWallsBlockData*>(static_cast<QObject*>(index.internalPointer()));
     if(block != NULL) {
         if(block->name().isEmpty()) {
             return QVariant("Untitled");
         }

         return QVariant(block->name());
     }

     return QVariant();
 }

 /**
   \brief Get's the icon data for the item
   */
 QVariant cwWallsImporterModel::NameColumnIconData(const QModelIndex& index) const {
     //This index is a block
     cwWallsBlockData* block = toBlockData(index); //qobject_cast<cwWallsBlockData*>(static_cast<QObject*>(index.internalPointer()));
     if(block != NULL) {
         QPixmap icon;

         switch(block->importType()) {
         case cwWallsBlockData::NoImport:
             if(!QPixmapCache::find(cwGlobalIcons::NoImport, &icon)) {
                 icon = QPixmap(cwGlobalIcons::NoImportFilename);
                 cwGlobalIcons::NoImport = QPixmapCache::insert(icon);
             }
             break;
         case cwWallsBlockData::Trip:
             if(!QPixmapCache::find(cwGlobalIcons::Trip, &icon)) {
                 icon = QPixmap(cwGlobalIcons::TripFilename);
                 cwGlobalIcons::Trip = QPixmapCache::insert(icon);
             }
             break;
         case cwWallsBlockData::Cave:
             if(!QPixmapCache::find(cwGlobalIcons::Cave, &icon)) {
                 icon = QPixmap(cwGlobalIcons::CaveFilename);
                 cwGlobalIcons::Cave = QPixmapCache::insert(icon);
             }
             break;
         case cwWallsBlockData::Structure:
             if(!QPixmapCache::find(cwGlobalIcons::Plus, &icon)) {
                 icon = QPixmap(cwGlobalIcons::PlusFilename);
                 cwGlobalIcons::Plus = QPixmapCache::insert(icon);
             }
         }

         return QVariant(icon);
     }
     return QVariant();
 }

 /**
   \brief Gets the index at row and column of parent
   */
 QModelIndex cwWallsImporterModel::index ( int row, int /*column*/, const QModelIndex & parent) const {
     if(GlobalData == NULL) { return QModelIndex(); }

     if(!parent.isValid()) {
         //This is the root use the root data
         QList<cwWallsBlockData*> rootBlocks = GlobalData->blocks();
         if(row >= rootBlocks.size()) {
             return QModelIndex(); //Invalid index
         }

         cwWallsBlockData* block = rootBlocks[row];
         return createAndRegisterIndex(row, block, Block);
     }

     //This if the index
     cwWallsBlockData* block = toBlockData(parent); //qobject_cast<cwWallsBlockData*>(static_cast<QObject*>(parent.internalPointer()));
     if(block != NULL) {
         if(row < block->childBlockCount()) {
             //This is another block index
             cwWallsBlockData* childBlock = block->childBlock(row);
             return createAndRegisterIndex(row, childBlock, Block);
         }

         int blockIndex = row - block->childBlockCount();
         if(blockIndex < block->shotCount()) {
             //This is a shot
             cwShot* shot = block->shot(blockIndex);
             return createAndRegisterIndex(row, shot, Shot);
         }
     }

     return QModelIndex();
 }

 /**
   \brief Gets the parent index of index
   */
 QModelIndex cwWallsImporterModel::parent ( const QModelIndex & index ) const {
     if(GlobalData == NULL) { return QModelIndex(); }

     if(!index.isValid()) { return QModelIndex(); }

     cwWallsBlockData* block = toBlockData(index); //qobject_cast<cwWallsBlockData*>(static_cast<QObject*>(index.internalPointer()));
     cwWallsBlockData* parentBlock = NULL;
     if(block != NULL) {
         parentBlock = block->parentBlock();
     }

     cwShot* shot = toShot(index);
     if(shot != NULL) {
         QObject* shotParent = shot->parentChunk(); //should be a chunk
         if(shotParent != NULL) {
             parentBlock = qobject_cast<cwWallsBlockData*>(shotParent->parent()); //Should be a block
         }
     }

     if(parentBlock == NULL) {
         //Index is the root
         return QModelIndex();
     }
     //The grandparent's block
     cwWallsBlockData* grandparentBlock = parentBlock->parentBlock();
     if(grandparentBlock == NULL) {
         //In the GlobalData
         int row = GlobalData->blocks().indexOf(parentBlock);
         return createAndRegisterIndex(row, parentBlock, Block);
     } else {
         int row = grandparentBlock->childBlocks().indexOf(parentBlock);
         return createAndRegisterIndex(row, parentBlock, Block);
     }

     return QModelIndex(); //Invalid object, so invalid parent
 }

 /**
   \brief Gets the number of rows
   */
 int cwWallsImporterModel::rowCount ( const QModelIndex & parent ) const {
     if(GlobalData == NULL) { return 0; }

     if(!parent.isValid()) {
         return GlobalData->blocks().count();
     }

     cwWallsBlockData* block = toBlockData(parent);
     if(block != NULL) {
         return block->childBlockCount() + block->shotCount();
     }

     return 0;
 }

/**
  \brief Sets the data for the model
  */
void cwWallsImporterModel::setWallsData(cwWallsGlobalData* data) {
    beginResetModel();
    GlobalData = data;
    endResetModel();

    if(GlobalData == NULL) { return; }

    //Go through the GlobalData and hook it up to this object
    foreach(cwWallsBlockData* block, GlobalData->blocks()) {
        connectBlock(block);
    }
}

QModelIndex cwWallsImporterModel::createAndRegisterIndex(int row, void* object, Type type) const {
    //Cast the constness away
    const QHash<void*, Type>* constPointerTypeLookup = &PointerTypeLookup;
    QHash<void*, Type>* unConstPointerTypeLookup = const_cast<QHash<void*, Type>*>(constPointerTypeLookup);

    //Add to the lookup
    unConstPointerTypeLookup->insert(object, type);
    return createIndex(row, 0, object);
}

/**
  \brief A recusive function that connect block and it's sub blocks and shots to this model
  */
void cwWallsImporterModel::connectBlock(cwWallsBlockData* block) {
    connect(block, SIGNAL(nameChanged()), SLOT(blockDataChanged()));
    connect(block, SIGNAL(importTypeChanged()), SLOT(blockDataChanged()));

    foreach(cwWallsBlockData* childBlock, block->childBlocks()) {
        connectBlock(childBlock);
    }

//    for(int i = 0; i < block->shotCount(); i++) {
//        cwShot* shot = block->shot(i);
//        cwStation* from = shot->fromStation();
//        cwStation* to = shot->toStation();

//        connect(from, SIGNAL())
//    }
}

/**
  \brief Converts the index into a cwWallsBlockData

  If the index is invalid or it's not block data, this returns NULL

  The point that's coming out of this function should not be delete or stored
  */
cwWallsBlockData* cwWallsImporterModel::toBlockData(const QModelIndex& index) const {
    if(!index.isValid()) { return NULL; }
    if(PointerTypeLookup.value(index.internalPointer(), Invalid) != Block) { return NULL; }
    return static_cast<cwWallsBlockData*>(index.internalPointer());
}


/**
  \brief Converts the index into a cwWallsBlockData

  If the index is invalid or it's not block data, this returns NULL

  The point that's coming out of this function should not be delete or stored
  */
cwShot* cwWallsImporterModel::toShot(const QModelIndex& index) const {
    if(!index.isValid()) { return NULL; }
    if(PointerTypeLookup.value(index.internalPointer(), Invalid) != Shot) { return NULL; }
    return static_cast<cwShot*>(index.internalPointer());
}

/**
  \brief Converts a block to an index

  If the block isn't part of the model, then this returns a invalid index
  */
QModelIndex cwWallsImporterModel::toIndex(cwWallsBlockData* block) {
    if(block == NULL) { return QModelIndex(); }

    //This is a root block
    int row;
    if(block->parentBlock() == NULL) {
        row = GlobalData->blocks().indexOf(block);
    } else {
        //This is some other leaf block
        cwWallsBlockData* parentBlock = block->parentBlock();
        row = parentBlock->childBlocks().indexOf(block);
    }

    //If the row is invalid
    if(row < 0) {
        return QModelIndex();
    }

    //Block is valid, create the index
    return createAndRegisterIndex(row, block, Block);
}

/**
  \brief Converts a shot to an index

  If the shot isn't part of the model, then this returns an invalid index
  */
QModelIndex cwWallsImporterModel::toIndex(cwShot* shot) {
    if(shot == NULL) { return QModelIndex(); }

    //Make the shot is part of a chunk
    cwSurveyChunk* parentChunk = shot->parentChunk();
    if(parentChunk == NULL) { return QModelIndex(); }

    //Make sure that the parentChunk is cwWallsBlockData
    cwWallsBlockData* parentBlockData = qobject_cast<cwWallsBlockData*>(parentChunk->parent());
    QModelIndex indexParentBlockData = toIndex(parentBlockData);
    if(indexParentBlockData.isValid()) { return QModelIndex(); }

    //Find the shot in the chunk
    int shotIndex = parentBlockData->indexOfShot(shot);
    if(shotIndex < 0) { return QModelIndex(); } //Couldn't find shot in the block

    //Found the block
    int row = parentBlockData->childBlockCount() + shotIndex;
    return createAndRegisterIndex(row, shot, Shot);
}

/**
  \brief Called when a block's data has changed
  */
void cwWallsImporterModel::blockDataChanged() {
    cwWallsBlockData* block = static_cast<cwWallsBlockData*>(sender());
    QModelIndex blockIndex = toIndex(block);
    if(!blockIndex.isValid()) { return; }
    emit dataChanged(blockIndex, blockIndex);
}

/**
  \brief Called when a shot's data has changed
  */
void cwWallsImporterModel::shotDataChanged() {
//    cwShot* shot = static_cast<cwShot*>(sender());
//    QModelIndex shotIndex = toIndex(shot);
//    if(!shotIndex.isValid()) { return; }
//    emit dataChanged(shotIndex, shotIndex);
}