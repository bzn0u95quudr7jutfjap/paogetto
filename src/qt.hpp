#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QKeyEvent>
#include <QTextEdit>
#include <QPlainTextEdit>

#include "Shell.hpp"

class SysinTextEdit : public QTextEdit {
	QPlainTextEdit * out;
	public :
		SysinTextEdit(QPlainTextEdit * p);
		void keyPressEvent(QKeyEvent * p) override;
};

class TerminalGUI : public QMainWindow {
	public:
		QPlainTextEdit * sysout;
		QTextEdit * sysin;
		TerminalGUI();
};
