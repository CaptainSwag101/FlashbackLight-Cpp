#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"

QString dataPath = "D:\\Games\\SteamLibrary\\steamapps\\common\\Danganronpa V3 Killing Harmony\\data\\win\\wrd_script";
SPC currentSPC;
WRD currentWRD;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scriptEditor = new ScriptEditor(this);
    setCentralWidget(scriptEditor);


    ui->centralWidget->setLayout(new QHBoxLayout());

    createDockWindows();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createDockWindows()
{
    QDockWidget *dock = new QDockWidget(tr("File Browser"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    fileBrowser = new QTreeView(dock);
    dock->setWidget(fileBrowser);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    fileBrowserModel = new QFileSystemModel(fileBrowser);
    fileBrowserModel->setRootPath(dataPath);
    fileBrowser->setModel(fileBrowserModel);
    fileBrowser->setRootIndex(fileBrowserModel->index(dataPath));
    connect(fileBrowser, &QTreeView::activated, this, &MainWindow::on_FileBrowser_activated);


    dock = new QDockWidget(tr("Script Browser"), this);
    scriptBrowser = new QListWidget(dock);
    dock->setWidget(scriptBrowser);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    scriptBrowser->addItem("(No SPC file currently loaded)");
    connect(scriptBrowser, &QListWidget::activated, this, &MainWindow::on_ScriptBrowser_activated);


    dock = new QDockWidget(tr("Preview"), this);
    previewer = new QFrame(dock);
    dock->setWidget(previewer);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    //connect(scriptEditor, &ScriptEditor::codeList_activated, this, &MainWindow::on_codeList_activated);

    /*
    customerList = new QListWidget(dock);
    customerList->addItems(QStringList()
            << "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton"
            << "Jane Doe, Memorabilia, 23 Watersedge, Beaton"
            << "Tammy Shea, Tiblanka, 38 Sea Views, Carlton"
            << "Tim Sheen, Caraba Gifts, 48 Ocean Way, Deal"
            << "Sol Harvey, Chicos Coffee, 53 New Springs, Eccleston"
            << "Sally Hobart, Tiroli Tea, 67 Long River, Fedula");
    dock->setWidget(customerList);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Paragraphs"), this);
    paragraphsList = new QListWidget(dock);
    paragraphsList->addItems(QStringList()
            << "Thank you for your payment which we have received today."
            << "Your order has been dispatched and should be with you "
               "within 28 days."
            << "We have dispatched those items that were in stock. The "
               "rest of your order will be dispatched once all the "
               "remaining items have arrived at our warehouse. No "
               "additional shipping charges will be made."
            << "You made a small overpayment (less than $5) which we "
               "will keep on account for you, or return at your request."
            << "You made a small underpayment (less than $1), but we have "
               "sent your order anyway. We'll add this underpayment to "
               "your next bill."
            << "Unfortunately you did not send enough money. Please remit "
               "an additional $. Your order will be dispatched as soon as "
               "the complete amount has been received."
            << "You made an overpayment (more than $5). Do you wish to "
               "buy more items, or should we return the excess to you?");
    dock->setWidget(paragraphsList);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    connect(customerList, &QListWidget::currentTextChanged,
            this, &MainWindow::insertCustomer);
    connect(paragraphsList, &QListWidget::currentTextChanged,
            this, &MainWindow::addParagraph);
    */
}

void MainWindow::loadSPCFile(QString path)
{
    QFile file(path);
    file.open(QFile::ReadOnly);
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    currentSPC = SPC::fromBytes(stream);
    currentSPC.filename = file.fileName();

    // Populate the script browser list
    QStringList scriptFiles;
    for (const SPCEntry &entry : currentSPC.files)
    {
        if (entry.filename.endsWith(".wrd", Qt::CaseInsensitive))
        {
            scriptFiles << entry.filename;
        }
    }
    scriptBrowser->clear();
    scriptBrowser->addItems(scriptFiles);
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
            currentWRD.filename = name;
            break;
        }
    }
    scriptEditor->populateCodeList();
}

void MainWindow::saveScriptData(QString name)
{

}

void MainWindow::loadCmdInfo(const int index)
{
    // Populate the opcode ComboBox with all known opcodes
    QStringList knownCmdNames;
    for (const WRDCmd &known : KNOWN_CMDS)
    {

    }

}

void MainWindow::saveCmdInfo(const int index)
{

}

void MainWindow::on_FileBrowser_activated(const QModelIndex &index)
{
    if (fileBrowserModel->isDir(index) || !fileBrowserModel->fileName(index).endsWith(".spc", Qt::CaseInsensitive))
        return;

    loadSPCFile(fileBrowserModel->filePath(index));
}

void MainWindow::on_ScriptBrowser_activated(const QModelIndex &index)
{
    loadScriptData(index.data().toString());
}

void MainWindow::on_codeList_activated(const QModelIndex &index)
{
    loadCmdInfo(index.row());
}
