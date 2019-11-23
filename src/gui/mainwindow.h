#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QMainWindow>
#include <atomic>
#include <exception>
#include <filesystem>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_find_button_clicked();

    void keyPressEvent(QKeyEvent *event);

    void on_add_files_clicked();

    void on_add_dir_clicked();

private:
    Ui::MainWindow *ui;
};
#endif  // MAINWINDOW_H
