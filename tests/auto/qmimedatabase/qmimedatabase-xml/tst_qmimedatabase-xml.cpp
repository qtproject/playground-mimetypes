#include "../tst_qmimedatabase.h"
#include <QDebug>
#include <QDir>

tst_qmimedatabase::tst_qmimedatabase()
{
    qputenv("QT_NO_MIME_CACHE", "1");
}

#include "../tst_qmimedatabase.cpp"
