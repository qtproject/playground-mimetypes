#ifndef QMIMETYPE_GLOBAL_H
#define QMIMETYPE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QMIMETYPE_LIBRARY)
#  define QMIMETYPE_EXPORT Q_DECL_EXPORT
#else
#  define QMIMETYPE_EXPORT Q_DECL_IMPORT
#endif

#endif // QMIMETYPE_GLOBAL_H
