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
#include <QObject>



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
    void on_pushButton_check_clicked();
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
    //new
    double getCPUUsage(DWORD pid);      // Fetch CPU usage
    double getMemoryUsage(DWORD pid);
    //new





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
