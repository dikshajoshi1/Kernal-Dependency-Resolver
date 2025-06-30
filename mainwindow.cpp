#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Module Dependency Analyzer");
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::hasCycle(const std::string& module, 
                         std::unordered_set<std::string>& visitedCycle, 
                         std::unordered_set<std::string>& recStack) 
{
    if (recStack.count(module)) return true;
    if (visitedCycle.count(module)) return false;

    visitedCycle.insert(module);
    recStack.insert(module);

    for (const std::string& dep : graph[module]) {
        if (hasCycle(dep, visitedCycle, recStack))
            return true;
    }

    recStack.erase(module);
    return false;
}

void MainWindow::dfs(const std::string& module) 
{
    if (visited.count(module)) return;
    if (tempVisited.count(module)) {
        QMessageBox::critical(this, "Error", 
            QString("Detected circular dependency involving %1").arg(QString::fromStdString(module)));
        return;
    }

    tempVisited.insert(module);
    for (const std::string& dep : graph[module]) {
        dfs(dep);
    }
    tempVisited.erase(module);
    visited.insert(module);
    loadOrder.push_back(module);
}

void MainWindow::topologicalSort(const std::string& targetModule) 
{
    dfs(targetModule);
}

void MainWindow::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open modules.dep", "", "Dependency Files (*.dep);;All Files (*)");
    if (!fileName.isEmpty()) {
        ui->filePathEdit->setText(fileName);
    }
}

void MainWindow::on_analyzeButton_clicked()
{
    QString depFilePath = ui->filePathEdit->text();
    QString targetModule = ui->moduleEdit->text();

    if (depFilePath.isEmpty() || targetModule.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please specify both file path and module name");
        return;
    }

    graph.clear();
    visited.clear();
    tempVisited.clear();
    loadOrder.clear();
    ui->resultText->clear();

    std::ifstream file(depFilePath.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, "Error", "Could not open file");
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        std::string module = line.substr(0, colonPos);
        std::string depsLine = line.substr(colonPos + 1);

        std::istringstream ss(depsLine);
        std::string dep;
        while (ss >> dep) {
            graph[module].push_back(dep);
        }
        if (!graph.count(module)) graph[module] = {};
    }

    std::string target = targetModule.toStdString();
    if (!graph.count(target)) {
        QMessageBox::critical(this, "Error", "Module not found in dep file");
        return;
    }

    std::unordered_set<std::string> visitedCycle;
    std::unordered_set<std::string> recStack;

    if (hasCycle(target, visitedCycle, recStack)) {
        QMessageBox::critical(this, "Error", 
            QString("Circular dependency detected involving '%1'").arg(targetModule));
        return;
    }

    topologicalSort(target);

    ui->resultText->append("Correct loading order:");
    for (const std::string& mod : loadOrder) {
        ui->resultText->append(QString::fromStdString(mod));
    }
}
