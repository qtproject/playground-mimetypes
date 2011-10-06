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
    ui->addTypesButton->hide(); // hack
    connect(ui->openFileButton, SIGNAL(clicked()), SLOT(onOpenFileButtonClicked()));
}

MimeTypeViewer::~MimeTypeViewer()
{
    delete ui;
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
    ui->localeCommentLabel->setText(mime.localeComment());
    ui->genericIconNameLabel->setText(mime.genericIconName());

    ui->globPatternsLabel->setText(mime.globPatterns().join(", "));
    ui->subClassesOfLabel->setText(mime.subClassOf().join(", "));

    ui->suffixesLabel->setText(mime.suffixes().join(", "));
    ui->preferredSuffixLabel->setText(mime.preferredSuffix());
    ui->filterStringLabel->setText(mime.filterString());
}
