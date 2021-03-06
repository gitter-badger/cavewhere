/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#include "cwPositioner3D.h"

cwPositioner3D::cwPositioner3D(QQuickItem *parent) :
    QQuickItem(parent)
{
}


/**
Sets position3D
*/
void cwPositioner3D::setPosition3D(QVector3D position3D) {
    if(Position3D != position3D) {
        Position3D = position3D;
        emit position3DChanged();
    }
}
