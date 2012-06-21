//Our includes
#include "cwScrapControlPointView.h"
#include "cwScrap.h"

cwScrapControlPointView::cwScrapControlPointView(QQuickItem *parent) :
    cwAbstractPointManager(parent),
    Scrap(NULL)
{
}


void cwScrapControlPointView::setScrap(cwScrap *scrap)
{
    if(Scrap != scrap) {
        if(Scrap != NULL) {
            disconnect(Scrap, &cwScrap::insertedPoints, this, &cwScrapControlPointView::pointsInserted);
            disconnect(Scrap, &cwScrap::removedPoints, this, &cwScrapControlPointView::pointsRemoved);
            disconnect(Scrap, &cwScrap::pointsReset, this, &cwScrapControlPointView::reset);
        }

        Scrap = scrap;

        if(Scrap != NULL) {
            connect(Scrap, &cwScrap::insertedPoints, this, &cwScrapControlPointView::pointsInserted);
            connect(Scrap, &cwScrap::removedPoints, this, &cwScrapControlPointView::pointsRemoved);
            connect(Scrap, &cwScrap::pointsReset, this, &cwScrapControlPointView::reset);
        }

        reset();

        scrapChanged();
    }
}

/**
 * @brief cwScrapControlPointView::updateItemData
 * @param item - The item that's going to be update
 * @param pointIndex - The index of the item in the item list
 */
void cwScrapControlPointView::updateItemData(QQuickItem *item, int pointIndex) {
    item->setProperty("scrap", QVariant::fromValue(Scrap));
    item->setProperty("pointIndex", pointIndex);
}

void cwScrapControlPointView::updateItemPosition(QQuickItem *item, int pointIndex)
{
    QPointF point = Scrap->points().at(pointIndex);
    item->setProperty("position3D", QVector3D(point));
}


/**
 * @brief cwScrapControlPointView::reset
 *
 * Resets the view, this reloads all the points
 */
void cwScrapControlPointView::reset()
{
    if(Scrap) {
        //Update the number of points
        resizeNumberOfItems(Scrap->numberOfPoints());
    } else {
        //Remove all the points
        resizeNumberOfItems(0);
    }
}
