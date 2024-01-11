#pragma once
#ifndef OPERATINGSYSTEM
#define OPERATINGSYSTEM
#define _CRT_SECURE_NO_WARNINGS
#include "FileSnapshot.h"
#include "Log.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
using namespace std;

#define BLOCK_SIZE 2048
#define BLOCK_NUMBER 1024
#define INODE_NUMBER 1024

// 文件夹信息
#define ENTRY_NUMBER 64
#define MAX_FILENAME_LENGTH 28

// 用户文件信息
#define MAX_USERNAME_LENGTH 28
#define MAX_PASSWORD_LENGTH 30
#define MAX_USER_NUMBER 64

// 组文件信息
#define MAX_GROUPNAME_LENGTH 20
#define MAX_USER_NUMBER_IN_GROUP 54
#define MAX_GROUP_NUMBER 16

// 一级索引块中有512个
#define INDEX_SIZE 512


struct Inode;


struct DirectoryBlock
{
    char fileName[ENTRY_NUMBER][MAX_FILENAME_LENGTH];
    int inodeID[ENTRY_NUMBER];
};

struct FileBlock;

// 结构体 IndexBlock，表示一级索引块
struct IndexBlock
{
    int blockPointers[INDEX_SIZE];
};

// 用户
struct User // 32 bytes
{
    char userName[MAX_USERNAME_LENGTH]; // 用户名
    short userID; // 用户ID （感觉可以直接用在数组中的位置决定用户ID）
    short groupID; // 用户组ID (管理员(0) or 教师(1) or 学生(2))
};

struct UserBlock // 4096 bytes
{
    struct User users[MAX_USER_NUMBER];
};

struct Password // 32 bytes
{
    short userID; // 用户ID
    char password[MAX_PASSWORD_LENGTH]; // 密码
};

struct PasswordBlock // 4096 bytes
{
    Password passwords[MAX_USER_NUMBER];
};

// 组
struct Group // 128 bytes
{
    char groupName[MAX_GROUPNAME_LENGTH];
    short groupID; // 同用户ID
    short userID[MAX_USER_NUMBER_IN_GROUP];
};

struct GroupBlock // 4096 bytes
{
    struct Group groups[MAX_GROUP_NUMBER];
};

class OperatingSystem {
private:
    struct Inode inodeMem[INODE_NUMBER];
    struct FileBlock blockMem[BLOCK_NUMBER];
    char blockBitmap[BLOCK_NUMBER / 8];

    struct UserBlock* userBlock;
    int userBlockID = 1;
    int userInodeID = 1;
    

    struct GroupBlock* groupBlock;
    int groupBlockID = 2;
    int groupInodeID = 2;

    struct PasswordBlock* passwordBlock;
    int passwordBlockID = 3;
    int passwordInodeID = 3;

    void printBinary(char ch);
    
    FileSnapshot* snapshot = nullptr;
    Log* systemLog = nullptr;

public:
    // 互斥锁
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_reader;
    int readerNum = 0;

    // 构造函数，将系统文件读入到内存中
    OperatingSystem();
    OperatingSystem(struct Inode inodeMem[INODE_NUMBER], struct FileBlock blockMem[BLOCK_NUMBER], char blockBitmap[BLOCK_NUMBER / 8]);
    ~OperatingSystem();
    Inode getinode();
    GroupBlock getblock();
    char blockbit();

    // 登录系统，成功返回true，失败返回false。
    bool loginSystems(const string userName, const string password);

    // execUser就是执行的用户，要判断这个用户有没有权限
    // 用户
    // 创建用户，失败返回-1。
    int createUser(const int execUserID, const string userName, const string password, const int groupID);
    
    // 根据用户名删除用户
    bool deleteUser(const int execUserID, const int userID);
    
    bool listuser(const int execUserID);
    // 根据用户名找用户ID，没找到返回-1。
    int getUserID(const int execUserID, const string userName);
    
    // 根据用户ID找用户名，没找到返回空串。
    string getUserName(const int execUserID, const int userID);
    
    // 根据用户ID找用户主要组，返回组ID
    int getUserMainGroup(const int execUserID, const int userID);

    // 修改用户名
    bool modifyUserName(const int execUserID, const int userID, const string userName);

    // 修改用户密码
    bool modifyUserPassword(const int execUserID, const int userID, const string password);

    // 修改用户组
    bool modifyUserGroup(const int execUserID, const int userID, const int groupID); 
    

    // 组
    // 创建组，失败返回-1。
    bool listgroup(const int execUserID);
    int createGroup(const int execUserID, const string groupName);
    
    // 删除组，失败返回false。
    bool deleteGroup(const int execUserID, const int groupID);
    
    // 根据组名找组ID，没找到返回-1。
    int getGroupID(const int execUserID, const string groupName);
    
    // 根据组ID找组名，没找到返回空串。
    string getGroupName(const int execUserID, const int gourpID);
    
    // 将用户添加到指定组，失败返回false
    bool addUserToGroup(const int execUserID, const int userID, const int groupID);
    
    // 将用户从指定组中删除，失败返回false
    bool deleteUserFromGroup(const int execUserID, const int userID, const int groupID);

    // 查询groupID里是否存在userID这一用户
    bool checkUserFromGroup(const int execUserID, const int userID, const int groupID);
    
    // 文件管理
    // 在path下创建文件夹
    bool createDirectory(const int execUserID, string path, const int userID, const int groupID, const char ownerPermission, const char groupPermission, const char otherPermission);

    // 在path下删除文件夹
    bool deleteDirectory(const int execUserID, string path);
    void deleteAllFileInDirectory(struct DirectoryBlock* block);
    
    // 列出path中的所有文件
    // DirectoryBlock listFiles(const int execUserID, string path);
    vector<DirectoryBlock>listFiles(const int execUserID, string path);

    void listBlock();
    int findBlock();
    int findInode();
    
    // 在path下创建文件
    bool createFile(const int execUserID, string path, const int userID, const int groupID, const char ownerPermission, const char groupPermission, const char otherPermission);

    // 在path下删除文件
    bool deleteFile(const int execUserID, string path);
    
    // 在path下读文件
    // FileBlock readFile(const int execUserID, string path);
    vector<FileBlock> readFile(const int execUserID, string path);
    
    // 在path下写文件
    bool writeFile(const int execUserID, string path, string content);

    bool writeFile(const int execUserID, string path, FileBlock newblock);

    // 判定path是否存在，如果不存在返回-1，是文件夹返回0，文件返回1。
    int checkPath(const int execUserID, string path);
    
    // 权限管理
    // 修改path下的文件或文件夹的permission
    bool modifyPermission(const int execUserID, string path, const char ownerPermission, const char groupPermission, const char otherPermission);

    // 修改path下的文件或文件夹的所有者
    bool modifyOwner(const int execUserID, string path, const int userID);
    
    // 修改path下的文件或文件夹的所属组
    bool modifyGroup(const int execUserID, string path, const int groupID);
    // 获取path下的文件对应的inode
    int getReadInodeID(const int execUserID,string path);
    int getWriteInodeID(const int execUserID, string path);
    // 获取path下的文件或文件夹的所有者
    int getOwner(const int execUserID, string path);
    
    // 获取path下的文件或文件夹的所属组    
    int getGroup(const int execUserID, string path);
    
    // 判断execUser有没有读权限
    bool checkReadPermission(const int execUserID, const int inodeID);
    
    // 判断execUser有没有写权限
    bool checkWritePermission(const int execUserID, const int inodeID);

    // 判断execUser有没有执行权限（这个好像没什么用）
    bool checkExecutePermission(const int execUserID, const int inodeID);
    
    // 系统
    // 系统写入
    bool writeSystem();

    // 系统备份
    bool backupSystem(const int execUserID, const string version);

    // 系统恢复
    bool recoverySystem(const int execUserID, const string version);

    // 文件快照
    // 初始化文件快照
    bool initializeSnapshot();
    
    void pushBlockAndInode(int blockID, int inodeID);
    void pushTag(string command);

    bool revoke();
    
    bool recover();
    void printSnapshot();
    void printLog();
    string checkLog();
};
#endif //