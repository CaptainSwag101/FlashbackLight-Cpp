#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QWidget>

namespace Ui {
class CodeEditor;
}

class CodeEditor : public QWidget
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();

private:
    Ui::CodeEditor *ui;
};

#endif // CODEEDITOR_H
