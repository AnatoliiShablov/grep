#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , ui{new Ui::MainWindow}

{
    ui->setupUi(this);
    QListWidgetItem *add;
    ui->files_and_dirs->addItem(add = new QListWidgetItem("..."));
    add->setTextAlignment(Qt::AlignCenter);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_files_and_dirs_itemDoubleClicked(QListWidgetItem *item)
{

    QString path = QFileDialog::getOpenFileName(this,"Open file or directory",QDir::homePath());
    if(path.length() == 0)
    {
        return;
    }
    if(item->text() == "...")
    {
        ui->files_and_dirs->insertItem(ui->files_and_dirs->currentRow(),path);
    }
    else
    {
        item->setText(path);
    }
}

void MainWindow::on_find_button_clicked()
{
    ui->result->clear();
    for (int row = 0; row  + 1< ui->files_and_dirs->count(); ++row)
    {
        QListWidgetItem *item = ui->files_and_dirs->item(row);
        auto path = QDir::cleanPath(item->text());
        ui->result->addItem(item->text());
        QString previous(path);
        while((path = QFileInfo(path).path()).length() < previous.length())
        {
            previous = path;
            ui->result->addItem(previous);
        }
    }
}
