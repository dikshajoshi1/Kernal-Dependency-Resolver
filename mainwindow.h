#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_analyzeButton_clicked();
    void on_browseButton_clicked();

private:
    Ui::MainWindow *ui;
    
    std::unordered_map<std::string, std::vector<std::string>> graph;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> tempVisited;
    std::vector<std::string> loadOrder;

    bool hasCycle(const std::string& module, 
                 std::unordered_set<std::string>& visitedCycle, 
                 std::unordered_set<std::string>& recStack);
    void dfs(const std::string& module);
    void topologicalSort(const std::string& targetModule);
};
#endif
