#ifndef MIMETYPEVIEWER_H
#define MIMETYPEVIEWER_H

#include <QWidget>

class QMimeDatabase;
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
    void onOpenFileButtonClicked();

private:
    Ui::MimeTypeViewer *ui;
    QMimeDatabase *dataBase;

    void updateTypes();
    void setMimeType(const QMimeType &mime);
};

#endif // MIMETYPEVIEWER_H
