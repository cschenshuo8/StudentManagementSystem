#pragma once
#ifndef FILESNAPSHOT
#define FILESNAPSHOT

#include <sys/stat.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

#define BLOCK_SIZE 2048

using namespace std;

struct Inode
{
    int inodeNumber;
    int blockID;
    int fileType;  // 0 represents directory, 1 represents normal file, 2 represents indirect block
    int userID; // 所属用户ID
    int groupID; // 所属组ID

    /* 访问权限
        0: 读权限
        1: 写权限
        2: 执行权限 
        ...
    */
    char ownerPermission;
    char groupPermission;
    char otherPermission;
};

struct FileBlock
{
    char content[BLOCK_SIZE];
};

struct BlockAndInode
{
    Inode inode;
    int inodeID;
    FileBlock block;
    int blockID;
    /*
        blockbit == 0 : Unused block.
        blockbit == 1 : Used block.
        blockbit == -1 : Flag for the end of atomic operation.
    */
    int blockbit;
};

class FileSnapshot
{
public:
    vector<BlockAndInode> snapshotStack;
    int nowPointer;
    int topPointer;
    int maxPointer;

public:
    FileSnapshot();
    ~FileSnapshot();
    bool readFromFile();
    bool writeToFile();
    
    bool init();

    bool pushcc(const BlockAndInode& blockAndInode);
    bool pop();

    bool revokePop();
};
#endif