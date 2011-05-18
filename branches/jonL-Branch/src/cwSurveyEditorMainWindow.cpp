//Our includes
#include "cwSurveyEditorMainWindow.h"
#include "cwSurveyChunk.h"
#include "cwStation.h"
#include "cwShot.h"
#include "cwSurvexImporter.h"
#include "cwTrip.h"
#include "cwSurveyChuckView.h"
#include "cwSurveyChunkGroupView.h"
#include "cwClinoValidator.h"
#include "cwStationValidator.h"
#include "cwCompassValidator.h"
#include "cwDistanceValidator.h"
#include "cwSurveyNoteModel.h"
#include "cwNoteItem.h"
#include "cwCave.h"
#include "cwCavingRegion.h"
#include "cwRegionTreeModel.h"
#include "cwImportSurvexDialog.h"
#include "cwTreeView.h"
#include "cwQMLWidget.h"
#include "cwSurvexExporterTripTask.h"
#include "cwSurvexExporterCaveTask.h"
#include "cwSurvexExporterRegionTask.h"
#include "cwCompassExporterCaveTask.h"
#include "cwLinePlotManager.h"
#include "cwUsedStationTaskManager.h"
#include "cwGlobalUndoStack.h"

//Qt includes
#include <QDeclarativeContext>
#include <QDeclarativeComponent>
#include <QFileDialog>
#include <QDebug>
#include <QSettings>
#include <QTreeView>
#include <QMessageBox>
#include <QThread>

#if defined(QMLJSDEBUGGER)
#include <qt_private/qdeclarativedebughelper_p.h>
#endif

#if defined(QMLJSDEBUGGER) && !defined(NO_JSDEBUGGER)
#include <jsdebuggeragent.h>
#endif
#if defined(QMLJSDEBUGGER) && !defined(NO_QMLOBSERVER)
#include <qdeclarativeviewobserver.h>
#endif

#if defined(QMLJSDEBUGGER)

// Enable debugging before any QDeclarativeEngine is created
struct QmlJsDebuggingEnabler
{
    QmlJsDebuggingEnabler()
    {
        QDeclarativeDebugHelper::enableDebugging();
    }
};

// Execute code in constructor before first QDeclarativeEngine is instantiated
static QmlJsDebuggingEnabler enableDebuggingHelper;

#endif // QMLJSDEBUGGER

cwSurveyEditorMainWindow::cwSurveyEditorMainWindow(QWidget *parent) :
    QMainWindow(parent),
    SurvexExporter(NULL),
    NoteModel(new cwSurveyNoteModel(this))
{


    setupUi(this);

#if defined(QMLJSDEBUGGER) && !defined(NO_JSDEBUGGER)
    new QmlJSDebugger::JSDebuggerAgent(DeclarativeView->engine());
#endif
#if defined(QMLJSDEBUGGER) && !defined(NO_QMLOBSERVER)
    new QmlJSDebugger::QDeclarativeViewObserver(DeclarativeView, this);
#endif

    //Setup undo redo
    UndoStack = new QUndoStack(this);

    connect(UndoStack, SIGNAL(canUndoChanged(bool)), ActionUndo, SLOT(setEnabled(bool)));
    connect(UndoStack, SIGNAL(canRedoChanged(bool)), ActionRedo, SLOT(setEnabled(bool)));
    connect(UndoStack, SIGNAL(undoTextChanged(QString)), SLOT(updateUndoText(QString)));
    connect(UndoStack, SIGNAL(redoTextChanged(QString)), SLOT(updateRedoText(QString)));
    connect(ActionUndo, SIGNAL(triggered()), UndoStack, SLOT(undo()));
    connect(ActionRedo, SIGNAL(triggered()), UndoStack, SLOT(redo()));

    DeclarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    DeclarativeView->setRenderHint(QPainter::SmoothPixmapTransform, true);
    DeclarativeView->setRenderHint(QPainter::Antialiasing, true);

    connect(actionSurvexImport, SIGNAL(triggered()), SLOT(importSurvex()));
    connect(actionSurvexExport, SIGNAL(triggered()), SLOT(openExportSurvexRegionFileDialog()));
    connect(actionCompassExport, SIGNAL(triggered()), SLOT(openExportCompassCaveFileDialog()));
    connect(actionReloadQML, SIGNAL(triggered()), SLOT(reloadQML()));

    Region = new cwCavingRegion(UndoStack, this);
    RegionTreeModel = new cwRegionTreeModel(this);
    RegionTreeModel->setCavingRegion(Region);

    reloadQML(true);

    ExportThread = new QThread(this);

    //Setup the loop closer
//    LinePlotManager = new cwLinePlotManager(this);
//    LinePlotManager->setRegion(Region);

}

void cwSurveyEditorMainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}

void OpenExportSurvexTripFileDialog();

/**
  \brief Opens the survex exporter
  */
void cwSurveyEditorMainWindow::openExportSurvexTripFileDialog() {
    QFileDialog* dialog = new QFileDialog(NULL, "Export Survex", "", "Survex *.svx");
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->open(this, SLOT(exportSurvexTrip(QString)));
}

/**
  \brief Exports the currently selected trip to filename
  */
void cwSurveyEditorMainWindow::exportSurvexTrip(QString /*filename*/) {
//    if(filename.isEmpty()) { return; }

//    cwTrip* trip = currentSelectedTrip();
//    if(trip != NULL) {
//        cwSurvexExporterTripTask* exportTask = new cwSurvexExporterTripTask();
//        exportTask->setOutputFile(filename);
//        exportTask->setData(*trip);
//        connect(exportTask, SIGNAL(finished()), SLOT(exporterFinished()));
//        connect(exportTask, SIGNAL(stopped()), SLOT(exporterFinished()));
//        exportTask->setThread(ExportThread);
//        exportTask->start();
//    }
}

/**
  \brief Called when the export task has completed
  */
void cwSurveyEditorMainWindow::exporterFinished() {
    if(sender()) {
        sender()->deleteLater();
    }
}

/**
  \brief Asks the user to choose a file to export the currently selected file
  */
void cwSurveyEditorMainWindow::openExportSurvexCaveFileDialog() {
    QFileDialog* dialog = new QFileDialog(NULL, "Export to Survex", "", "Survex *.svx");
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->open(this, SLOT(exportSurvexCave(QString)));
}

/**
  \brief Exports the currently selected cave to a file
  */
void cwSurveyEditorMainWindow::exportSurvexCave(QString /*filename*/) {
//    if(filename.isEmpty()) { return; }
//    cwCave* cave = currentSelectedCave();
//    if(cave != NULL) {
//        cwSurvexExporterCaveTask* exportTask = new cwSurvexExporterCaveTask();
//        exportTask->setOutputFile(filename);
//        exportTask->setData(*cave);
//        connect(exportTask, SIGNAL(finished()), SLOT(exporterFinished()));
//        connect(exportTask, SIGNAL(stopped()), SLOT(exporterFinished()));
//        exportTask->setThread(ExportThread);
//        exportTask->start();
//    }
}

/**
  \brief Open a file dialog for the user to save
  all the data to survex file
  */
void cwSurveyEditorMainWindow::openExportSurvexRegionFileDialog() {
    QFileDialog* dialog = new QFileDialog(NULL, "Export to Survex", "", "Survex *.svx");
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->open(this, SLOT(exportSurvexRegion(QString)));
}

/**
  \brief Exports the survex region to filename
  */
void cwSurveyEditorMainWindow::exportSurvexRegion(QString filename) {
    cwSurvexExporterRegionTask* exportTask = new cwSurvexExporterRegionTask();
    exportTask->setOutputFile(filename);
    exportTask->setData(*Region);
    connect(exportTask, SIGNAL(finished()), SLOT(exporterFinished()));
    connect(exportTask, SIGNAL(stopped()), SLOT(exporterFinished()));
    exportTask->setThread(ExportThread);
    exportTask->start();
}

/**
  Opens the compass file export dialog
  */
void cwSurveyEditorMainWindow::openExportCompassCaveFileDialog() {
    QFileDialog* dialog = new QFileDialog(NULL, "Export selected cave to Compass", "", "Compass *.dat");
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->open(this, SLOT(exportCaveToCompass(QString)));
}

/**
  Exports the currently select cave to Compass
  */
void cwSurveyEditorMainWindow::exportCaveToCompass(QString /*filename*/) {
//    if(filename.isEmpty()) { return; }
//    cwCave* cave = currentSelectedCave();
//    if(cave != NULL) {
//        cwCompassExportCaveTask* exportTask = new cwCompassExportCaveTask();
//        exportTask->setOutputFile(filename);
//        exportTask->setData(*cave);
//        connect(exportTask, SIGNAL(finished()), SLOT(exporterFinished()));
//        connect(exportTask, SIGNAL(stopped()), SLOT(exporterFinished()));
//        exportTask->setThread(ExportThread);
//        exportTask->start();
//    }
}

/**
  \brief Opens the suvrex import dialog
  */
void cwSurveyEditorMainWindow::importSurvex() {
    cwImportSurvexDialog* survexImportDialog = new cwImportSurvexDialog(Region, this);
    survexImportDialog->setUndoStack(UndoStack);
    survexImportDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    survexImportDialog->open();
}

void cwSurveyEditorMainWindow::reloadQML(bool fullReload) {

    if(fullReload)  {
        delete DeclarativeView;
        DeclarativeView = new QDeclarativeView();
        verticalLayout->addWidget(DeclarativeView);

        DeclarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
        DeclarativeView->setRenderHint(QPainter::SmoothPixmapTransform, true);
        DeclarativeView->setRenderHint(QPainter::Antialiasing, true);
    }

    QDeclarativeContext* context = DeclarativeView->rootContext();
    context->setParent(this);

    qmlRegisterType<cwCavingRegion>("Cavewhere", 1, 0, "CavingRegion");
    qmlRegisterType<cwCave>("Cavewhere", 1, 0, "Cave");
    qmlRegisterType<cwSurveyChunk>();//"Cavewhere", 1, 0, "cwSurveyChunk");
    qmlRegisterType<cwSurveyChunkView>("Cavewhere", 1, 0, "SurveyChunkView");
    qmlRegisterType<cwSurveyChunkGroupView>("Cavewhere", 1, 0, "SurveyChunkGroupView");
    qmlRegisterType<cwTrip>();
    qmlRegisterType<cwClinoValidator>("Cavewhere", 1, 0, "ClinoValidator");
    qmlRegisterType<cwStationValidator>("Cavewhere", 1, 0, "StationValidator");
    qmlRegisterType<cwCompassValidator>("Cavewhere", 1, 0, "CompassValidator");
    qmlRegisterType<cwDistanceValidator>("Cavewhere", 1, 0, "DistanceValidator");
    qmlRegisterType<cwSurveyNoteModel>("Cavewhere", 1, 0, "NoteModel");
    qmlRegisterType<cwNoteItem>("Cavewhere", 1, 0, "NoteItem");
    qmlRegisterType<cwTreeView>("Cavewhere", 1, 0, "TreeView");
    qmlRegisterType<cwRegionTreeModel>("Cavewhere", 1, 0, "RegionTreeModel");
    qmlRegisterType<cwQMLWidget>("Cavewhere", 1, 0, "ProxyWidget");
    qmlRegisterType<QWidget>("Cavewhere", 1, 0, "QWidget");
    qmlRegisterType<cwUsedStationTaskManager>("Cavewhere", 1, 0, "UsedStationTaskManager");


    context->setContextProperty("regionModel", RegionTreeModel);
    context->setContextProperty("region", Region);

    DeclarativeView->setSource(QUrl::fromLocalFile("qml/CavewhereMainWindow.qml"));

}

/**
  \brief Set's the survey data for the current editor
  */
void cwSurveyEditorMainWindow::setSurveyData(QItemSelection /*selected*/, QItemSelection /*deselected*/) {

//    QList<QModelIndex> selectedIndexes = selected.indexes();
//    if(!selectedIndexes.isEmpty()) {
//        QModelIndex firstSelected = selectedIndexes.first();
//        QVariant objectVariant = firstSelected.data(cwRegionTreeModel::ObjectRole);
//        cwTrip* trip = qobject_cast<cwTrip*>(objectVariant.value<QObject*>());
//        cwCave* cave = qobject_cast<cwCave*>(objectVariant.value<QObject*>());
//        QDeclarativeContext* context = DeclarativeView->rootContext();

//        if(trip != NULL) {
//            qDebug() << "Setting the trip";
//            context->setContextProperty("surveyData", trip);
//            //context->setContextProperty("currentPage", "SurveyEditor");
//        } else if(cave != NULL) {
//            qDebug() << "Setting the cave data";
//            context->setContextProperty("caveData", cave);
//            //context->setContextProperty("currentPage", "CavePage");
//        }
//    }
}

/**
  \brief Returns the currently selected cave

  If not cave is select, then this returns NULL
  */
//cwCave* cwSurveyEditorMainWindow::currentSelectedCave() const {
//    QList<QModelIndex> selectedIndexes = RegionTreeView->selectionModel()->selectedIndexes();

//    if(selectedIndexes.isEmpty()) {
//        qWarning() << "No elements selected";
//        return NULL;
//    }

//    QModelIndex tripIndex = selectedIndexes.first();
//    cwCave* cave = RegionTreeModel->cave(tripIndex);
//    return cave;
//}

/**
  \brief Returns the currently selected trip

  If no trip is selected, then this return NULL
  */
//cwTrip* cwSurveyEditorMainWindow::currentSelectedTrip() const {
//    QList<QModelIndex> selectedIndexes = RegionTreeView->selectionModel()->selectedIndexes();

//    if(selectedIndexes.isEmpty()) {
//        qWarning() << "No elements selected";
//        return NULL;
//    }

//    QModelIndex tripIndex = selectedIndexes.first();
//    cwTrip* trip = RegionTreeModel->trip(tripIndex);

//    return trip;
//}

void cwSurveyEditorMainWindow::updateUndoText(QString undoText) {
    ActionUndo->setText(QString("Undo - %1").arg(undoText));
}

void cwSurveyEditorMainWindow::updateRedoText(QString redoText) {
    ActionRedo->setText(QString("Redo - %1").arg(redoText));
}