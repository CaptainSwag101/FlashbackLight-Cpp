#include "codeeditor.h"
#include "ui_codeeditor.h"

CodeEditor::CodeEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CodeEditor)
{
    ui->setupUi(this);
}

CodeEditor::~CodeEditor()
{
    delete ui;
}
