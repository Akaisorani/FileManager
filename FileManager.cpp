#include<iostream>
#include<fstream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<iomanip>
#include "FileManager.h"
using namespace std;
//FileManager

FileManager::FileManager():fileSet(NULL){}
FileManager::~FileManager(){delete fileSet;fileSet=NULL;}

bool FileManager::createFileSet(char* filePath, int maxFileNumberOfNewSet)
{	
	if(bigfp.is_open()){printf("ERROR:Please close the last FileSet.\n");return false;}
	
	bigfp.open(filePath);
	if(bigfp.is_open()){printf("ERROR:File has already existed.\n");bigfp.close();return false;}
	
	bigfp.open(filePath,ios::out);bigfp.close();
	bigfp.open(filePath,ios::in|ios::out|ios::binary);
	if(!bigfp.is_open()){printf("ERROR:Create failed.\nMaybe you should use '/' instead of '\\'.\n");return false;}
	fileSet=new FileSet(bigfp,maxFileNumberOfNewSet);

	return true;
}
bool FileManager::openFileSet(char* filePath)
{	
	if(bigfp.is_open()){printf("ERROR:Please close the last FileSet.\n");return false;}
	bigfp.open(filePath,ios::in|ios::out|ios::binary);
	if(!bigfp.is_open()){printf("ERROR:File doesn't exist.\n");return false;}
	int maxFileNumberOfNewSet;
	bigfp.seekg(4);
	bigfp.read((char*)&maxFileNumberOfNewSet,sizeof(maxFileNumberOfNewSet));
	fileSet=new FileSet(bigfp,maxFileNumberOfNewSet);
	fileSet->loadHeader(bigfp);
	return true;
}
bool FileManager::addFileToFileSet(char* filePath)
{	
	if(fileSet==NULL)return false;
	if(smallfp.is_open())smallfp.close();
	smallfp.open(filePath,ios::in|ios::out|ios::binary);
	if(!smallfp.is_open())return false;
	bool sucess=fileSet->addFile(smallfp,filePath);
	smallfp.close();
	return sucess;
}
bool FileManager::deleteFileFromFileSet(char* fileName)
{
	if(fileSet==NULL)return false;
	return fileSet->deleteFile(fileName);
}
bool FileManager::searchFileFromFileSet(char* fileName)
{	
	if(fileSet==NULL)return false;
	if(fileSet->searchFile(fileName)>0)return true; else return false;
}
bool FileManager::fetchFileFromFileSet(char* fileName, char* newPathAndName)
{	
	if(fileSet==NULL)return false;
	smallfp.close();
	if(fileSet->searchFile(fileName)<=0)return false;
	smallfp.open(newPathAndName,ios::out|ios::binary);
	bool sucess=fileSet->fetchFile(smallfp,fileName);
	smallfp.close();
	return sucess;
}
void FileManager::listAllFile()
{
	if(fileSet==NULL)return;
	fileSet->listAllFile();
}
bool FileManager::closeFileSet()
{
	if(!bigfp.is_open())return false;
	fileSet->writeHeader(bigfp);
	bigfp.close();smallfp.close();
	delete fileSet;
	return true;
}

//FileTag========================================================
FileTag::FileTag():fileFlag(0),fileSize(0),fileOffset(0){
	memset(fileName,0,sizeof(fileName));
}
FileTag::~FileTag(){};

char* FileTag::getFileName(){return fileName;}
void FileTag::setFileName(char* filename){strcpy(fileName,filename);}

int FileTag::getFileFlag(){return fileFlag;}
void FileTag::setFileFlag(int newflag){fileFlag=newflag;}

int FileTag::getFileSize(){return fileSize;}
void FileTag::setFileSize(int newFileSize){fileSize=newFileSize;}

__int64 FileTag::getFileOffset(){return fileOffset;}
void FileTag::setFileOffset(__int64 newOffset){fileOffset=newOffset;}

void FileTag::writeTag(fstream &os)
{
	//二进制编码
	os.write(fileName,256);
	os.write((char*)&fileFlag,sizeof(fileFlag));
	os.write((char*)&fileSize,sizeof(fileSize));
	os.write((char*)&fileOffset,sizeof(fileOffset));
}
void FileTag::loadTag(fstream &is)
{
	//二进制解析
	is.read(fileName,256);
	is.read((char*)&fileFlag,sizeof(fileFlag));
	is.read((char*)&fileSize,sizeof(fileSize));
	is.read((char*)&fileOffset,sizeof(fileOffset));
}
//Header===========================================================
Header::Header(){}
Header::Header(int maxnumber)
{
	strcpy(setMark,"HIT-");
	fileSize=0;
	fileCnt=0;
	fileTagList=new FileTag[maxnumber+1];
	maxFileNumber=maxnumber;
	fileTagList[0].setFileSize(20+272*maxFileNumber);
}
Header::~Header()
{
	delete fileTagList;
}

void Header::setMaxFileNumber(int maxnumber)
{
	maxFileNumber=maxnumber;
}
int Header::getMaxFileNumber()
{
	return maxFileNumber;
}
void Header::loadHeader(fstream &is)
{
	is.seekg(0);
	is.read(setMark,4);
	is.read((char*)&maxFileNumber,sizeof(maxFileNumber));
	is.read((char*)&fileCnt,sizeof(fileCnt));
	is.read((char*)&fileSize,sizeof(fileSize));
	int i;
	for(i=1;i<=maxFileNumber;i++){
		fileTagList[i].loadTag(is);
	}	
}
void Header::writeHeader(fstream &os)
{
	os.seekp(0);
	os.write(setMark,4);
	os.write((char*)&maxFileNumber,sizeof(maxFileNumber));
	os.write((char*)&fileCnt,sizeof(fileCnt));
	os.write((char*)&fileSize,sizeof(fileSize));
	int i;
	for(i=1;i<=maxFileNumber;i++){
		fileTagList[i].writeTag(os);
	}
}

//FileSet===========================================================
FileSet::FileSet():fp(*(new fstream)){}
FileSet::FileSet(fstream &is,int maxnumber):Header(maxnumber),fp(is)
{}

FileSet::~FileSet(){}
bool FileSet::addFile(fstream &is,char* filePath)
{	
	char* pBuffer=NULL;
	is.seekg(0,ios::end);
	int len=is.tellg();
	pBuffer=(char*)malloc(len+1024*1024);
	is.seekg(0,ios::beg);
	is.read(pBuffer,len);

	__int64 newoffset;
	int i;
	for(i=1;i<=maxFileNumber;i++){
		if(fileTagList[i].getFileFlag()==0){
			break;
		}else if(fileTagList[i].getFileFlag()==2){
			if(fileTagList[i].getFileSize()>=len)
				break;
		}
	}	
	if(i>maxFileNumber)return false;
	newoffset=fileTagList[i-1].getFileOffset()+fileTagList[i-1].getFileSize();
	
	int pathlen=strlen(filePath);
	int j;
	for(j=pathlen-1;j>=0;j--)if(filePath[j]=='/'||filePath[j]=='\\')break;
	char filename[256]="";
	memcpy(filename,filePath+j+1,pathlen-j-1);
	
	fileTagList[i].setFileName(filename);
	fileTagList[i].setFileFlag(1);
	fileTagList[i].setFileSize(len);
	fileTagList[i].setFileOffset(newoffset);
	fp.seekp(newoffset);
	fp.write(pBuffer,len);
	if(fp.tellp()>fileSize)fileSize=fp.tellp();
	fileCnt++;
	free(pBuffer);
	return true;
}
bool FileSet::deleteFile(char* fileName)
{
	int i;
	for(i=1;i<=maxFileNumber;i++){
		if(!strcmp(fileName,fileTagList[i].getFileName())){
			break;
		}
	}
	if(i>maxFileNumber)return false;
	fileTagList[i].setFileFlag(2);
	fileCnt--;
	return true;
}
int FileSet::searchFile(char *fileName)
{
	int i;
	for(i=1;i<=maxFileNumber;i++){
		if(!strcmp(fileName,fileTagList[i].getFileName())&&fileTagList[i].getFileFlag()==1){
			break;
		}
	}
	if(i>maxFileNumber)return -1; else return i;	
}
bool FileSet::fetchFile(fstream &os,char* fileName)
{	
	
	int i=searchFile(fileName);
	if(i<=0)return false;
	int len=fileTagList[i].getFileSize();
	char* pBuffer;
	pBuffer=(char*)malloc(len+1024*1024);
	fp.seekg(fileTagList[i].getFileOffset());
	fp.read(pBuffer,len);
	os.seekp(0);
	os.write(pBuffer,len);
	free(pBuffer);
	return true;
}
void FileSet::listAllFile()
{
	cout<<"This fileset has "<<fileCnt<<" files."<<endl<<endl;
	for(int i=1;i<=maxFileNumber;i++){
		if(fileTagList[i].getFileFlag()==1){
			cout<<setw(30)<<setiosflags(ios::left)<<fileTagList[i].getFileName();
			cout<<fileTagList[i].getFileSize()<<" bytes"<<endl;
		}
	}
}

