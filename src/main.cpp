#include <iostream>
#include <string>
#include <fstream>

#include "OperatoreBinario.hpp"
#include "Sequente.hpp"
#include "Shell.hpp"
#include "qt.hpp"

using namespace std;

int main(int argc, char ** argv){
	QApplication a(argc,argv);
	TerminalGUI w;
	w.show();
	return a.exec();
}
