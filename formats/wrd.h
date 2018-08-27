#ifndef WRD_H
#define WRD_H

#include <QDataStream>
#include <QVector>
#include "formats/stx.h"
#include "formats/wrdcmd.h"

class WRD
{
public:
    static WRD fromBytes(QByteArray &bytes);
    static QByteArray toBytes(WRD &wrd);

    QString filename;
    QStringList labels;
    QStringList params;
    QStringList strings;
    QList<WRDCmd> code;
    bool externalStrings;
};

#endif // WRD_H
