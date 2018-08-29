#ifndef WRDCMD_H
#define WRDCMD_H

#include <QVector>

class WRDCmd
{
public:
    QString getName() const;
    QVector<uchar> getArgTypes() const;   // 0 = flag, 1 = raw number, 2 = string, 3 = label name
    bool isVarLength() const;

    static const QVector<QString> NAME_LIST;
    static const QVector<QVector<uchar>> ARGTYPE_LIST;
    uchar opcode;
    QVector<ushort> argData;
};

#endif // WRDCMD_H
