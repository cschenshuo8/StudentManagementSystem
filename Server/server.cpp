#include "TcpServer.h"
#include "OperatingSystem.h"
#include "Log.h"
#include <stdio.h>  
#include <stdlib.h>  
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <string.h>  
#include <unistd.h> 
#include <vector>
using namespace std;

struct SockInfo
{
    TcpServer* s;
    TcpSocket* tcp;
    struct sockaddr_in addr;
};

struct SockInfoAndOS
{
    SockInfo* sockinfo;
    OperatingSystem* os;
    Log* serverLog;
};

// File transfer
string recvFile(struct SockInfo*& pinfo)
{
    string ret = "";
    while (1)
    {
        string msg = pinfo->tcp->recvMsg();
        cout << msg << endl;
        if (!msg.empty())
        {
            ret += msg;
        }
        else
        {
            break;
        }
    }
    return ret;
}

// Mutex
void mutexInit(OperatingSystem* os)
{
    pthread_mutex_init(&os->mutex, NULL);
    pthread_mutex_init(&os->mutex_reader, NULL);
}

void mutexDestroy(OperatingSystem* os)
{
    pthread_mutex_destroy(&os->mutex);
    pthread_mutex_destroy(&os->mutex_reader);
}

void downReader(OperatingSystem* os)
{
    pthread_mutex_lock(&os->mutex_reader);
    os->readerNum++;
    if (os->readerNum == 1) pthread_mutex_lock(&os->mutex);
    pthread_mutex_unlock(&os->mutex_reader);
}

void upReader(OperatingSystem* os)
{
    pthread_mutex_lock(&os->mutex_reader);
    os->readerNum--;
    if (os->readerNum == 0) pthread_mutex_unlock(&os->mutex);
    pthread_mutex_unlock(&os->mutex_reader);
}

void downWriter(OperatingSystem* os)
{
    pthread_mutex_lock(&os->mutex);
}

void upWriter(OperatingSystem* os)
{
    pthread_mutex_unlock(&os->mutex);
}

// All the functions
bool login(struct SockInfo*& pinfo, int& userID, OperatingSystem* os)
{

    pinfo->tcp->sendMsg(string("Please enter your username and password.\n"));
    
    string username = pinfo->tcp->recvMsg();
    string password = pinfo->tcp->recvMsg();
    downReader(os);
    if (os->loginSystems(username, password))
    {
        pinfo->tcp->sendMsg("Accepted");
        userID = os->getUserID(0, username);
        upReader(os);
        return true;
    } else 
    {
        pinfo->tcp->sendMsg("Rejected");
        upReader(os);
        return false;
    }

}

void printLog(OperatingSystem* os, string command)
{
    os->pushTag(command);
    os->writeSystem();
}

string publishHomework(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string homeworkName = pinfo->tcp->recvMsg();
    string option = pinfo->tcp->recvMsg();
    string content = (option == "1")? recvFile(pinfo): pinfo->tcp->recvMsg();

    string retMsg = "";
    string command = "Publish homework " + homeworkName + " to course " + course + ".";

    downWriter(os);
    if (os->checkPath(userID, string("/" + course)) == 0)
    {
        int ownerID = os->getOwner(0, string("/" + course));
        int groupID = os->getGroup(0, string("/" + course));
        if (os->checkPath(userID, string("/" + course + "/" + homeworkName)) == -1)
        {
            os->createDirectory(0, string("/" + course+ "/" + homeworkName), ownerID, groupID, 7, 7, 0);
            os->createFile(0, string("/" + course + "/" + homeworkName + "/_assignment"), ownerID, groupID, 7, 1, 0);
            os->writeFile(0, string("/" + course + "/" + homeworkName + "/_assignment"), content);
            printLog(os, command);
            retMsg = "Successfully published!";
        } else
        if (os->checkPath(userID, string("/" + course + "/" + homeworkName)) == -1)
        {
            os->writeFile(0, string("/" + course + "/" + homeworkName + "/_assignment"), content);
            printLog(os, command);
            retMsg = "Successfully updated!";
        } else
        {
            retMsg = "Homework publish failed.";
        }
    } else
    {
        retMsg = "Course does not exist.";
    }

    upWriter(os);
    return retMsg;
}

string checkHomework(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string homeworkName = pinfo->tcp->recvMsg();
    string studentName = pinfo->tcp->recvMsg();

    string retMsg = "";
    
    downReader(os);
    if (os->checkPath(userID, string("/" + course)) == 0)
    {
        if (os->checkPath(userID, string("/" + course + "/" + homeworkName)) == 0)
        {
            //os->listFiles(0, string("/" + course + "/" + homeworkName));
            if (os->checkPath(userID, string("/" + course + "/" + homeworkName + "/" + studentName)) == 0)
            {
                //os->listFiles(0, string("/" + course + "/" + homeworkName + "/" + studentName));
                vector<FileBlock> files = os->readFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_homework"));
                for (FileBlock& file: files)
                {
                    retMsg += string(file.content);
                }
            } else
            {
                retMsg = "The student did not submit the homework.";
            }
        } else
        {
            retMsg = "Homework dose not exist.";
        }
    } else
    {
        retMsg = "Course does not exist.";
    }

    upReader(os);
    return retMsg;
}

int isScoreValid(string score)
{
    if (score.size() >= 3) return false;
    for (auto ch: score)
    {
        if ((ch < '0') || (ch > '9'))
        {
            return -1;
        }
    }
    int scoreNum = stoi(score);
    if ((scoreNum >= 0) && (scoreNum <= 100))
    {
        return scoreNum;
    } else
    {
        return -1;
    }
}

string mark(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string homeworkName = pinfo->tcp->recvMsg();
    string studentName = pinfo->tcp->recvMsg();
    string score = pinfo->tcp->recvMsg();
    
    int scoreNum = isScoreValid(score);
    if (scoreNum == -1)
    {
        return string("Invalid score.");
    }

    string retMsg = "";
    string command = "Mark " + studentName + "'s homework " + homeworkName + " in course " + course + ".";
    
    downWriter(os);
    if (os->checkPath(userID, string("/" + course)) == 0)
    {
        if (os->checkPath(userID, string("/" + course + "/" + homeworkName)) == 0)
        {
            if (os->checkPath(userID, string("/" + course + "/" + homeworkName + "/" + studentName)) == 0)
            {
                vector<FileBlock> files = os->readFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_score"));
                string scoreAndComment = "";
                for (FileBlock& file: files)
                {
                    scoreAndComment += string(file.content);
                } 
                scoreAndComment.erase(0, 10);
                string scoreString = "Score: ";
                for (int i = 0; i < 3 - score.size(); i++)
                {
                    scoreString += " ";
                }
                scoreAndComment = scoreString + score + scoreAndComment;
                os->writeFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_score"), scoreAndComment);
                printLog(os, command);
                retMsg = "Successfully marking!";
            } else
            {
                retMsg = "The student did not submit the homework.";
            }
        } else
        {
            retMsg = "Homework dose not exist.";
        }
    } else
    {
        retMsg = "Course does not exist.";
    }

    upWriter(os);
    return retMsg;
}

string writeComment(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string homeworkName = pinfo->tcp->recvMsg();
    string studentName = pinfo->tcp->recvMsg();
    string option = pinfo->tcp->recvMsg();
    string content = (option == "1")? recvFile(pinfo): pinfo->tcp->recvMsg();

    string retMsg = "";
    string command = "Write command to " + studentName + "'s homework " + homeworkName + " in course " + course + ".";
    
    downWriter(os);
    if (os->checkPath(userID, string("/" + course)) == 0)
    {
        if (os->checkPath(userID, string("/" + course + "/" + homeworkName)) == 0)
        {
            if (os->checkPath(userID, string("/" + course + "/" + homeworkName + "/" + studentName)) == 0)
            {
                vector<FileBlock> files = os->readFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_score"));
                string scoreAndComment = "";
                for (FileBlock& file: files)
                {
                    scoreAndComment += string(file.content);
                } 
                string scoreString = scoreAndComment.substr(0, 10);
                scoreAndComment = scoreString + "\n" + "Comment:\n" + content;
                os->writeFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_score"), scoreAndComment);
                printLog(os, command);
                retMsg = "Successfully writing comment!";
            } else
            {
                retMsg = "The student did not submit the homework.";
            }
        } else
        {
            retMsg = "Homework dose not exist.";
        }
    } else
    {
        retMsg = "Course does not exist.";
    }
    upWriter(os);
    return retMsg;
}

string checkScoreAndComment(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string homeworkName = pinfo->tcp->recvMsg();
    string studentName = pinfo->tcp->recvMsg();

    string retMsg = "";
    
    downReader(os);
    if (os->checkPath(userID, string("/" + course)) == 0)
    {
        if (os->checkPath(userID, string("/" + course + "/" + homeworkName)) == 0)
        {
            if (os->checkPath(userID, string("/" + course + "/" + homeworkName + "/" + studentName)) == 0)
            {
                vector<FileBlock> files = os->readFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_score"));
                for (FileBlock& file: files)
                {
                    retMsg += string(file.content);
                } 
            } else
            {
                retMsg = "The student did not submit the homework.";
            }
        } else
        {
            retMsg = "Homework dose not exist.";
        }
    } else
    {
        retMsg = "Course does not exist.";
    }
    upReader(os);
    return retMsg;
}

string checkAssignment(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string homeworkName = pinfo->tcp->recvMsg();

    string retMsg = "";
    
    downReader(os);
    if (os->checkPath(userID, string("/" + course)) == 0)
    {
        if (os->checkPath(userID, string("/" + course + "/" + homeworkName)) == 0)
        {
            vector<FileBlock> files = os->readFile(0, string("/" + course + "/" + homeworkName + "/_assignment"));
            string scoreAndComment = "";
            for (FileBlock& file: files)
            {
                retMsg += string(file.content);
            } 
        } else
        {
            retMsg = "Homework dose not exist.";
        }
    } else
    {
        retMsg = "Course does not exist.";
    }
    upReader(os);
    return retMsg;
}

string submitHomework(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string homeworkName = pinfo->tcp->recvMsg();
    string studentName = pinfo->tcp->recvMsg();
    string option = pinfo->tcp->recvMsg();
    string content = (option == "1")? recvFile(pinfo): pinfo->tcp->recvMsg();

    string retMsg = "";
    string command = "Submit " + studentName + "'s homework to homework " + homeworkName + " in course " + course + ".";    
    downWriter(os);
    int studentID = os->getUserID(0, studentName);
    if (studentID != -1)
    {
        if (os->checkPath(studentID, string("/" + course)) == 0)
        {
            int ownerID = os->getOwner(0, string("/" + course));
            int groupID = os->getGroup(0, string("/" + course));
            if (os->checkPath(studentID, string("/" + course + "/" + homeworkName)) == 0)
            {
                //os->listFiles(0, string("/" + course + "/" + homeworkName));
                if (os->checkPath(studentID, string("/" + course + "/" + homeworkName + "/" + studentName)) == 0)
                {
                    os->writeFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_homework"), content);
                    printLog(os, command);
                    retMsg = "Successfully update homework content!";
                } else
                {
                    cout << "before create dir:" << endl;
                    os->listFiles(0, string("/" + course + "/" + homeworkName));
                    os->createDirectory(0, string("/" + course + "/" + homeworkName + "/" + studentName), ownerID, groupID, 7, 7, 0);
                    os->createFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_homework"), studentID, groupID, 7, 1, 0);
                    os->writeFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_homework"), content);
                    os->createFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_score"), ownerID, groupID, 7, 1, 0);
                    os->writeFile(0, string("/" + course + "/" + homeworkName + "/" + studentName + "/_score"), "Score:    \nComment:\n");
                    printLog(os, command);
                    retMsg = "Homework submitted successfully!";
                }
                //os->listFiles(0, string("/" + course + "/" + homeworkName + "/" + studentName));
            } else
            {
                retMsg = "Homework dose not exist.";
            }
        } else
        {
            retMsg = "Course does not exist.";
        }
    } else
    {
        retMsg = "Student does not exist.";
    }
    upWriter(os);
    return retMsg;
}

string deleteHomework(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string homeworkName = pinfo->tcp->recvMsg();

    string retMsg = "";
    string command = "Delete homework " + homeworkName + " in course " + course + ".";    

    downWriter(os);
    if (os->checkPath(userID, string("/" + course)) == 0)
    {
        int ownerID = os->getOwner(0, string("/" + course));
        if (os->checkPath(userID, string("/" + course + "/" + homeworkName)) == 0)
        {
            if ((userID == ownerID) || (os->getUserMainGroup(0, userID) == 0))
            {
                os->deleteDirectory(0, string("/" + course + "/" + homeworkName));
                printLog(os, command);
                retMsg = "Delete successfully.";
            } else
            {
                retMsg = "No permission.";
            }
        } else
        {
            retMsg = "The homework does not exists.";
        }
    } else
    {
        retMsg = "Course does not exist.";
    }
    upWriter(os);
    return retMsg;
}

string listCourse(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string checkUserName = pinfo->tcp->recvMsg();
    
    string retMsg = "All courses: \n";

    downReader(os);
    int checkUserID = os->getUserID(0, checkUserName);
    cout << "checkUserID" << checkUserID << endl;

    if (checkUserID != -1)
    {
        vector<DirectoryBlock> directories = os->listFiles(checkUserID, string("/"));
        for (DirectoryBlock& directory: directories)
        {
            struct DirectoryBlock* pdirectory = &directory;
            
            for (int i = 0; i < ENTRY_NUMBER; i++)
            {
                string fileName = string(pdirectory->fileName[i]);
                if (fileName.empty()) continue;
                cout << fileName << endl;
                if (os->checkPath(checkUserID, string("/" + fileName)) == 0)
                {
                    cout << fileName << endl;
                    retMsg += fileName + "\n";
                } 
            }
        }

    } else
    {
        retMsg = "User does not exist.";
    }
    upReader(os);

    return retMsg;
}

string listHomework(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string checkUserName = pinfo->tcp->recvMsg();
    
    string retMsg = "All homework: \n";

    downReader(os);
    int checkUserID = os->getUserID(0, checkUserName);

    if (checkUserID != -1)
    {
        cout << "checkUserID: " <<  checkUserID << endl;
        if (os->checkPath(checkUserID, string("/" + course)) == 0)
        {
            vector<DirectoryBlock> directories = os->listFiles(0, string("/" + course));
            for (DirectoryBlock& directory: directories)
            {
                struct DirectoryBlock* pdirectory = (struct DirectoryBlock* )(&directory);

                for (int i = 0; i < ENTRY_NUMBER; i++)
                {
                    string fileName = string(pdirectory->fileName[i]);
                    if (fileName.empty()) continue;
                    cout << fileName << endl;
                    if (os->checkPath(checkUserID, string("/" + course + "/" + fileName)) == 0)
                    {
                        retMsg += fileName + "\n";
                    } 
                }
            }
        } else
        {
            retMsg = "Course does not exist.";
        }
    } else
    {
        retMsg = "User does not exist.";
    }
    upReader(os);

    return retMsg;
}

string createCourse(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string teacherName = pinfo->tcp->recvMsg();
    cout << course << endl;
    cout << teacherName << endl;

    string retMsg = "";
    string command = "Create course " + course + ".";

    downWriter(os);
    os->listuser(0);
    int teacherID = os->getUserID(userID, teacherName);
    if (teacherID != -1)
    {
        if (os->getUserMainGroup(0, teacherID) == 1)
        {
            if (os->checkPath(userID, string("/" + course)) != 0)
            {
                int groupID = os->getGroupID(0, course);
                if (groupID == -1)
                {
                    groupID = os->createGroup(userID, course);
                    os->addUserToGroup(userID, teacherID, groupID);
                    os->createDirectory(userID, string("/" + course), teacherID, groupID, 7, 7, 0);
                    printLog(os, command);
                    retMsg = "Create course success!";
                } else
                {
                    retMsg = "Create course failed.";
                }
            } else
            {
                retMsg = "Course exists.";
            }
        } else
        {
            retMsg = "User is not a teacher.";
        }
        
    } else
    {
        retMsg = "Teacher does not exist.";
    }
    upWriter(os);

    return retMsg;
}

string deleteCourse(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();

    string retMsg = "";
    string command = "Delete course " + course + ".";

    downWriter(os);
    if (os->checkPath(userID, string("/" + course)) == 0)
    {
        os->deleteDirectory(0, string("/" + course));
        int courseID = os->getGroupID(0, course);
        os->deleteGroup(0, courseID);
        printLog(os, command);
        retMsg = "Delete course success!";
    } else
    {
        retMsg = "Course does not exist.";
    }
    upWriter(os);

    return retMsg;
}

string addToCourse(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string studentName = pinfo->tcp->recvMsg();

    string retMsg = "";
    string command = "Add student " + studentName + " to course " + course + "."; 

    downWriter(os);
    int studentID = os->getUserID(userID, studentName);
    if (studentID != -1)
    {
        if (os->checkPath(userID, string("/" + course)) == 0)
        {
            int courseID = os->getGroupID(userID, course);
            if (os->checkUserFromGroup(userID, studentID, courseID))
            {
                retMsg = "The student has already chosen the course.";
            } else 
            {
                os->addUserToGroup(userID, studentID, courseID);
                printLog(os, command);
                retMsg = "Successfully added!";
            }
        } else
        {
            retMsg = "Course does not exists.";
        }
    } else
    {
        retMsg = "Student does not exist.";
    }
    upWriter(os);

    return retMsg;
}

void deleteStudentAssignmentFromCourse(OperatingSystem* os, const int studentID, const string studentName, const int courseID, const string courseName)
{
    vector<DirectoryBlock> courseDirectories = os->listFiles(0, string("/" + courseName));
    for (DirectoryBlock& courseDirectory: courseDirectories)
    {
        for (int i = 0; i < ENTRY_NUMBER; i++)
        {
            if (string(courseDirectory.fileName[i]) != "")
            {
                string homeworkName = courseDirectory.fileName[i];
                if (os->checkPath(0, string("/" + courseName + "/" + homeworkName + "/" + studentName)) == 0)
                {
                    os->deleteDirectory(0, string("/" + courseName + "/" + homeworkName + "/" + studentName));
                }
            }
        }
    }
}

string deleteFromCourse(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    string studentName = pinfo->tcp->recvMsg();

    string retMsg = "";
    string command = "Delete student " + studentName + " from course " + course + "."; 

    downWriter(os);
    int studentID = os->getUserID(userID, studentName);
    if (studentID != -1)
    {
        if (os->checkPath(userID, string("/" + course)) == 0)
        {
            int courseID = os->getGroupID(userID, course);
            if (!os->checkUserFromGroup(userID, studentID, courseID))
            {
                retMsg = "Student has already dropped out of class.";
            } else 
            {
                deleteStudentAssignmentFromCourse(os, studentID, studentName, courseID, course);
                os->deleteUserFromGroup(userID, studentID, courseID);
                printLog(os, command);
                retMsg = "Successfully deleted!";
            }
        } else
        {
            retMsg = "Course does not exists.";
        }
    } else
    {
        retMsg = "Student does not exist.";
    }
    upWriter(os);

    return retMsg;
}

string listCourseUser(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string course = pinfo->tcp->recvMsg();
    
    string studentMsg = "Students:\n";
    string teacherMsg = "Teacher:\n";
    string retMsg = "";

    downReader(os);
    int courseID = os->getGroupID(0, course);
    
    if (courseID != -1)
    {
        vector<FileBlock> files = os->readFile(0, string("/_group"));
        for (FileBlock& file: files)
        {
            struct GroupBlock* pfile = (struct GroupBlock *) (&file);
            
            struct Group* pgroup = nullptr;
            for (int i = 0; i < MAX_GROUP_NUMBER; i++)
            {
                if (pfile->groups[i].groupID == courseID)
                {
                    pgroup = (struct Group *) (&pfile->groups[i]);
                }
            }
            if (pgroup == nullptr) continue;

            for (int i = 0; i < MAX_USER_NUMBER_IN_GROUP; i++)
            {
                string userName = os->getUserName(0, pgroup->userID[i]);
                if (userName != "")
                {
                    if (os->getUserMainGroup(0, pgroup->userID[i]) == 1)
                    {
                        teacherMsg += userName + "\n";
                    } 
                    if (os->getUserMainGroup(0, pgroup->userID[i]) == 2)
                    {
                        studentMsg += userName + "\n";
                    }
                }
            } 
        }
        retMsg = teacherMsg + "\n" + studentMsg;
    } else
    {
        retMsg = "Course does not exist.";
    }
    upReader(os);

    return retMsg;
}

string listGroupUser(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string groupName = pinfo->tcp->recvMsg();

    string retMsg = "";

    downReader(os);
    int groupID = os->getGroupID(userID, groupName);
    
    if (groupID != -1)
    {
        vector<FileBlock> files = os->readFile(userID, string("/_group"));
        for (FileBlock& file: files)
        {
            struct GroupBlock* pfile = (struct GroupBlock *) (&file);

            struct Group* pgroup = nullptr;
            for (int i = 0; i < MAX_GROUP_NUMBER; i++)
            {
                if (pfile->groups[i].groupID == groupID)
                {
                    pgroup = (struct Group *) (&pfile->groups[i]);
                }
            }
            if (pgroup == nullptr) continue;

            // cout << string(pgroup->groupName) << endl;
            for (int i = 0; i < MAX_USER_NUMBER_IN_GROUP; i++)
            {
                string userName = os->getUserName(userID, pgroup->userID[i]);
                if (userName != "")
                {
                    retMsg += userName + "\n";
                }
            }
        }
    } else
    {
        retMsg = "Group does not exist.";
    }
    upReader(os);

    return retMsg;
}

string createUser(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string groupID = pinfo->tcp->recvMsg();
    string newUserName = pinfo->tcp->recvMsg();
    string password = pinfo->tcp->recvMsg();

    if (groupID != "0" && groupID != "1" && groupID != "2")
    {
        return string("User identity error.");
    }
    int group = stoi(groupID);

    string retMsg = "";
    string command = "Create user " + newUserName + ".";

    downWriter(os);
    if (os->getUserID(userID, newUserName) == -1)
    {
        int newUserID = os->createUser(userID, newUserName, password, group);
        os->addUserToGroup(userID, newUserID, group);
        printLog(os, command);
        retMsg = "Created successfully!.";
    } else
    {
        retMsg = "The user already exists.";
    }
    upWriter(os);

    return retMsg;
}

void deleteTeacherCourse(OperatingSystem* os, const int teacherID, const string teacherName)
{
    vector<DirectoryBlock> allCourseDirectories = os->listFiles(0, string("/"));
    for (DirectoryBlock& courseDirectory: allCourseDirectories)
    {
        for (int i = 0; i < ENTRY_NUMBER; i++)
        {
            if (string(courseDirectory.fileName[i]) != "")
            {
                string courseName = courseDirectory.fileName[i];
                if ((os->checkPath(0, string("/" + courseName)) == 0) && (os->getOwner(0, string("/" + courseName)) == teacherID))
                {
                    int courseID = os->getGroupID(0, courseName);
                    os->deleteDirectory(0, string("/" + courseName));
                    os->deleteGroup(0, courseID);
                }
            }
        }
    }
}

void deleteStudentCourse(OperatingSystem* os, const int studentID, const string studentName)
{
    vector<DirectoryBlock> allCourseDirectories = os->listFiles(0, string("/"));
    for (DirectoryBlock& courseDirectory: allCourseDirectories)
    {
        for (int i = 0; i < ENTRY_NUMBER; i++)
        {
            if (string(courseDirectory.fileName[i]) != "")
            {
                string courseName = courseDirectory.fileName[i];
                if (os->checkPath(0, string("/" + courseName)) == 0)
                {
                    int courseID = os->getGroupID(0, courseName);
                    if (os->checkUserFromGroup(0, studentID, courseID))
                    {
                        deleteStudentAssignmentFromCourse(os, studentID, studentName, courseID, courseName);
                        os->deleteUserFromGroup(0, studentID, courseID);
                    }
                }
            }
        }
    }
}

string deleteUser(struct SockInfo*& pinfo, const int execUserID, const int userGroupID, OperatingSystem* os)
{
    string userName = pinfo->tcp->recvMsg();

    string retMsg = "";
    string command = "Delete user " + userName + ".";

    downWriter(os);
    int userID = os->getUserID(execUserID, userName);
    if (userID == -1)
    {
        retMsg = "The user does not exist.";
    } else
    if (userID == 0 || userID == 1)
    {
        retMsg = "The user cannot be deleted.";
    } else
    {
        int mainGroupID = os->getUserMainGroup(execUserID, userID);
        if (mainGroupID == 1)
        {
            deleteTeacherCourse(os, userID, userName);
        } else 
        if (mainGroupID == 2)
        {
            deleteStudentCourse(os, userID, userName);
        }
        os->deleteUserFromGroup(execUserID, userID, mainGroupID);
        os->deleteUser(execUserID, userID);
        printLog(os, command);
        retMsg = "Deleted successfully!.";
    }
    upWriter(os);

    return retMsg;
}

string modifyPassword(struct SockInfo*& pinfo, const int execUserID, const int userGroupID, OperatingSystem* os)
{
    string userName = pinfo->tcp->recvMsg();
    string oldPassword = pinfo->tcp->recvMsg();
    string newPassword = pinfo->tcp->recvMsg();
    string confirmPassword = pinfo->tcp->recvMsg();
    
    string retMsg = "";
    string command = "Modify user " + userName + "'s password.";

    downWriter(os);
    if (os->getUserID(0, userName) == -1)
    {
        retMsg = "User does not exist.";
    } else
    {
        if (os->loginSystems(userName, oldPassword))
        {
            if (newPassword == confirmPassword)
            {
                int userID = os->getUserID(0, userName);
                os->modifyUserPassword(0, userID, newPassword);
                printLog(os, command);
                retMsg = "Modified successfully!";
            } else
            {
                retMsg = "The two new passwords are different.";
            }
        } else
        {
            retMsg == "wrong old password.";
        }
    }
    upWriter(os);

    return retMsg;
}

string backup(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string versionName = pinfo->tcp->recvMsg();

    string retMsg = "";
    downWriter(os);
    if (os->backupSystem(userID, versionName))
    {
        retMsg = "Backup succeeded!";
    } else 
    {
        retMsg = "Backup failed.";
    }
    upWriter(os);
    return retMsg;
}

string recover(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string versionName = pinfo->tcp->recvMsg();

    string retMsg = "";
    string command = "Recover system to version " + versionName + ".";

    downWriter(os);
    if (os->recoverySystem(userID, versionName))
    {
        printLog(os, command);
        retMsg = "Recover succeeded!";
    } else 
    {
        retMsg = "Recover failed.";
    }
    upWriter(os);
    return retMsg;
}

string revoke(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string retMsg = "";
    downWriter(os);
    if (os->revoke())
    {
        retMsg = "Revoke succeeded!";
    } else 
    {
        retMsg = "Revoke failed.";
    }
    upWriter(os);
    return retMsg;
}

string clearsnapshot(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string retMsg = "";
    downWriter(os);
    if (os->initializeSnapshot())
    {
        retMsg = "clear succeeded!";
    } else 
    {
        retMsg = "clear failed.";
    }
    upWriter(os);
    return retMsg;
}

string checkLog(struct SockInfo*& pinfo, const int userID, const int userGroupID, OperatingSystem* os)
{
    string retMsg = "";
    downReader(os);
    retMsg = os->checkLog();
    upReader(os);
    return retMsg;
}

void systemWorking(struct SockInfo*& pinfo, const int& userID, OperatingSystem* os)
{
    downReader(os);
    int userGroupID = os->getUserMainGroup(0, userID);
    upReader(os);

    cout << userGroupID << endl;
    
    pinfo->tcp->sendMsg(os->getGroupName(0, userGroupID));

    // 处理不同指令
    while (1)
    {
        string optionMsg = pinfo->tcp->recvMsg();
        string retMsg = "";
        // cout << optionMsg << endl;
        if (optionMsg == "publishhomework")
        {
            retMsg = publishHomework(pinfo, userID, userGroupID, os);
        } else 
        if (optionMsg == "checkhomework")
        {
            retMsg = checkHomework(pinfo, userID, userGroupID, os);
        }  else 
        if (optionMsg == "mark")
        {
            retMsg = mark(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "writecomment")
        {
            retMsg = writeComment(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "checkscoreandcomment")
        {
            retMsg = checkScoreAndComment(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "checkassignment")
        {
            retMsg = checkAssignment(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "submithomework")
        {
            retMsg = submitHomework(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "deletehomework")
        {
            retMsg = deleteHomework(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "listcourse")
        {
            retMsg = listCourse(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "listhomework")
        {
            retMsg = listHomework(pinfo, userID, userGroupID, os);
        } else 
        if (optionMsg == "createcourse")
        {
            retMsg = createCourse(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "deletecourse")
        {
            retMsg = deleteCourse(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "addtocourse")
        {
            retMsg = addToCourse(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "deletefromcourse")
        {
            retMsg = deleteFromCourse(pinfo, userID, userGroupID, os);
        } else
        if(optionMsg == "listcourseuser")
        {
            retMsg = listCourseUser(pinfo, userID, userGroupID, os);
        } else 
        if (optionMsg == "listgroupuser")
        {
            retMsg = listGroupUser(pinfo, userID, userGroupID, os);
        } else 
        if (optionMsg == "createuser")
        {
            retMsg = createUser(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "deleteuser")
        {
            retMsg = deleteUser(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "modifypassword")
        {
            retMsg = modifyPassword(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "backup")
        {
            retMsg = backup(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "recover")
        {
            retMsg = recover(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "revoke")
        {
            retMsg = revoke(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "checklog")
        {
            retMsg = checkLog(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "clearsnapshot")
        {
            retMsg = clearsnapshot(pinfo, userID, userGroupID, os);
        } else
        if (optionMsg == "exit")
        {
            break;
        } else
        {
            continue;
        }
        cout << retMsg << endl;
        pinfo->tcp->sendMsg(retMsg);
    }
}

void* working(void* arg)
{
    struct SockInfoAndOS* ptmp = static_cast<struct SockInfoAndOS*>(arg);
    
    struct SockInfo* pinfo = ptmp->sockinfo;
    OperatingSystem* os = ptmp->os;

    // 连接建立成功, 打印客户端的IP和端口信息
    char ip[32];
    printf("客户端的IP: %s, 端口: %d\n",
        inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, ip, sizeof(ip)),
        ntohs(pinfo->addr.sin_port));

    pinfo->tcp->sendMsg(string("Welcome to the Student Management system!\n"));

    // 5. 登录
    int userID = -1; //登录的用户
    while (!login(pinfo, userID, os))
    {
        string option = pinfo->tcp->recvMsg();
        if (option == "exit")
        {
            break;
        }
    }
    
    if (userID != -1)
    {
        systemWorking(pinfo, userID, os);
    }
    ptmp->os = nullptr;
    os = nullptr;

    delete ptmp->sockinfo->tcp;
    delete ptmp->sockinfo;
    delete os;
    delete ptmp;
    return nullptr;
}

int main()
{
    // 将系统文件读入到内存
    OperatingSystem* os = new OperatingSystem();

    mutexInit(os);

    // 创建监听的套接字
    TcpServer s;

    // 绑定本地的IP port并设置监听
    s.setListen(10000);

    // 阻塞并等待客户端的连接
    while (1)
    {
        SockInfo* info = new SockInfo;
        TcpSocket* tcp = s.acceptConn(&info->addr);
        if (tcp == nullptr)
        {
            cout << "重试...." << endl;
            continue;
        }
        // 创建子线程
        pthread_t tid;
        info->s = &s;
        info->tcp = tcp;

        SockInfoAndOS* tmp = new SockInfoAndOS;
        tmp->sockinfo = info;
        tmp->os = os;

        pthread_create(&tid, NULL, working, tmp);
        pthread_detach(tid);
    }

    mutexDestroy(os);
    delete os;

    return 0;
}