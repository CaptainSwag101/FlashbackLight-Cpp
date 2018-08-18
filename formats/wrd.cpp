#include "wrd.h"

WRD WRD::fromBytes(QByteArray &bytes)
{
    QDataStream stream(&bytes, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    WRD result;

    ushort stringCount;
    stream >> stringCount;
    ushort labelCount;
    stream >> labelCount;
    ushort paramCount;
    stream >> paramCount;
    ushort sublabelCount;
    stream >> sublabelCount;
    stream.skipRawData(4);

    uint sublabelOffsetsPointer;
    stream >> sublabelOffsetsPointer;
    uint labelOffsetsPointer;
    stream >> labelOffsetsPointer;
    uint labelNamesPointer;
    stream >> labelNamesPointer;
    uint paramsPointer;
    stream >> paramsPointer;
    uint stringsPointer;
    stream >> stringsPointer;

    // We need 2 bytes for each opcode
    while (stream.device()->pos() + 1 < sublabelOffsetsPointer)
    {
        uchar b;
        stream >> b;

        if (b != 0x70)
            throw "Invalid opcode.";

        WRDCmd cmd;
        stream >> cmd.opcode;

        // Copy opcode name and argument info from our reference table
        cmd.name = KNOWN_CMDS[cmd.opcode].name;
        cmd.argTypes = KNOWN_CMDS[cmd.opcode].argTypes;
        cmd.variableLength = KNOWN_CMDS[cmd.opcode].variableLength;

        // Read command arguments, if any
        stream.setByteOrder(QDataStream::BigEndian);
        while (stream.device()->pos() + 1 < sublabelOffsetsPointer)
        {
            if (stream.device()->peek(1).at(0) == 0x70)
                break;

            ushort arg;
            stream >> arg;
            cmd.argData.append(arg);
        }
        stream.setByteOrder(QDataStream::LittleEndian);

        result.code.append(cmd);
    }

    // Read the name for each label
    stream.device()->seek(labelNamesPointer);
    for (ushort i = 0; i < labelCount; ++i)
    {
        uchar nameLen;
        stream >> nameLen;
        QString labelName = QString::fromUtf8(stream.device()->read(nameLen));
        result.labels.append(labelName);
        stream.skipRawData(1);  // Skip null terminator
    }

    // Read plaintext parameter names
    stream.device()->seek(paramsPointer);
    for (ushort i = 0; i < paramCount; ++i)
    {
        uchar paramLen;
        stream >> paramLen;
        QString param = QString::fromUtf8(stream.device()->read(paramLen));
        result.params.append(param);
        stream.skipRawData(1);
    }

    result.externalStrings = (stringsPointer == 0);

    // Read dialogue text strings
    if (result.externalStrings)
    {
        // Strings are stored in the "(current spc name)_text_(region).spc" file,
        // within an STX file with the same name as the current WRD file.

        // TODO: Make this work
        /*
        QString stxFile = QFileInfo(in_file).absolutePath();
        if (stxFile.endsWith(".SPC", Qt::CaseInsensitive))
            stxFile.chop(4);

        QString region = "_US";
        if (stxFile.right(3).startsWith("_"))
        {
            region = stxFile.right(3);
            stxFile.chop(3);
        }

        stxFile.append("_text" + region + ".SPC");
        stxFile.append(QDir::separator());
        stxFile.append(QFileInfo(in_file).fileName());
        stxFile.replace(".wrd", ".stx");

        if (QFileInfo(stxFile).exists())
        {
            QFile f(stxFile);
            f.open(QFile::ReadOnly);
            const QByteArray stxData = f.readAll();
            f.close();
            result.strings = get_stx_strings(stxData);
        }
        */
    }
    else
    {
        stream.device()->seek(stringsPointer);
        for (ushort i = 0; i < stringCount; ++i)
        {
            short stringLen;

            // The string length is a signed byte, so if it's larger than 0x7F,
            // that means the length is actually stored in a signed short,
            // since we can't have a negative string length.
            // ┐(´∀｀)┌
            if ((uchar)stream.device()->peek(1).at(0) >= 0x80)
            {
                stream >> stringLen;
            }
            else
            {
                uchar c;
                stream >> c;
                stringLen = c;
            }

            //ushort *stringData = new ushort[stringLen];
            ushort *stringData = new ushort[stringLen];
            stream.device()->read((char *)stringData, stringLen * 2);
            QString string = QString::fromUtf16(stringData);
            result.strings.append(string);
            delete[] stringData;
        }
    }

    stream.setDevice(0);
    return result;
}
