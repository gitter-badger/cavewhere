/**************************************************************************
**
**    Copyright (C) 2015 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/


#include "cwPageViewAttachedType.h"
#include "cwPage.h"

cwPageViewAttachedType::cwPageViewAttachedType(QObject *parent) : QObject(parent)
{

}

cwPageViewAttachedType::~cwPageViewAttachedType()
{

}


/**
* @brief cwPageViewAttachedType::page
* @return
*/
cwPage* cwPageViewAttachedType::page() const {
    return Page;
}

/**
* @brief cwPageViewAttachedType::setPage
* @param page
*/
void cwPageViewAttachedType::setPage(cwPage* page) {
    if(Page != page) {
        Page = page;
        emit pageChanged();
    }
}

