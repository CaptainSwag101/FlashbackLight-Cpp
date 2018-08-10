#include "scripteditor.h"
#include "ui_scripteditor.h"
#include "common.h"

ScriptEditor::ScriptEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);
}

ScriptEditor::~ScriptEditor()
{
    delete ui;
}

void ScriptEditor::populateCodeList()
{
    QStringList parsedCmds;
    for (int x = 0; x < currentWRD.code.count(); ++x)
    {
        WRDCmd cmd = currentWRD.code.at(x);
        QString str = cmd.name + " (";

        for (int i = 0; i < cmd.args.count(); ++i)
        {
            const ushort val = cmd.args.at(i);

            if (i < cmd.argTypes.count())
                switch (cmd.argTypes.at(i))
                {
                case 0: // flag/plaintext parameter
                    if (val < currentWRD.params.count())
                        str += currentWRD.params.at(val);
                    else
                        str += "!" + QString::number(val) + "!";
                    break;
                case 1: // raw number
                    str += QString::number(val);
                    break;
                case 2: // dialogue string
                    if (val < currentWRD.strings.count())
                        str += currentWRD.strings.at(val);
                    else
                        str += "!" + QString::number(val) + "!";
                    break;
                case 3: // label name
                    if (val < currentWRD.labels.count())
                        str += currentWRD.labels.at(val);
                    else
                        str += "!" + QString::number(val) + "!";
                    break;
                }
            else
                str += currentWRD.params.at(val);

            if (i + 1 < cmd.args.count())
                str += ", ";
        }
        str += ")";

        parsedCmds << str;
    }
    ui->codeList->clear();
    ui->codeList->addItems(parsedCmds);
}

void ScriptEditor::on_codeList_activated(const QModelIndex &index)
{
    // Populate the opcode ComboBox with all known opcodes
    ui->cbOpcode->blockSignals(true);
    ui->cbOpcode->clear();

    for (const WRDCmd &known : KNOWN_CMDS)
    {
        ui->cbOpcode->addItem(known.name);
    }

    //ui->spinboxParamNum->setValue(1);
    on_spinboxParamNum_valueChanged(1);
}

void ScriptEditor::on_spinboxParamNum_valueChanged(int i)
{
    const WRDCmd cmd = currentWRD.code.at(ui->codeList->currentRow());
    ui->cbOpcode->setCurrentIndex(cmd.opcode);
    ui->labelOpcodeIndex->setText("0x" + QString::number(cmd.opcode, 16));
    ui->cbOpcode->blockSignals(false);


    ui->cbParamList->blockSignals(true);
    ui->spinboxParamNum->blockSignals(true);


    const int argCount = cmd.args.count();
    if (argCount > 0)
    {
        ui->cbParamList->setEnabled(true);
        ui->spinboxParamNum->setEnabled(true);
        ui->spinboxParamNum->setMinimum(1);
        ui->spinboxParamNum->setMaximum(argCount);
        ui->labelParamTotal->setText(QString::number(argCount));


        ui->cbParamList->clear();
        ui->cbParamList->setEditable(false);
        const ushort val = cmd.args.at(i - 1);

        if (i - 1 < cmd.argTypes.count())   // The visual number is one-based, so we need to adjust everything here.
            switch (cmd.argTypes.at(i - 1))
            {
            case 0: // flag/plaintext parameter
                ui->cbParamList->addItems(currentWRD.params);
                break;
            case 1: // raw number
                ui->cbParamList->setEditable(true);
                ui->cbParamList->addItem(QString::number(val));
                break;
            case 2: // dialogue string
                ui->cbParamList->addItems(currentWRD.strings);
                break;
            case 3: // label name
                ui->cbParamList->addItems(currentWRD.labels);
                break;
            }
        else
            ui->cbParamList->addItems(currentWRD.params);

        ui->cbParamList->setCurrentIndex(val);
    }
    else
    {
        ui->cbParamList->setEnabled(false);
        ui->spinboxParamNum->setMinimum(0);
        ui->spinboxParamNum->setMaximum(0);
        ui->spinboxParamNum->setValue(0);
        ui->spinboxParamNum->setEnabled(false);
        ui->cbParamList->clear();
        ui->labelParamTotal->setText("0");
    }
    ui->cbParamList->blockSignals(false);
    ui->spinboxParamNum->blockSignals(false);
}
