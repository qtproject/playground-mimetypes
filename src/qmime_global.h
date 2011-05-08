#ifndef QMIME_GLOBAL_H
#define QMIME_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QMIME_LIBRARY)
#  define QMIME_EXPORT Q_DECL_EXPORT
#else
#  define QMIME_EXPORT Q_DECL_IMPORT
#endif

#endif // QMIME_GLOBAL_H
