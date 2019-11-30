#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow{parent}, ui{new Ui::MainWindow} {
    ui->setupUi(this);
    ui->result->setUniformItemSizes(true);
    QSizePolicy policy = ui->progress_bar->sizePolicy();
    policy.setRetainSizeWhenHidden(true);
    ui->progress_bar->setHidden(true);
    ui->progress_bar->setSizePolicy(policy);
    connect(&background_thread, &multithreading_grep::send_info_percentage, this, &MainWindow::new_info_percentage,
            Qt::QueuedConnection);
    connect(&background_thread, &multithreading_grep::send_info_substr, this, &MainWindow::new_info_substr,
            Qt::QueuedConnection);
    connect(&background_thread, &multithreading_grep::send_complete_signal, this, &MainWindow::finish_searching,
            Qt::QueuedConnection);
}

MainWindow::~MainWindow() {
    delete ui;
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

void MainWindow::on_find_cancel_button_clicked() {
    if (ui->find_cancel_button->text() == "Cancel") {
        ui->find_cancel_button->setText("Find");
        ui->progress_bar->setHidden(true);
        background_thread.terminate_process();
    } else {
        if (ui->string_to_find->text().isEmpty() || ui->files_and_dirs->count() == 0) {
            QMessageBox::warning(this, "BAD", "BAD");
        } else {
            total_found = 0;
            std::queue<QString> list;
            for (int row = 0; row < ui->files_and_dirs->count(); ++row) {
                list.push(ui->files_and_dirs->item(row)->text());
            }
            ui->result->clear();
            ui->find_cancel_button->setText("Cancel");
            ui->progress_bar->setHidden(false);
            ui->progress_bar->setValue(0);
            background_thread.new_task(list, ui->string_to_find->text(), !ui->ignore_case->checkState());
        }
    }
}

void MainWindow::finish_searching() {
    ui->find_cancel_button->setText("Find");
    ui->progress_bar->setHidden(true);
    QMessageBox::information(this, "Finish", QString::number(total_found));
}

void MainWindow::new_info_percentage(int percentage) {
    ui->progress_bar->setValue(percentage);
}

void MainWindow::new_info_substr(QString const &new_line) {
    ui->total_amount->setText("Total amount: " + QString::number(++total_found));
    ui->result->addItem(new_line);
}
