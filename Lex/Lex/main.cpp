#include <iostream>
#include <string>
#include <fstream>
#include "Lex.h"
using namespace std;

int main() {
	cout << "Input the lex file name ending up with \".l\"\n>>> ";
	string infile;
	cin >> infile;
	// filename ends up with ".l"
	if (infile.size() > 2 && infile[infile.size() - 2] == '.' && infile[infile.size() - 1] == 'l') {
		ifstream ifs(infile.c_str());
		if (!ifs) {
			cout << "Can not read the lex file!" << endl;
		}
		else {
			ofstream ofs("lex.yy.c");
			if (!ofs) {
				cout << "Can not write the C file!" << endl;
			}
			else {
				int errline = ParseLexFile(ifs, ofs);
				if (errline == 0) {
					cout << "Output the C file lex.yy.c." << endl;
				}
				else if (errline == -1) {
					cout << "Regular expression lexical error." << endl;
				}
				else {
					cout << "Lex file error in line " << errline << " ." << endl;
				}
				ofs.close();
			}
			ifs.close();
		}
	}
	else {
		cout << "Not a lex file!" << endl;
	}
	return 0;
}
