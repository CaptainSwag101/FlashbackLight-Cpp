#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileSystemModel>
#include <QMainWindow>
#include <QListWidget>
#include <QTreeView>
#include "formats/spc.h"
#include "formats/wrd.h"

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
    void initializeCodeList();
    void updateCodeListEntry(int index);
    void initializeCodeEditor(int index);
    void clearParameterArea();

    static SPC currentSPC;
    static WRD currentWRD;
    static QString dataPath;

    Ui::MainWindow *ui;
    QFileSystemModel *fileBrowserModel;


private slots:
    void on_codeList_activated(const QModelIndex &index);
    void on_fileBrowser_activated(const QModelIndex &index);
    void on_scriptBrowser_activated(const QModelIndex &index);
    bool eventFilter(QObject *obj, QEvent *e);
    void on_parameterWidget_changed(int value);
    void on_cbOpcode_currentIndexChanged(int index);
};

#endif // MAINWINDOW_H
