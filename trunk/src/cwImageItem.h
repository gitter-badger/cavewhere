#ifndef CWIMAGEITEM_H
#define CWIMAGEITEM_H

//Qt includes
#include <QString>
#include <QFutureWatcher>

//Our includes
#include "cwGLRenderer.h"
#include "cwImage.h"
#include "cwImageData.h"
class cwImageProperties;

class cwImageItem : public cwGLRenderer
{
    Q_OBJECT

    Q_PROPERTY(cwImage image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(QString projectFilename READ projectFilename WRITE setProjectFilename NOTIFY projectFilenameChanged())
    Q_PROPERTY(float rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(QMatrix4x4 modelMatrix READ modelMatrix NOTIFY modelMatrixChanged)
    Q_PROPERTY(cwImageProperties* imageProperties READ imageProperties NOTIFY imagePropertiesChanged)

public:
    cwImageItem(QDeclarativeItem *parent = 0);

    void setImage(const cwImage& image);
    cwImage image() const;

    cwImageProperties* imageProperties() const;

    void setRotation(float degrees);
    float rotation() const;

    QString projectFilename() const;
    void setProjectFilename(QString projectFilename);

    QMatrix4x4 modelMatrix() const;

    Q_INVOKABLE void updateRotationCenter();

    Q_INVOKABLE QPointF mapQtViewportToNote(QPoint qtViewportCoordinate);


signals:
    void rotationChanged();
    void imageChanged();
    void projectFilenameChanged();
    void modelMatrixChanged();
    void imagePropertiesChanged();

protected:
    virtual void initializeGL();
    virtual void resizeGL();
    virtual void paintFramebuffer();

private:

    //The project filename for this class
    cwImage Image;
    cwImageProperties* ImageProperties; //!< The image properties of the current Image
    cwImageData OriginalMetadata;  //This will never have image data, only the metadata
    float Rotation;
    QString ProjectFilename;
    QMatrix4x4 RotationModelMatrix;
    QPointF RotationCenter;

    //For loading the image from disk
    QFutureWatcher<QPair<QByteArray, QSize> >* LoadNoteWatcher;

    //For rendering
    int vVertex; //!< The attribute location of the vVertex
    int ModelViewProjectionMatrix; //!< The attribute location for modelViewProjection matrix
    GLuint NoteTexture; //!< The texture id for rendering the notes to the screen
    QSize ImageSize; //!< The size of the biggest mipmap
    QGLShaderProgram* ImageProgram; //!< The image shader program that's used to render the image
    QGLBuffer NoteVertexBuffer; //!< The vertex buffer

    void initializeShaders();
    void initializeVertexBuffers();
    void initializeTexture();

    /**
      This class allow use to load the mipmaps in a thread way
      from the database

      The project filename must be set so we can load the data
*/
    class LoadImage {
    public:
        LoadImage(QString projectFilename) :
            Filename(projectFilename) {    }

        typedef QPair<QByteArray, QSize> result_type;

        QPair<QByteArray, QSize> operator()(int imageId);

        QString Filename;
    };


private slots:
     void ImageFinishedLoading();

};

/**
  Gets the image
  */
inline cwImage cwImageItem::image() const {
    return Image;
}

/**
  Gets the rotation of the image in degress.
  */
inline float cwImageItem::rotation() const {
    return Rotation;
}

/**
  \brief Gets the project Filename
  */
inline QString cwImageItem::projectFilename() const {
    return ProjectFilename;
}

/**
  \brief Gets the model matrix
  */
inline QMatrix4x4 cwImageItem::modelMatrix() const {
    return RotationModelMatrix;
}

/**
Gets imageProperties
*/
inline cwImageProperties* cwImageItem::imageProperties() const {
    return ImageProperties;
}
#endif // CWIMAGEITEM_H