#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <mutex>
#include <thread>

#include "../grep/grep.h"

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
    void keyPressEvent(QKeyEvent *event);

    void on_add_files_clicked();

    void on_add_dir_clicked();

    void on_find_cancel_button_clicked();

    void finish_searching();

    void new_info_percentage(int percentage);

    void new_info_substr(QString const &new_line);

private:
    Ui::MainWindow *ui;

    multithreading_grep background_thread;
    size_t total_found;
};
#endif  // MAINWINDOW_H
