/**************************************************************************
**
**    Copyright (C) 2014 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/


//Our includes
#include "cwCaptureManager.h"
#include "cw3dRegionViewer.h"
#include "cwScreenCaptureCommand.h"
#include "cwScene.h"
#include "cwGraphicsImageItem.h"
#include "cwDebug.h"
#include "cwImageResolution.h"
#include "cwCaptureItem.h"
#include "cwViewportCapture.h"
#include "cwDebug.h"

//Qt includes
#include <QLabel>
#include <QPixmap>
#include <QScopedPointer>
#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsRectItem>

cwCaptureManager::cwCaptureManager(QObject *parent) :
    QAbstractListModel(parent),
    Resolution(300.0),
    BottomMargin(0.0),
    TopMargin(0.0),
    RightMargin(0.0),
    LeftMargin(0.0),
    NumberOfImagesProcessed(0),
    Scene(new QGraphicsScene(this)),
    PaperRectangle(new QGraphicsRectItem()),
    BorderRectangle(new QGraphicsRectItem()),
    Scale(1.0)
{
    Scene->addItem(PaperRectangle);
    Scene->addItem(BorderRectangle);

    QPen pen;
    pen.setCosmetic(true);
    pen.setWidthF(1.0);

    QBrush brush;
    brush.setColor(Qt::white);
    brush.setStyle(Qt::SolidPattern);

    PaperRectangle->setPen(pen);
    PaperRectangle->setBrush(brush);

}

/**
* @brief cwCaptureManager::setView
* @param view
*/
void cwCaptureManager::setView(cw3dRegionViewer* view) {
    if(View != view) {
        View = view;
        emit viewChanged();
    }
}

/**
* @brief cwCaptureManager::setPaperSize
* @param paperSize
*
* Sets the paper size, currently inches
*/
void cwCaptureManager::setPaperSize(QSizeF paperSize) {
    if(PaperSize != paperSize) {
        PaperSize = paperSize;

        PaperRectangle->setRect(QRectF(QPointF(0, 0), PaperSize));

        emit paperSizeChanged();
    }
}

/**
* @brief cwCaptureManager::setResolution
* @param resolution
*/
void cwCaptureManager::setResolution(double resolution) {
    if(Resolution != resolution) {
        Resolution = resolution;
        emit resolutionChanged();
    }
}

/**
* @brief cwCaptureManager::setBottomMargin
* @param bottomMargin
*/
void cwCaptureManager::setBottomMargin(double bottomMargin) {
    if(BottomMargin != bottomMargin) {
        BottomMargin = bottomMargin;
        emit bottomMarginChanged();
    }
}

/**
* @brief cwCaptureManager::setTopMargin
* @param topMargin
*/
void cwCaptureManager::setTopMargin(double topMargin) {
    if(TopMargin != topMargin) {
        TopMargin = topMargin;
        emit topMarginChanged();
    }
}

/**
 * @brief cwCaptureManager::setRightMargin
 * @param rightMargin
 */
void cwCaptureManager::setRightMargin(double rightMargin) {
    if(RightMargin != rightMargin) {
        RightMargin = rightMargin;
        emit rightMarginChanged();
    }
}

/**
 * @brief cwCaptureManager::setLeftMargin
 * @param leftMargin
 */
void cwCaptureManager::setLeftMargin(double leftMargin) {
    if(LeftMargin != leftMargin) {
        LeftMargin = leftMargin;
        emit leftMarginChanged();
    }
}

/**
 * @brief cwCaptureManager::setViewport
 * @param viewport
 *
 * This is the capturing viewport in opengl. This is the area that will be captured
 * and saved by the manager. The rectangle should be in pixels.
 */
void cwCaptureManager::setViewport(QRect viewport) {
    if(Viewport != viewport) {
        Viewport = viewport;
        emit viewportChanged();
    }
}

/**
* @brief cwCaptureManager::setFilename
* @param filename
*
* The filename of the output file
*/
void cwCaptureManager::setFilename(QUrl filename) {
    if(Filename != filename) {
        Filename = filename;
        emit filenameChanged();
    }
}

/**
* @brief cwCaptureManager::setFileType
* @param fileType
*/
void cwCaptureManager::setFileType(FileType fileType) {
    if(Filetype != fileType) {
        Filetype = fileType;
        emit fileTypeChanged();
    }
}

/**
 * @brief cwCaptureManager::capture
 *
 * Executes the screen capture
 *
 * If the screen screen capture is already running, this does nothing
 */
void cwCaptureManager::capture()
{
    if(NumberOfImagesProcessed != 0) {
        qDebug() << "NumberOfImagesProcessed is not zero, but is" << NumberOfImagesProcessed << "this is a bug!" << LOCATION;
        return;
    }

    NumberOfImagesProcessed = Captures.size();

    foreach(cwViewportCapture* capture, Captures) {
        connect(capture, &cwViewportCapture::finishedCapture, this, &cwCaptureManager::saveCaptures, Qt::UniqueConnection);
        capture->setPreviewCapture(false);
        capture->capture();
    }
}

/**
 * @brief cwCaptureManager::addViewportCapture
 * @param capture
 *
 * This adds the viewport capture to the capture manager
 */
void cwCaptureManager::addViewportCapture(cwViewportCapture *capture)
{
    if(Captures.contains(capture)) {
        qWarning() << "Capture already added:" << capture << LOCATION;
        return;
    }

    if(capture == nullptr) { return; }
    addPreviewCaptureItemHelper(capture);
    addFullResultionCaptureItemHelper(capture);

    connect(capture, &cwViewportCapture::previewItemChanged, this, &cwCaptureManager::addPreviewCaptureItem);

    capture->setName(QString("Capture %1").arg(Captures.size() + 1));

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    Captures.append(capture);
    Layers.append(capture);
    endInsertRows();
    emit numberOfCapturesChanged();
}

void cwCaptureManager::removeViewportCapture(cwViewportCapture *capture)
{
    Q_UNUSED(capture)

}

/**
 * @brief cwCaptureLayerModel::rowCount
 * @param parent
 * @return The number of rows in the model
 */
int cwCaptureManager::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return Captures.size();
}

/**
 * @brief cwCaptureManager::data
 * @param index
 * @param role
 * @return
 */
QVariant cwCaptureManager::data(const QModelIndex &index, int role) const
{
    //Make sure the index is valid
    if(!index.isValid()) {
        return QVariant();
    }

    switch(role) {
    case LayerObjectRole:
        return QVariant::fromValue(Layers.at(index.row()));
    case LayerNameRole:
        return Layers.at(index.row())->name();
    default:
        return QVariant();
    }

    return QVariant();
}

/**
 * @brief cwCaptureManager::index
 * @param row
 * @param column
 * @param parent
 * @return The model index, see Qt documentation QAbstractItemModel::index() for details
 */
QModelIndex cwCaptureManager::index(int row, int column, const QModelIndex &parent) const
{
    return QAbstractListModel::index(row, column, parent);
}

/**
 * @brief cwCaptureManager::roleNames
 * @return
 */
QHash<int, QByteArray> cwCaptureManager::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LayerObjectRole] = "layerObjectRole";
    roles[LayerNameRole] = "layerNameRole";
    return roles;
}

/**
 * @brief cwCaptureManager::addPreviewCaptureItem
 *
 * This will add the preview item to the graphics scene of the preview item
 *
 * This function should only be called through a signal and slot
 */
void cwCaptureManager::addPreviewCaptureItem()
{
    Q_ASSERT(dynamic_cast<cwViewportCapture*>(sender()) != nullptr);
    cwViewportCapture* viewportCapture = static_cast<cwViewportCapture*>(sender());
    addPreviewCaptureItemHelper(viewportCapture);
}

/**
 * @brief cwCaptureManager::addPreviewCaptureItem
 *
 * This will add the full item to the graphics scene of the preview item
 *
 * This function should only be called through a signal and slot
 */
void cwCaptureManager::addFullResultionCaptureItem()
{
    Q_ASSERT(dynamic_cast<cwViewportCapture*>(sender()) != nullptr);
    cwViewportCapture* viewportCapture = static_cast<cwViewportCapture*>(sender());
    addFullResultionCaptureItemHelper(viewportCapture);
    disconnect(viewportCapture, &cwViewportCapture::fullResolutionItemChanged, this, &cwCaptureManager::addFullResultionCaptureItem);
    NumberOfImagesProcessed--;

    if(NumberOfImagesProcessed == 0) {
        saveScene();
    }
}

/**
 * @brief cwCaptureManager::saveCaptures
 */
void cwCaptureManager::saveCaptures()
{
    NumberOfImagesProcessed--;

    if(NumberOfImagesProcessed == 0) {
        //All the images have been captured, go through all the captures add the fullImages
        //to the scene.

        foreach(cwViewportCapture* capture, Captures) {
            scene()->addItem(capture->fullResolutionItem());
            capture->setResolution(resolution());
            capture->previewItem()->setVisible(false);
            capture->setPreviewCapture(false);
        }

//        PaperRectangle->setVisible(false);

        saveScene();

        foreach(cwViewportCapture* capture, Captures) {
            capture->previewItem()->setVisible(true);
            capture->fullResolutionItem()->setVisible(false);
            disconnect(capture, &cwViewportCapture::finishedCapture, this, &cwCaptureManager::saveCaptures);
        }
    }
}

/**
 * @brief cwCaptureManager::exportScene
 */
void cwCaptureManager::saveScene()
{
    QSizeF imageSize = paperSize() * resolution();
    QRectF imageRect = QRectF(QPointF(), imageSize);

    QRectF sceneRect = QRectF(QPointF(), paperSize()); //Scene->itemsBoundingRect();
    QImage outputImage(imageSize.toSize(), QImage::Format_ARGB32);
    outputImage.fill(Qt::white); //transparent);

    cwImageResolution resolutionDPI(resolution(), cwUnits::DotsPerInch);
    cwImageResolution resolutionDPM = resolutionDPI.convertTo(cwUnits::DotsPerMeter);

    outputImage.setDotsPerMeterX(resolutionDPM.value());
    outputImage.setDotsPerMeterY(resolutionDPM.value());

    QPainter painter(&outputImage);
    Scene->render(&painter, imageRect, sceneRect);

    outputImage.save(filename().toLocalFile(), "png");

    emit finishedCapture();
}

/**
 * @brief cwCaptureManager::tileProjection
 * @param viewport
 * @param originalProjection
 * @return
 *
 * This will take original viewport and original projection and find the
 * tile projection matrix using the tileViewport. The tileViewport
 * should be a sub rectangle of the original viewport. This function
 * will work with orthognal and perspective projections
 */
cwProjection cwCaptureManager::tileProjection(QRectF tileViewport,
                                                    QSizeF imageSize,
                                                    const cwProjection &originalProjection) const
{
    double originalProjectionWidth = originalProjection.right() - originalProjection.left();
    double originalProjectionHeight = originalProjection.top() - originalProjection.bottom();

    double left = originalProjection.left() + originalProjectionWidth
            * (tileViewport.left() / imageSize.width());
    double right = left + originalProjectionWidth * tileViewport.width() / imageSize.width();
    double bottom = originalProjection.bottom() + originalProjectionHeight
            * (tileViewport.top() / imageSize.height());
    double top = bottom + originalProjectionHeight * tileViewport.height() / imageSize.height();

    cwProjection projection;
    switch(originalProjection.type()) {
    case cwProjection::Perspective:
    case cwProjection::PerspectiveFrustum:
        projection.setFrustum(left, right,
                              bottom, top,
                              originalProjection.near(), originalProjection.far());
        return projection;
    case cwProjection::Ortho:
        projection.setOrtho(left, right,
                            bottom, top,
                            originalProjection.near(), originalProjection.far());
        return projection;
    default:
        qWarning() << "Can't return tile matrix because original matrix isn't a orth or perspectiveMatrix" << LOCATION;
        break;
    }
    return projection;
}

/**
 * @brief cwCaptureManager::addBorderItem
 *
 * This function will add a new border item to the scene
 */
void cwCaptureManager::addBorderItem()
{
    QPen pen;
    pen.setWidthF(4.0);
    pen.setColor(Qt::black);

    QGraphicsRectItem* rectItem = new QGraphicsRectItem();
    rectItem->setPen(QPen());
    rectItem->setBrush(Qt::NoBrush);
    rectItem->setRect(viewport());
    rectItem->setScale(Scale);
    rectItem->setZValue(1.0);

    Scene->addItem(rectItem);
}

/**
 * @brief cwCaptureManager::calcScale
 *
 * This caluclates the scale
 */
void cwCaptureManager::calcScale()
{
    double pixelPerInchWidth = screenPaperSize().width() / paperSize().width();
    double pixelPerInchHeight = screenPaperSize().height() / paperSize().height();

    qDebug() << "Pixel per inch: " << pixelPerInchHeight << pixelPerInchWidth;

    //Create the scale need to convert the viewport into the correct rendering resolution
    QPointF scale(resolution() / pixelPerInchWidth, resolution() / pixelPerInchHeight);

    if(scale.x() != scale.y()) {
        qWarning() << "Scale isn't equal in x " << scale.x() << "y" << scale.y() << "using X" << LOCATION;
    }

    Scale = scale.x();
}

/**
 * @brief cwCaptureManager::croppedTile
 * @param tileSize
 * @param imageSize
 * @param row
 * @param column
 * @return Return the tile size of row and column
 *
 * This may crop the tile, if the it goes beyond the imageSize
 */
QSize cwCaptureManager::calcCroppedTileSize(QSize tileSize, QSize imageSize, int row, int column) const
{
    QSize croppedTileSize = tileSize;

    bool exactEdgeX = imageSize.width() % tileSize.width() == 0;
    bool exactEdgeY = imageSize.height() % tileSize.height() == 0;
    int lastColumnIndex = imageSize.width() / tileSize.width();
    int lastRowIndex = imageSize.height() / tileSize.height();


    Q_ASSERT(column <= lastColumnIndex);
    Q_ASSERT(row <= lastRowIndex);

    if(column == lastColumnIndex && !exactEdgeX)
    {
        //Edge tile, crop it
        double tilesInImageX = imageSize.width() / (double)tileSize.width();
        double extraX = tilesInImageX - floor(tilesInImageX);
        double imageWidthCrop = ceil(tileSize.width() * extraX);
        croppedTileSize.setWidth(imageWidthCrop);
    }

    if(row == lastRowIndex && !exactEdgeY)
    {
        //Edge tile, crop it
        double tilesInImageY = imageSize.height() / (double)tileSize.height();
        double extraY = tilesInImageY - floor(tilesInImageY);
        double imageHeightCrop = ceil(tileSize.height() * extraY);
        croppedTileSize.setHeight(imageHeightCrop);
    }

    return croppedTileSize;
}

/**
 * @brief cwCaptureManager::scaleCaptureToFitPage
 * @param item
 *
 * This will try to fit the item to the page. By default it'll be added to the (0, 0) and
 * by stretched to fit the page.
 */
void cwCaptureManager::scaleCaptureToFitPage(cwViewportCapture *item)
{
    Q_ASSERT(item->previewItem() != nullptr);

    QSizeF paperSize = PaperSize;
    double itemAspect = item->viewport().height() /
            item->viewport().width();

    if(itemAspect > 0) {
        //Item's height is greater than it's width
            item->setPaperHeightOfItem(paperSize.height());
    } else {
        //Item's Width is greater than it's height
        item->setPaperWidthOfItem(paperSize.width());
    }

}

/**
 * @brief cwCaptureManager::addPreviewCaptureItemHelper
 * @param capture
 */
void cwCaptureManager::addPreviewCaptureItemHelper(cwViewportCapture *capture)
{
    if(capture->previewItem() != nullptr) {
        Scene->addItem(capture->previewItem());
        scaleCaptureToFitPage(capture);
    }
}

/**
 * @brief cwCaptureManager::addFullResultionCaptureItemHelper
 * @param catpure
 */
void cwCaptureManager::addFullResultionCaptureItemHelper(cwViewportCapture *capture)
{
    if(capture->fullResolutionItem() != nullptr) {
        Scene->addItem(capture->fullResolutionItem());
    }
}

/**
* @brief cwCaptureManager::setScreenPaperSize
* @param screenPaperSize - The paperSize in pixel, this is what shown on the screen
*/
void cwCaptureManager::setScreenPaperSize(QSizeF screenPaperSize) {
    if(ScreenPaperSize != screenPaperSize) {
        ScreenPaperSize = screenPaperSize;
        emit screenPaperSizeChanged();
    }
}
