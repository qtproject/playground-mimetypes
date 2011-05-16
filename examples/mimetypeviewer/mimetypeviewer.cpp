#include "mimetypeviewer.h"
#include "ui_mimetypeviewer.h"

#include <QFileDialog>
#include <QMimeDatabase>
#include <QDebug>

MimeTypeViewer::MimeTypeViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MimeTypeViewer),
    dataBase(new QMimeDatabase)
{
    ui->setupUi(this);
    connect(ui->addTypesButton, SIGNAL(clicked()), SLOT(onAddTypesButtonClicked()));
    connect(ui->openFileButton, SIGNAL(clicked()), SLOT(onOpenFileButtonClicked()));
}

MimeTypeViewer::~MimeTypeViewer()
{
    delete ui;
}

void MimeTypeViewer::addDatabase(const QString &file)
{
    QString errorString;
    if (!dataBase->addMimeTypes(file, &errorString))
        qWarning() << "Can't add types from" << file << ":" << errorString;

    updateTypes();
}

void MimeTypeViewer::onAddTypesButtonClicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select database"));
    if (file.isEmpty())
        return;

    addDatabase(file);
}

void MimeTypeViewer::onOpenFileButtonClicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select file"));
    if (file.isEmpty())
        return;

    ui->fileLineEdit->setText(file);
    QMimeType mime = dataBase->findByFile(QFileInfo(file));
    if (mime.isValid())
        setMimeType(mime);
}

void MimeTypeViewer::updateTypes()
{
    ui->listWidget->clear();
    foreach (const QMimeType &mime, dataBase->mimeTypes()) {
        ui->listWidget->addItem(mime.type());
    }
    ui->listWidget->sortItems();
}

void MimeTypeViewer::setMimeType(const QMimeType &mime)
{
    ui->mimeTypeLabel->setText(mime.type());
    ui->aliasesLabel->setText(mime.aliases().join(", "));
    ui->commentLabel->setText(mime.comment());

    QStringList list;
    foreach (const QMimeGlobPattern &pattern, mime.globPatterns()) {
        list.append(pattern.regExp().pattern());
    }
    ui->globPatternsLabel->setText(list.join(", "));
    ui->subClassesOfLabel->setText(mime.subClassOf().join(", "));
}
