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
    // Connect button click to the slot
    /*connect(ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::onButtonClick);*/

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
    char processName[MAX_PATH] = "<unknown>";

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

/*new
void MainWindow::onButtonClick()
{
    bool ok;
    pid = ui->lineEdit->text().toUInt(&ok);

    if (!ok) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid PID.");
        return;
    }

    // Start the timer for real-time updates
    timer->start(1000);  // Update every second
    getProcessInfo(pid);
}

void MainWindow::updateProcessInfo()
{
    if (pid == 0) {
        return;
    }

    // Calculate real-time CPU usage percentage
    double cpuUsage = calculateCPUUsage(prevKernelTime, prevUserTime);

    // Update the CPU label with the new usage
    ui->label_4->setText(QString("CPU Usage: %1%").arg(cpuUsage));

    // Get memory usage percentage
    double memoryUsage = getMemoryUsage(pid);
    ui->label_5->setText(QString("Memory Usage: %1%").arg(memoryUsage));
}

double MainWindow::calculateCPUUsage(FILETIME &prevKernel, FILETIME &prevUser)
{
    // Get current times for the process
    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) {
        return 0.0;
    }

    if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
        ULARGE_INTEGER kernelTime, userTime;
        kernelTime.LowPart = ftKernel.dwLowDateTime;
        kernelTime.HighPart = ftKernel.dwHighDateTime;
        userTime.LowPart = ftUser.dwLowDateTime;
        userTime.HighPart = ftUser.dwHighDateTime;

        // Calculate the difference in time between current and previous readings
        double kernelDelta = (kernelTime.QuadPart - prevKernel.QuadPart) / 10000.0;  // Convert to milliseconds
        double userDelta = (userTime.QuadPart - prevUser.QuadPart) / 10000.0;  // Convert to milliseconds

        // Simple approximation of CPU usage
        double cpuUsage = (kernelDelta + userDelta) / 100.0;
        prevKernel = ftKernel;
        prevUser = ftUser;

        CloseHandle(hProcess);
        return cpuUsage;
    }
}

double MainWindow::getMemoryUsage(DWORD pid)
{
    // Retrieve process memory information
    PROCESS_MEMORY_COUNTERS pmc;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == NULL) {
        return -1;
    }

    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        // Get total physical memory and calculate the percentage
        double totalMemory = getTotalMemory();
        double memoryUsage = (pmc.WorkingSetSize / totalMemory) * 100.0;
        CloseHandle(hProcess);
        return memoryUsage;
    }

    CloseHandle(hProcess);
    return -1;
}

DWORD MainWindow::getTotalMemory()
{
    MEMORYSTATUS status;
    GlobalMemoryStatus(&status);
    return status.dwTotalPhys;  // Total physical memory in bytes
}

*/
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

