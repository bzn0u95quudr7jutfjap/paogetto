#include <QMainWindow>
#include <QWidget>
#include <QKeyEvent>
#include <QString>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextCursor>

using namespace std;

#include "Shell.hpp"
#include "qt.hpp"

SysinTextEdit::SysinTextEdit(QPlainTextEdit * p): out(p) {
	setMinimumHeight(70);
	setMaximumHeight(70);
};

void SysinTextEdit::keyPressEvent(QKeyEvent * p){
       if(p->key() == Qt::Key_Return){
			out->moveCursor(QTextCursor::End);
			out->insertPlainText(">>> " + this->toPlainText() + "\n");
			string txt = Shell::eval(Shell::parse(this->toPlainText().toStdString()));
			if(txt.size()){
				out->insertPlainText(txt.c_str());
				out->insertPlainText("\n");
			}
			out->moveCursor(QTextCursor::End);
			out->ensureCursorVisible();
			this->setText("");
       }else{
	       QTextEdit::keyPressEvent(p);
       }
}


TerminalGUI::TerminalGUI(){
	QVBoxLayout * mainly = new QVBoxLayout();
	sysout = new QPlainTextEdit();
	sysout->setReadOnly(true);
	sysout->setLineWrapMode(QPlainTextEdit::LineWrapMode::WidgetWidth);
	sysout->setMaximumBlockCount(-1);
	sysin = new SysinTextEdit(sysout);
	sysout->setFocusPolicy(Qt::NoFocus);
	sysout->setPlainText("Digita \"help\" nella barra sotto per stampare il manuale\n");

	mainly->addWidget(sysout);
	mainly->addWidget(sysin);
	
	sysin->sizePolicy().setVerticalPolicy(QSizePolicy::Maximum);
	sysout->sizePolicy().setVerticalPolicy(QSizePolicy::MinimumExpanding);
        
	QWidget *cnt = new QWidget();
	cnt->setLayout(mainly);
	setCentralWidget(cnt);
}
