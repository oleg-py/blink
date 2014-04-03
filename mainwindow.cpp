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
    auto availableTypes = core->availableSelectorTypes();
    ui->anime_selectorFormat->addItems(availableTypes);
    ui->manga_selectorFormat->addItems(availableTypes);
    core->setAnimeOutFormat(availableTypes.at(0));
    core->setMangaOutFormat(availableTypes.at(0));
    connect(core, &BlinkCore::animelistTotal,
            [this](int i) { ui->anime_progress->setMaximum(qMax(i, 1)); });
    connect(core, &BlinkCore::animelistProcessed,
            ui->anime_progress, &QProgressBar::setValue);
    connect(core, &BlinkCore::mangalistTotal,
            [this](int i) { ui->manga_progress->setMaximum(qMax(i, 1)); });
    connect(core, &BlinkCore::mangalistProcessed,
            ui->manga_progress, &QProgressBar::setValue);

    connect(core, &BlinkCore::finished,
            this, &MainWindow::onBlinkFinished);
    connect(core, &BlinkCore::error,
            this, &MainWindow::onBlinkError);

    connect(ui->usernameEdit, &QLineEdit::textChanged,
            core, &BlinkCore::setUsername);
    connect(ui->usernameEdit, &QLineEdit::textChanged, this, &MainWindow::purgeProgress);
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
    if (ui->usernameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("You must specify a username"));
        return;
    }
    if (!ui->anime_group->isChecked() && !ui->manga_group->isChecked()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Nothing to generate covers for.<br />"
                                "Please at least one list type"));
        return;
    }
    if (ui->anime_group->isChecked() && ui->manga_group->isChecked()
            && ui->anime_path->text() == ui->manga_path->text()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("You cannot save both cover files into one"));
        return;
    }
    if (ui->anime_group->isChecked() && ui->anime_path->text().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("You must specify a path to save anime list covers"));
        return;
    }
    if (ui->manga_group->isChecked() && ui->manga_path->text().isEmpty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("You must specify a path to save manga list covers"));
        return;
    }
    enableControls(false);
    core->startProcessing();
}

void MainWindow::enableControls(bool enable)
{
    ui->anime_group->setEnabled(enable);
    ui->manga_group->setEnabled(enable);
    ui->usernameEdit->setEnabled(enable);
    ui->blinkButton->setEnabled(enable);
}

void MainWindow::purgeProgress()
{
    ui->anime_progress->setValue(0);
    ui->anime_progress->setMaximum(1);
    ui->manga_progress->setValue(0);
    ui->manga_progress->setMaximum(1);
}

void MainWindow::onBlinkFinished()
{
    // this is a workaround because MAL can report incorrect quantities
    // so I just quickly change the values to indicate correct ones
    ui->anime_progress->setMaximum(qMax(1,ui->anime_progress->value()));
    ui->manga_progress->setMaximum(qMax(1,ui->manga_progress->value()));
    QMessageBox::information(this, tr("Info"),
                             tr("Saving completed"));
    enableControls(true);
}

void MainWindow::onBlinkError(QString msg)
{
    enableControls(true);
    QMessageBox::critical(this, tr("Error"),
                          msg);
}
