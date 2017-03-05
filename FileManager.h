#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include<iostream>
#include<stdio.h>
#include<fstream>
#include<string.h>

 using namespace std;

class FileTag
{
public:
	FileTag();
	~FileTag();
	
	char* getFileName();
	void setFileName(char* name);
	
	int getFileFlag();
	void setFileFlag(int newflag);
	
	int getFileSize();
	void setFileSize(int newFileSize);
	
	__int64 getFileOffset();
	void setFileOffset(__int64 newOffset);
	
	void writeTag(fstream &os);
	void loadTag(fstream &is);
	
private:
	char fileName[256];
	int fileFlag,fileSize;
	__int64 fileOffset;
};
class Header
{
public:
	Header();
	Header(int maxnumber);
	~Header();

	void setMaxFileNumber(int maxnumber);
	int getMaxFileNumber();
	
	void loadHeader(fstream &is);
	void writeHeader(fstream &os);
	
protected:
	char setMark[4];
	__int64 fileSize;
	int fileCnt;
	FileTag *fileTagList;	
	int maxFileNumber;
};

class FileSet:public Header
{
public:
	FileSet();
	FileSet(fstream &is,int maxnumber);
	//FileSet(fstream &is,int maxnumber,int d);
	~FileSet();
	
	bool addFile(fstream &is,char* filePath);
	bool deleteFile(char* fileName);
	int searchFile(char* fileName);
	bool fetchFile(fstream &os,char* fileName);
	void listAllFile();
private:
	fstream &fp;
};
class FileManager
{
public:
	FileManager();
	~FileManager();
	
	bool createFileSet(char* filePath, int maxFileNumberOfNewSet);
	bool openFileSet(char* filePath);
	bool addFileToFileSet(char* filePath);
	bool deleteFileFromFileSet(char* fileName);
	bool searchFileFromFileSet(char* fileName);
	bool fetchFileFromFileSet(char* fileName, char* newPathAndName);
	void listAllFile();
	bool closeFileSet();
	
private:
	FileSet *fileSet;
	fstream bigfp,smallfp;
};
#endif