#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <queue>
#include <vector>

#include<atomic>
#include<mutex>
#include<exception>
#include <thread>
#include<filesystem>


#include <QMainWindow>

#include <QListWidget>
#include <QDir>
#include <QFileDialog>


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
    void on_files_and_dirs_itemDoubleClicked(QListWidgetItem *item);

    void on_find_button_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
