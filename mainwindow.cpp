#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logic/blinkcore.h"
#include <QtAlgorithms>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    core(new BlinkCore(this)),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->anime_selectorFormat->addItems(core->availableSelectorTypes());
    ui->manga_selectorFormat->addItems(core->availableSelectorTypes());
    connect(core, &BlinkCore::animelistTotal,
            [this](int i) { ui->anime_progress->setMaximum(qMax(i, 1)); });
    connect(core, &BlinkCore::animelistProcessed,
            ui->anime_progress, &QProgressBar::setValue);
    connect(core, &BlinkCore::mangalistTotal,
            [this](int i) { ui->manga_progress->setMaximum(qMax(i, 1)); });
    connect(core, &BlinkCore::mangalistProcessed,
            ui->manga_progress, &QProgressBar::setValue);

    connect(core, &BlinkCore::finished,
            [this] {setEnabled(true); QMessageBox::information(
                    this, tr("Finished!"), tr("Everything is done"));});
    connect(core, &BlinkCore::error,
            [this] (QString s) {setEnabled(true); QMessageBox::critical(this, tr("Error!"), s);});

    connect(ui->usernameEdit, &QLineEdit::textChanged,
            core, &BlinkCore::setUsername);
    connect(ui->anime_group, &QGroupBox::toggled, core, &BlinkCore::processAnimelist);
    connect(ui->manga_group, &QGroupBox::toggled, core, &BlinkCore::processMangalist);
    connect(ui->anime_path, &QLineEdit::textChanged, core, &BlinkCore::setAnimeOutPath);
    connect(ui->manga_path, &QLineEdit::textChanged, core, &BlinkCore::setMangaOutPath);
    connect(ui->anime_selectorFormat, &QComboBox::currentTextChanged, core, &BlinkCore::setAnimeOutFormat);
    connect(ui->manga_selectorFormat, &QComboBox::currentTextChanged, core, &BlinkCore::setMangaOutFormat);
    connect(ui->anime_browseButton, &QPushButton::clicked, [this]{writePath(ui->anime_path);});
    connect(ui->manga_browseButton, &QPushButton::clicked, [this]{writePath(ui->manga_path);});
    connect(ui->blinkButton, &QPushButton::clicked, this, &MainWindow::onBlinkButtonPressed);
}

MainWindow::~MainWindow()
{
    delete core;
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::writePath(QLineEdit *le)
{
    QString path
            = QFileDialog::getSaveFileName(this, tr("Save covers file as"),
                                           QString(),
                                           tr("Cascading style sheet files (*.css)"
                                              ";;All files (*.*)"));
    if (!path.isEmpty()) le->setText(path);
}

void MainWindow::onBlinkButtonPressed()
{
    setDisabled(true);
    // TODO better checking here
    core->startProcessing();
}
