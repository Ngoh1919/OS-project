#ifndef CPUTHREAD_H
#define CPUTHREAD_H

#include <QThread>
#include <QSemaphore>

class CpuThread : public QThread {
    Q_OBJECT  // Necessary for signal-slot system
public:
    void run() override;

signals:
    void cpuUpdated(double cpuUsage);
};

#endif // CPUTHREAD_H
