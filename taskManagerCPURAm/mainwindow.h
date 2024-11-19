#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QTextEdit>
#include <QTimer>
#include <QSemaphore>
#include "CpuThread.h"
#include "MemoryThread.h"
#include <QTextEdit>
#include <QPushButton>
#include <vector>
#include <string>
#include <windows.h>
#include <mutex>
#include <QGraphicsView>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //new
    /*void onButtonClick();
    void updateProcessInfo();*/
    //new
    void on_killButton_clicked();//new
    void on_listButton_clicked();  // Slot for button click
    void startMonitoring();               // New slot to start monitoring
    void updateCpuUsage(double cpuUsage);  // Slot to update CPU usage
    void updateMemoryUsage(double memUsage);

private:

    Ui::MainWindow *ui;
    CpuThread *cpuThread;   // Threads to be initialized on button click
    MemoryThread *memThread;
    std::mutex processMutex;
    //2
    QTimer *timer;
    DWORD pid;                   // Process ID
    HANDLE processHandle;
    /*new
    QLineEdit *pidInput;      // Input field for PID
    QPushButton *submitButton; // Submit button
    QLabel *cpuLabel;         // Label to show CPU usage
    QLabel *ramLabel;         // Label to show RAM usage
    QTimer *timer;            // Timer to update every second

    FILETIME prevKernelTime;  // Previous kernel time for CPU usage calculation
    FILETIME prevUserTime;    // Previous user time for CPU usage calculation
     DWORD pid;                // Process ID for which we will fetch info

    void getProcessInfo(DWORD pid);
    double calculateCPUUsage(FILETIME &prevKernel, FILETIME &prevUser);
    double getMemoryUsage(DWORD pid);
    long getTotalMemory();
    */



    // Struct to hold process information
    struct ProcessInfo {
        DWORD pid;
        std::string name;
    };

    std::vector<ProcessInfo> getProcessList();  // Method to get process list
    std::string getProcessName(DWORD processID);
    //new2
    bool killProcess(DWORD pid);
};
#endif // MAINWINDOW_H
