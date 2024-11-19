#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "CpuThread.h"
#include "MemoryThread.h"
#include <QSemaphore>
#include <psapi.h>
#include <algorithm>
#include <sstream>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>
#include <unistd.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <string>


QSemaphore semaphore(1);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Create and start CPU thread
    CpuThread *cpuThread = new CpuThread();
    connect(cpuThread, &CpuThread::cpuUpdated, this, &MainWindow::updateCpuUsage);
    cpuThread->start();

    // Create and start Memory thread
    MemoryThread *memThread = new MemoryThread();
    connect(memThread, &MemoryThread::memoryUpdated, this, &MainWindow::updateMemoryUsage);
    memThread->start();

    // Connect the "Start Monitoring" button to startMonitoring slot
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::startMonitoring);
    // Connect the button click to the on_listButton_clicked slot
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::on_listButton_clicked);
    //new


    // Initialize Timer for real-time updates
    /*timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateProcessInfo);*/
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::on_killButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (cpuThread) {
        cpuThread->quit();
        cpuThread->wait();
        delete cpuThread;
    }
    if (memThread) {
        memThread->quit();
        memThread->wait();
        delete memThread;
    }
}

// Slot to start CPU and memory monitoring threads
void MainWindow::startMonitoring() {
    if (!cpuThread) { // Check if threads are already running
        cpuThread = new CpuThread();
        connect(cpuThread, &CpuThread::cpuUpdated, this, &MainWindow::updateCpuUsage);
        cpuThread->start();
    }

    if (!memThread) {
        memThread = new MemoryThread();
        connect(memThread, &MemoryThread::memoryUpdated, this, &MainWindow::updateMemoryUsage);
        memThread->start();
    }
}

void MainWindow::updateCpuUsage(double cpuUsage) {
    ui->progressBar->setRange(0, 1000);
    ui->progressBar->setValue(static_cast<int>(cpuUsage * 10));  // For 1 decimal place

    // Display the exact percentage with one decimal in the progress bar
    ui->progressBar->setFormat(QString::number(cpuUsage, 'f', 2) + " %");
}

void MainWindow::updateMemoryUsage(double memUsage) {
    ui->progressBar_2->setRange(0, 1000);
    ui->progressBar_2->setValue(static_cast<int>(memUsage * 10));  // Multiply by 10 for one decimal place

    // Display the exact memory usage percentage with one decimal in the progress bar
    ui->progressBar_2->setFormat(QString::number(memUsage, 'f', 2) + " %");
}

//list
std::string MainWindow::getProcessName(DWORD processID) {
    char processName[MAX_PATH] = "System proccess";

    // Open process handle
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess) {
        HMODULE hMod;
        DWORD cbNeeded;

        // Get the first module (the executable)
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
            GetModuleBaseNameA(hProcess, hMod, processName, sizeof(processName) / sizeof(char));
        }
    }
    CloseHandle(hProcess);
    return std::string(processName);
}

std::vector<MainWindow::ProcessInfo> MainWindow::getProcessList() {
    std::lock_guard<std::mutex> lock(processMutex);  // Lock with mutex

    std::vector<ProcessInfo> processList;
    DWORD processes[1024], countNeeded;

    // Get the list of process identifiers
    if (!EnumProcesses(processes, sizeof(processes), &countNeeded)) {
        return processList;  // Return an empty list on failure
    }

    DWORD processCount = countNeeded / sizeof(DWORD);

    for (DWORD i = 0; i < processCount; i++) {
        if (processes[i] != 0) {
            ProcessInfo pInfo;
            pInfo.pid = processes[i];
            pInfo.name = getProcessName(processes[i]);
            processList.push_back(pInfo);
        }
    }

    // Sort the process list alphabetically by process name
    std::sort(processList.begin(), processList.end(), [](const ProcessInfo &a, const ProcessInfo &b) {
        return a.name < b.name;
    });

    return processList;
}

void MainWindow::on_listButton_clicked() {
    // Get the list of processes
    std::vector<ProcessInfo> processList = getProcessList();

    // Prepare the output string
    std::stringstream output;
    for (const auto& process : processList) {
        output << "PID: " << process.pid << " | Name: " << process.name << "\n";
    }

    // Display in the QTextEdit
    ui->textBrowser->setText(QString::fromStdString(output.str()));
}

//new
void MainWindow::on_pushButton_check_clicked() {
    // Get PID from user input
    bool ok;
    DWORD pid = ui->lineEdit->text().toUInt(&ok);

    if (!ok || pid == 0) {
        ui->label_4->setText("Invalid PID");
        ui->label_5->setText("Invalid PID");
        return;
    }

    // Fetch CPU and RAM usage
    double cpuUsage = getCPUUsage(pid);
    double memoryUsage = getMemoryUsage(pid);

    // Update CPU progress bar
    ui->progressBar_3->setRange(0, 100);
    ui->progressBar_3->setValue(static_cast<int>(cpuUsage)); // Set the integer value
    ui->progressBar_3->setFormat(QString("CPU: %0.1%").arg(cpuUsage, 0, 'f', 2)); // Display percentage in text

    // Update Memory progress bar
    ui->progressBar_4->setRange(0, 100);
    ui->progressBar_4->setValue(static_cast<int>(memoryUsage)); // Set the integer value
    ui->progressBar_4->setFormat(QString("Memory: %1%").arg(memoryUsage, 0, 'f', 2)); // Display percentage in text


}
double MainWindow::getCPUUsage(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return 0.0;

    FILETIME creationTime, exitTime, kernelTime, userTime;
    static FILETIME prevKernelTime = {}, prevUserTime = {};
    double cpuUsage = 0.0;

    if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
        ULARGE_INTEGER kernel, user, prevKernel, prevUser;

        kernel.LowPart = kernelTime.dwLowDateTime;
        kernel.HighPart = kernelTime.dwHighDateTime;

        user.LowPart = userTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;

        prevKernel.LowPart = prevKernelTime.dwLowDateTime;
        prevKernel.HighPart = prevKernelTime.dwHighDateTime;

        prevUser.LowPart = prevUserTime.dwLowDateTime;
        prevUser.HighPart = prevUserTime.dwHighDateTime;

        cpuUsage = (kernel.QuadPart - prevKernel.QuadPart + user.QuadPart - prevUser.QuadPart) / 10000.0;

        prevKernelTime = kernelTime;
        prevUserTime = userTime;
    }

    CloseHandle(hProcess);
    return cpuUsage;
}

double MainWindow::getMemoryUsage(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return 0.0;

    PROCESS_MEMORY_COUNTERS pmc;
    double memoryUsage = 0.0;

    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        if (GlobalMemoryStatusEx(&memStatus)) {
            memoryUsage = (static_cast<double>(pmc.WorkingSetSize) / memStatus.ullTotalPhys) * 100.0;
        }
    }

    CloseHandle(hProcess);
    return memoryUsage;
}

bool MainWindow::killProcess(DWORD pid) {
    std::lock_guard<std::mutex> lock(processMutex);  // Locking with mutex for thread safety

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == nullptr) {
        return false;  // Could not open the process
    }

    // Try to terminate the process
    BOOL result = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);  // Ensure we close the handle

    return result != 0;  // Return true if termination succeeded
}
void MainWindow::on_killButton_clicked() {
    QString pidText = ui->lineEdit->text();  // Get PID from input field
    bool ok;
    DWORD pid = pidText.toULong(&ok);  // Convert to DWORD (unsigned long) for PID

    if (!ok) {
        QMessageBox::warning(this, "Invalid PID", "Please enter a valid numeric PID.");
        return;
    }

    if (killProcess(pid)) {
        QMessageBox::information(this, "Process Terminated", "The process has been successfully terminated.");
    } else {
        QMessageBox::warning(this, "Termination Failed", "Failed to terminate the process. It might require administrator privileges or be protected.");
    }
}

