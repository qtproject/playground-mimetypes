#ifndef MIMETYPEVIEWER_H
#define MIMETYPEVIEWER_H

#include <QWidget>

class QMimeDatabase;
class QMimeDatabaseBuilder;
class QMimeType;

namespace Ui {
    class MimeTypeViewer;
}

class MimeTypeViewer : public QWidget
{
    Q_OBJECT

public:
    explicit MimeTypeViewer(QWidget *parent = 0);
    ~MimeTypeViewer();

    void addDatabase(const QString &file);

public slots:
    void onAddTypesButtonClicked();
    void onOpenFileButtonClicked();

private:
    Ui::MimeTypeViewer *ui;
    QMimeDatabase *dataBase;
    QMimeDatabaseBuilder *databaseBuilder;

    void updateTypes();
    void setMimeType(const QMimeType &mime);
};

#endif // MIMETYPEVIEWER_H
