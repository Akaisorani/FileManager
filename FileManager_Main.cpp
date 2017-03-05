#include<iostream>
#include<fstream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "FileManager.h"
#include "FileManager.cpp"

using namespace std;

int main(){
	
	FileManager fm;
	//fm.createFileSet("E:/file/CarSet.dat",1000);
	fm.openFileSet("./CarSet.dat");
	char str[100]="pic (1).jpg";
	char nam[100]="pic (1).jpg";
	for(int i=0;i<=9;i++){
		nam[5]='0'+i;
		str[5]='A'+i;
		//fm.addFileToFileSet(str);
		fm.fetchFileFromFileSet(nam,str);
	}
	fm.closeFileSet();
	
	
	return 0;

}