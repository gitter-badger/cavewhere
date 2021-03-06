/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#ifndef CWTASK_H
#define CWTASK_H

#include <QMutex>
#include <QObject>
#include <QReadWriteLock>
#include <QTimer>

/**
  \brief A member functions in the class are thread safe
  */

class cwTask : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int numberOfSteps READ numberOfSteps NOTIFY numberOfStepsChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)

public:
    enum Status {
        Ready,
        PreparingToStart,
        Running,
        Stopped,
        Restart
    };

    explicit cwTask(QObject *parent = 0);

    void setParentTask(cwTask* parentTask);
    void setThread(QThread* threadToRunOn, Qt::ConnectionType connectionType = Qt::AutoConnection);

    int numberOfSteps() const;
    int progress() const;

    Status status() const;
    bool isRunning() const;
    bool isReady() const;

    QString name() const;
    void setName(QString name);


    //Do not move this to a slot!!! You will break things
    //TODO: figure out why this is bad...
    //void stop();

public slots:
    void start();
    void restart();
    void stop();

signals:
    void started();
    void finished();
    void stopped();
    void preparingToStart();
//    void progressed(int step);
    void progressChanged();
    void statusMessage(QString message);
    void numberOfStepsChanged(int numberOfSteps);
    void shouldRerun();
    void nameChanged();
    void threadChanged();

protected:
    void setNumberOfSteps(int steps);
    void setProgress(int progress);
    virtual void runTask() = 0;

protected slots:
    void done();

private:
    mutable QReadWriteLock NumberOfStepsLocker;
    mutable QReadWriteLock StatusLocker;
    mutable QReadWriteLock ProgressLocker;
    mutable QReadWriteLock NameLocker;

    int NumberOfSteps;
    int Progress; //!<

    Status CurrentStatus;

    QList<cwTask*> ChildTasks;
    cwTask* ParentTask;

    QString Name; //!< The name of the task

    void privateStop();
    bool isParentsRunning();

private:
    Q_INVOKABLE void startOnCurrentThread();
    Q_INVOKABLE void changeThreads(QThread* thread);

};

/**
  Returns if the task is ready
  */
inline bool cwTask::isReady() const {
    return status() == Ready;
}




#endif // CWTASK_H
