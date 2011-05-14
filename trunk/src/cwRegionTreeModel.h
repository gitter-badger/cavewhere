#ifndef CWREGIONTREEMODEL_H
#define CWREGIONTREEMODEL_H

//Our includes
class cwCavingRegion;
class cwTrip;
class cwCave;

//Qt includes
#include <QAbstractItemModel>
#include <QtGlobal>
#include <QDebug>

class cwRegionTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_ENUMS(ItemType)
    Q_ENUMS(RoleItem)

public:
    enum RoleItem {
        TypeRole = Qt::UserRole + 1,
        NameRole, //For everything
        DateRole,  //Only valid for trips
        ObjectRole, //For exctracting the object
        IconSourceRole //Store a url to the image
    };

    enum ItemType {
        RegionType,
        CaveType,
        TripType
    };

    explicit cwRegionTreeModel(QObject *parent = 0);

    void setCavingRegion(cwCavingRegion* region);

    Q_INVOKABLE QModelIndex index ( int row, int column, const QModelIndex & parent ) const;
   // Q_INVOKABLE QModelIndex index ( int row, const QModelIndex& parent) const;
    QModelIndex index (cwCave* cave) const;
    QModelIndex index (cwTrip* trip) const;
    QModelIndex parent ( const QModelIndex & index ) const;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    Q_INVOKABLE QVariant data ( const QModelIndex & index, int role) const;
    Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role);
    Q_INVOKABLE void removeIndex(QModelIndex item);
    Qt::ItemFlags flags ( const QModelIndex & index);

    Q_INVOKABLE cwTrip* trip(const QModelIndex& index) const;
    Q_INVOKABLE cwCave* cave(const QModelIndex& index) const;

signals:

public slots:

private slots:
    void beginInsertCaves(int beginIndex, int endIndex);
    void endInsertCaves(int beginIndex, int endIndex);

    void beginRemoveCaves(int beginIndex, int endIndex);
    void endRemoveCaves(int beginIndex, int endIndex);

    void beginInsertTrip(int beginIndex, int endIndex);
    void endInsertTrip(int beginIndex, int endIndex);

    void beginRemoveTrip(int beginIndex, int endIndex);
    void endRemoveTrip(int beginIndex, int endIndex);

    void caveDataChanged();
    void tripDataChanged();

private:

    cwCavingRegion* Region;

    void addCaveConnections(int beginIndex, int endIndex);
    void removeCaveConnections(int beginIndex, int endIndex);

    void addTripConnections(cwCave* parentCave, int beginIndex, int endIndex);
    void removeTripConnections(cwCave* parentCave, int beginIndex, int endIndex);

};


/**
  \brief Called when a cave is beginning to be added
  */
inline void cwRegionTreeModel::beginInsertCaves(int beginIndex, int endIndex) {
    beginInsertRows(QModelIndex(), beginIndex, endIndex);
}

/**
  \brief Cave removed
  */
inline void cwRegionTreeModel::endRemoveCaves(int /*beginIndex*/, int /*endIndex*/) {
    endRemoveRows();
}

#endif // CWREGIONTREEMODEL_H
