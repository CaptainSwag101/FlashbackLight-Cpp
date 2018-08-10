#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>

namespace Ui {
class ScriptEditor;
}

class ScriptEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);
    ~ScriptEditor();
    void populateCodeList();


public slots:
    void on_codeList_activated(const QModelIndex &index);
    void on_spinboxParamNum_valueChanged(int i);


private:
    Ui::ScriptEditor *ui;
};

#endif // SCRIPTEDITOR_H
