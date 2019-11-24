#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , ui{new Ui::MainWindow}
    , searching{false}
    , background_scheduler{nullptr}

{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    searching = false;
    delete background_scheduler;
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
    std::lock_guard<std::mutex> locker{change_info_mutex};
    if (searching) {
        searching = false;
        delete background_scheduler;
        background_scheduler = nullptr;
        ui->find_cancel_button->setText("Find");
        ui->progress_bar->setHidden(true);
    } else {
        if (ui->string_to_find->text().isEmpty() || ui->files_and_dirs->count() == 0) {
            QMessageBox::warning(this, "BAD", "BAD");
        } else {
            delete background_scheduler;
            searching = true;
            total_found = 0;
            QStringList list;
            for (int row = 0; row < ui->files_and_dirs->count(); ++row) {
                list.append(ui->files_and_dirs->item(row)->text());
            }
            background_scheduler = new grep_with_scheduler(
                std::ref(searching), std::move(list), ui->ignore_case, [this] { this->finish_searching(); },
                [this](int percentage, QStringList const &new_lines) { this->new_info(percentage, new_lines); });
            ui->find_cancel_button->setText("Cancel");
            ui->progress_bar->setHidden(false);
            ui->progress_bar->setValue(0);
        }
    }
}

void MainWindow::finish_searching() {
    if (searching) {
        std::lock_guard<std::mutex> locker{change_info_mutex};
        searching = false;
        ui->find_cancel_button->setText("Find");
        ui->progress_bar->setHidden(true);
        QMessageBox::information(this, "Finish", QString::number(total_found));
    }
}

void MainWindow::new_info(int percentage, QStringList const &new_lines) {
    std::lock_guard<std::mutex> locker{change_info_mutex};
    ui->total_amount->setText("Total amount: " + QString::number(total_found += static_cast<size_t>(new_lines.count())));
    ui->result->addItems(new_lines);
    ui->progress_bar->setValue(percentage);
}
