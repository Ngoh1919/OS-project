#include "CpuThread.h"
#include <windows.h>

extern QSemaphore semaphore;  // Using the global semaphore

double calculateCPUUsage() {
    static ULONGLONG prevIdleTime = 0, prevKernelTime = 0, prevUserTime = 0;
    FILETIME idleTime, kernelTime, userTime;

    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        ULONGLONG idle = ((ULONGLONG)idleTime.dwLowDateTime | ((ULONGLONG)idleTime.dwHighDateTime << 32));
        ULONGLONG kernel = ((ULONGLONG)kernelTime.dwLowDateTime | ((ULONGLONG)kernelTime.dwHighDateTime << 32));
        ULONGLONG user = ((ULONGLONG)userTime.dwLowDateTime | ((ULONGLONG)userTime.dwHighDateTime << 32));

        ULONGLONG total = kernel + user;
        ULONGLONG totalTime = total - prevKernelTime - prevUserTime;
        ULONGLONG idleTimeDiff = idle - prevIdleTime;

        prevKernelTime = kernel;
        prevUserTime = user;
        prevIdleTime = idle;

        return 100.0 * (1.0 - (double)idleTimeDiff / totalTime);
    }
    return 0.0;
}

void CpuThread::run() {
    while (true) {
        semaphore.acquire();
        double cpuUsage = calculateCPUUsage();
        emit cpuUpdated(cpuUsage);
        semaphore.release();
        sleep(1);
    }
}
