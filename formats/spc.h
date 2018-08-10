#ifndef SPC_H
#define SPC_H

#include <QDataStream>
#include <QString>
#include <QVector>
#include "spcentry.h"

class SPC
{
public:
    static SPC fromBytes(QDataStream &stream, int len = -1);
    static QByteArray toBytes(SPC &spc);

    QString filename;
    QByteArray unk1;
    uint unk2;
    QVector<SPCEntry> files;
};

#endif // SPC_H
