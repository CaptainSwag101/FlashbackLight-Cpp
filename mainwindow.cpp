#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    loadConfig();

    // Populate the opcode combobox with all known opcodes.
    ui->cbOpcode->blockSignals(true);
    ui->cbOpcode->clear();
    for (const QString &string : WRDCmd::NAME_LIST)
    {
        ui->cbOpcode->addItem(string);
    }
    ui->cbOpcode->installEventFilter(this);
    ui->cbOpcode->setEnabled(false);
    ui->cbOpcode->blockSignals(false);

    ui->parameterScrollArea->widget()->setLayout(new QVBoxLayout());
    // This is required to make the scroll area resize properly when adding/removing widgets:
    ui->parameterScrollArea->widget()->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);

    createDockWindows();
}

MainWindow::~MainWindow()
{
    delete ui;
}


/// If the config file exists, load our settings from it.
/// Otherwise prompt the user for their unpacked game data location (CPK files need to already be unpacked).
void MainWindow::loadConfig()
{
    QFile configFile(QDir::currentPath() + "/V3_Editor.cfg");

    if (configFile.exists() && (dataPath.isEmpty() || !QDir(dataPath).exists()))
    {
        configFile.open(QFile::ReadOnly);
        QTextStream stream(&configFile);

        while (!stream.atEnd())
        {
            QString line = stream.readLine();
            QString key = line.section(" = ", 0, 0).toLower();
            QString value = line.section(" = ", 1);    // Since paths are case-sensitive on some OSes, don't convert this to lowercase

            if (key == "datapath")
                dataPath = value;
        }

        configFile.close();
    }

    if (!configFile.exists() || dataPath.isEmpty() || !QDir(dataPath).exists())
    {
        QMessageBox::information(nullptr, "V3 Editor", "The config file is either missing, or does not contain a valid path to your Danganronpa V3 \"data/win\" directory. "
                                                    "You will now be prompted to specify its location.");

        QString path = QFileDialog::getExistingDirectory(this, "Location of Danganronpa V3 \"data/win\" directory");
        if (path.isEmpty() || !QDir(dataPath).exists())
        {
            QMessageBox::warning(nullptr, "Invalid game data path", "A valid game data path was not provided. The program will not work correctly without a valid data directory path.");
            return;
        }
        dataPath = path;

        // Create the config file and write the data path to it.
        //if (configFile.exists()) configFile.remove();
        configFile.open(QFile::WriteOnly);
        QTextStream stream(&configFile);

        stream << "datapath = " + QDir::toNativeSeparators(dataPath);

        configFile.close();
    }
}

void MainWindow::createDockWindows()
{
    fileBrowserModel = new QFileSystemModel(ui->fileBrowser);
    fileBrowserModel->setRootPath(dataPath + "/wrd_script");
    fileBrowserModel->setNameFilters(QStringList() << "*.spc" << "*.SPC");
    ui->fileBrowser->setModel(fileBrowserModel);
    ui->fileBrowser->setRootIndex(fileBrowserModel->index(dataPath + "/wrd_script"));
    //ui->fileBrowser->setColumnWidth(0, 250);
    ui->fileBrowser->setHeaderHidden(true);
    ui->fileBrowser->hideColumn(1);
    ui->fileBrowser->hideColumn(2);
    ui->fileBrowser->hideColumn(3);
    connect(ui->fileBrowser, &QTreeView::activated, this, &MainWindow::on_fileBrowser_activated);
    //viewMenu->addAction(dock->toggleViewAction());


    ui->scriptBrowser->addItem("(No SPC file currently loaded)");
    connect(ui->scriptBrowser, &QListWidget::activated, this, &MainWindow::on_scriptBrowser_activated);
    //viewMenu->addAction(dock->toggleViewAction());


    // TODO: Set up the Previewer widget here once we implement it.
}


void MainWindow::loadSPCFile(QString path)
{
    QFile file(path);
    file.open(QFile::ReadOnly);
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    currentSPC = SPC::fromBytes(stream);
    currentSPCFilename = file.fileName();

    // Populate the script browser list
    ui->scriptBrowser->clear();
    for (const SPCEntry &entry : currentSPC.files)
    {
        if (entry.filename.endsWith(".wrd", Qt::CaseInsensitive))
        {
            ui->scriptBrowser->addItem(entry.filename);
        }
    }
}

void MainWindow::saveSPCFile(QString path)
{

}

void MainWindow::loadScriptData(QString name)
{
    for (SPCEntry &entry : currentSPC.files)
    {
        if (entry.filename == name)
        {
            currentWRD = WRD::fromBytes(entry.data);
            currentWRDFilename = name;
            break;
        }
    }

    initializeCodeList();
}

void MainWindow::saveScriptData(QString name)
{

}


void MainWindow::initializeCodeList()
{
    ui->codeList->clear();
    for (int row = 0; row < currentWRD.code.count(); ++row)
    {
        ui->codeList->addItem(QString());
        updateCodeListEntry(row);
    }

    ui->cbOpcode->setEnabled(false);
    clearParameterArea();
}

void MainWindow::updateCodeListEntry(int index)
{
    const WRDCmd cmd = currentWRD.code.at(index);
    const int argCount = cmd.argData.count();
    const int argTypeCount = cmd.getArgTypes().count();

    ui->codeList->item(index)->setTextColor(QListWidgetItem().textColor());

    QString str = cmd.getName() + " (";
    for (int i = 0; i < argCount; ++i)
    {
        const ushort val = cmd.argData.at(i);

        if (argTypeCount > 0 && (cmd.argData.size() == cmd.getArgTypes().size() || cmd.isVarLength()))
        {
            if (cmd.getArgTypes().at(i % argTypeCount) == 0 && val < currentWRD.params.size())       // flag/plaintext parameter
                str += currentWRD.params.at(val);
            else if (cmd.getArgTypes().at(i % argTypeCount) == 1)                                    // raw number
                str += QString::number(val);
            else if (cmd.getArgTypes().at(i % argTypeCount) == 2 && val < currentWRD.strings.size()) // dialogue string
                str += currentWRD.strings.at(val);
            else if (cmd.getArgTypes().at(i % argTypeCount) == 3 && val < currentWRD.labels.size())  // label name
                str += currentWRD.labels.at(val);
            else
            {
                str += "!" + QString::number(val) + "!";
                ui->codeList->item(index)->setTextColor(Qt::red);
            }
        }
        else
        {
            str += "!" + QString::number(val) + "!";
            ui->codeList->item(index)->setTextColor(Qt::red);
        }

        if ((i + 1) < cmd.argData.count())
        {
            // The IFF, WAK, and IFW opcodes look better without commas, trust me.
            if (cmd.opcode != 0x01 && cmd.opcode != 0x02 && cmd.opcode != 0x03)
                str += ",";

            str += " ";
        }
    }
    str += ")";

    ui->codeList->item(index)->setText(str);
}

void MainWindow::initializeCodeEditor(int index)
{
    const WRDCmd cmd = currentWRD.code.at(index);

    // Set the opcode comboBox value to the current command's opcode
    ui->cbOpcode->setEnabled(true);
    ui->cbOpcode->blockSignals(true);
    ui->cbOpcode->setCurrentIndex(cmd.opcode);
    ui->labelOpcodeIndex->setText("Opcode ID: 0x" + QString::number(cmd.opcode, 16).toUpper().rightJustified(2, '0'));
    ui->cbOpcode->blockSignals(false);


    // Populate the parameter area with the appropriate widget
    // for each argument.
    //
    // TODO: Use a model or something to provide the data, it's definitely
    // wasteful to copy the same data lists a million times to each widget!
    //
    clearParameterArea();
    for (int a = 0; a < cmd.argData.count(); ++a)
    {
        const ushort val = cmd.argData.at(a);

        if (a >= cmd.getArgTypes().count() || cmd.getArgTypes().at(a) == 0)   // plaintext flag parameter
        {
            QComboBox *cb = new QComboBox(ui->parameterScrollArea->widget());
            cb->addItems(currentWRD.params);
            cb->setCurrentIndex(val);
            cb->setFocusPolicy(Qt::ClickFocus);
            cb->installEventFilter(this);
            ui->parameterScrollArea->widget()->layout()->addWidget(cb);
            connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_parameterWidget_changed);
        }
        else if (cmd.getArgTypes().at(a) == 1)                           // raw number
        {
            QSpinBox *sb = new QSpinBox(ui->parameterScrollArea->widget());
            sb->setValue(val);
            sb->setFocusPolicy(Qt::ClickFocus);
            sb->installEventFilter(this);
            ui->parameterScrollArea->widget()->layout()->addWidget(sb);
            connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::on_parameterWidget_changed);
        }
        else if (cmd.getArgTypes().at(a) == 2)                           // dialogue string
        {
            QComboBox *cb = new QComboBox(ui->parameterScrollArea->widget());
            cb->addItems(currentWRD.strings);
            cb->setCurrentIndex(val);
            cb->setFocusPolicy(Qt::ClickFocus);
            cb->installEventFilter(this);
            ui->parameterScrollArea->widget()->layout()->addWidget(cb);
            connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_parameterWidget_changed);
        }
        else if (cmd.getArgTypes().at(a) == 3)                           // label name
        {
            QComboBox *cb = new QComboBox(ui->parameterScrollArea->widget());
            cb->addItems(currentWRD.labels);
            cb->setCurrentIndex(val);
            cb->setFocusPolicy(Qt::ClickFocus);
            cb->installEventFilter(this);
            ui->parameterScrollArea->widget()->layout()->addWidget(cb);
            connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_parameterWidget_changed);
        }
    }
}

/// Delete all existing widgets from the parameter area.
/// (Here's hoping this won't be buggy or problematic as fuck).
void MainWindow::clearParameterArea()
{
    for (QWidget *child : ui->parameterScrollArea->widget()->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
    {
        child->disconnect();
        delete child;
    }
}

int MainWindow::promptUnsavedChanges()
{
    // If we don't have any unsaved changes, we can just exit without prompting the user.
    if (!unsavedChanges) return QMessageBox::Yes;

    QMessageBox prompt;
    prompt.setText("The current document has been modified.");
    prompt.setInformativeText("Do you want to save your changes?");
    prompt.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    prompt.setDefaultButton(QMessageBox::Save);
    int result = prompt.exec();

    if (result == QMessageBox::Save)
    {
        // TODO: Save the current file
    }

    return result;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    int result = promptUnsavedChanges();
    if (result == QMessageBox::Save || result == QMessageBox::Discard)
        event->accept();
    else if (result == QMessageBox::Cancel)
        event->ignore();
}

void MainWindow::on_codeList_activated(const QModelIndex &index)
{
    initializeCodeEditor(index.row());
}

void MainWindow::on_fileBrowser_activated(const QModelIndex &index)
{
    if (fileBrowserModel->isDir(index) || !fileBrowserModel->fileName(index).endsWith(".spc", Qt::CaseInsensitive))
        return;

    loadSPCFile(fileBrowserModel->filePath(index));
}

void MainWindow::on_scriptBrowser_activated(const QModelIndex &index)
{
    loadScriptData(index.data().toString());
}

/// We use this to prevent the command parameter widgets from updating when
/// you scroll over them with the mouse wheel.
bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Wheel)
        if (qobject_cast<QComboBox *>(obj) || qobject_cast<QSpinBox *>(obj))
            return true;

    return false;
}

void MainWindow::on_parameterWidget_changed(int value)
{
    if (QObject::sender()->isWidgetType())
    {
        const int widgetIndex = ui->parameterScrollArea->widget()->layout()->indexOf((QWidget *)QObject::sender());
        currentWRD.code[ui->codeList->currentRow()].argData[widgetIndex] = (ushort)value;
        updateCodeListEntry(ui->codeList->currentRow());
    }
    return;
}

void MainWindow::on_cbOpcode_currentIndexChanged(int index)
{
    currentWRD.code[ui->codeList->currentRow()].opcode = index;
    ui->labelOpcodeIndex->setText("Opcode ID: 0x" + QString::number(index, 16).toUpper().rightJustified(2, '0'));
    updateCodeListEntry(ui->codeList->currentRow());
    initializeCodeEditor(ui->codeList->currentRow());
}
