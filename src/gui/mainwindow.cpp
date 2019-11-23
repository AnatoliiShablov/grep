#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , ui{new Ui::MainWindow}

{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_find_button_clicked() {
    ui->result->clear();
    for (int row = 0; row < ui->files_and_dirs->count(); ++row) {
        QListWidgetItem *item = ui->files_and_dirs->item(row);
        auto path = QDir::cleanPath(item->text());
        QString previous(path);
        while ((path = QFileInfo(path).path()).length() < previous.length()) {
            ui->result->addItem(previous.right(previous.size() - path.length()));
            previous = path;
        }
        ui->result->addItem(previous);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Delete) {
        delete ui->files_and_dirs->takeItem(ui->files_and_dirs->currentRow());
    }
}

void MainWindow::on_add_files_clicked() {
    QFileDialog dialog(this, "Open Files", QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
        for (auto file : fileNames) {
            ui->files_and_dirs->addItem(file);
        }
    }
}

void MainWindow::on_add_dir_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Open Directories", QDir::homePath());
    if (!dir.isEmpty()) {
        ui->files_and_dirs->addItem(dir);
    }
}
