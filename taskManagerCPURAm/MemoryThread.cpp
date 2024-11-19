#include "MemoryThread.h"
#include "qsemaphore.h"
#include <windows.h>
#include <iostream>

extern QSemaphore semaphore;

double calculateMemoryUsage() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        double totalPhysMem = memInfo.ullTotalPhys;
        double freePhysMem = memInfo.ullAvailPhys;
        return 100.0 * (1.0 - (freePhysMem / totalPhysMem));
    }
    return 0.0;
}

void MemoryThread::run() {
    while (true) {
        semaphore.acquire();
        double memUsage = calculateMemoryUsage();
        emit memoryUpdated(memUsage);
        semaphore.release();
        sleep(1);
    }
}
