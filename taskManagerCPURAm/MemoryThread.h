#ifndef MEMORYTHREAD_H
#define MEMORYTHREAD_H

#include <QThread>

class MemoryThread : public QThread {
    Q_OBJECT
public:
    void run() override;

signals:
    void memoryUpdated(double memUsage);
};

#endif // MEMORYTHREAD_H
