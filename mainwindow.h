#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileSystemModel>
#include <QMainWindow>
#include <QListWidget>
#include <QTreeView>
#include "common.h"
#include "scripteditor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    void createDockWindows();
    void loadSPCFile(QString path);
    void saveSPCFile(QString path = currentSPC.filename);
    void loadScriptData(QString name);
    void saveScriptData(QString name = currentWRD.filename);
    void loadCmdInfo(const int index);
    void saveCmdInfo(const int index);

    Ui::MainWindow *ui;
    QTreeView *fileBrowser;
    QFileSystemModel *fileBrowserModel;
    QListWidget *scriptBrowser;
    ScriptEditor *scriptEditor;
    QFrame *previewer;


private slots:
    void on_FileBrowser_activated(const QModelIndex &index);
    void on_ScriptBrowser_activated(const QModelIndex &index);
    void on_codeList_activated(const QModelIndex &index);
};

#endif // MAINWINDOW_H
