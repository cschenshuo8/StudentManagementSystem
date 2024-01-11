#include "TcpSocket.h"
#include <stdio.h>  
#include <stdlib.h>
#include <termios.h>
#include <netinet/in.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <string.h>  
#include <unistd.h> 
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <string.h>

#define BuffSize 0xFF //可传输文件大小

bool sendFile(TcpSocket* tcp, string path)
{
    int fd1 = open(string("../../SendFiles/" + path).c_str(), O_RDONLY);
    if (fd1 == -1)
    {
        tcp->sendMsg(string(""));
        printf("File open failed.\n");
        return false;
    }
    
    int length = 0;
    char tmp[BuffSize];
    memset(tmp, 0, sizeof(tmp));
    while ((length = read(fd1, tmp, sizeof(tmp))) > 0)
    {
        // 发送数据
        tcp->sendMsg(string(tmp, length));

        memset(tmp, 0, sizeof(tmp));
    }
    tcp->sendMsg(string(""));
    return true;
}

bool login(TcpSocket* tcp, string& userInfo)
{
    string msg = tcp->recvMsg();
    cout << msg << endl;

    string username;
    cout << "Username: ";
    cin >> username;
    tcp->sendMsg(username);

    string password;
    cout << "Password: ";
    cin >> password;
    tcp->sendMsg(password);
    
    msg = tcp->recvMsg();
    cout << msg << endl;
    if (msg == "Accepted") 
    {
        userInfo = username;
        return true;
    } else
    {
        cout << "Incorrect username or password!\n" << endl;
        return false;
    }

}

void publishHomework(TcpSocket* tcp)
{
    tcp->sendMsg(string("publishhomework"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string homeworkName;
    cin >> homeworkName;
    tcp->sendMsg(homeworkName);

    string option = "";
    cin >> option;
    tcp->sendMsg(option);

    string content;
    cin >> content;
    if (option == "1")
    {
        sendFile(tcp, content);
    } else
    {
        tcp->sendMsg(content);
    }

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void checkHomework(TcpSocket* tcp, string userInfo)
{
    tcp->sendMsg(string("checkhomework"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string homeworkName;
    cin >> homeworkName;
    tcp->sendMsg(homeworkName);
    
    string studentName;
    if (userInfo == "")
    {
        cin >> studentName;
    } else
    {
        studentName = userInfo;
    }
    tcp->sendMsg(studentName);
    
    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void mark(TcpSocket* tcp)
{
    tcp->sendMsg(string("mark"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string homeworkName;
    cin >> homeworkName;
    tcp->sendMsg(homeworkName);

    string studentName;
    cin >> studentName;
    tcp->sendMsg(studentName);

    string score;
    cin >> score;
    tcp->sendMsg(score);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void writeComment(TcpSocket* tcp)
{
    tcp->sendMsg(string("writecomment"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string homeworkName;
    cin >> homeworkName;
    tcp->sendMsg(homeworkName);

    string studentName;
    cin >> studentName;
    tcp->sendMsg(studentName);

    string option = "";
    cin >> option;
    tcp->sendMsg(option);

    string content;
    cin >> content;
    if (option == "1")
    {
        sendFile(tcp, content);
    } else
    {
        tcp->sendMsg(content);
    }

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void checkScoreAndComment(TcpSocket* tcp, string userInfo)
{
    tcp->sendMsg(string("checkscoreandcomment"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string homeworkName;
    cin >> homeworkName;
    tcp->sendMsg(homeworkName);

    string studentName;
    if (userInfo == "")
    {
        cin >> studentName;
    } else
    {
        studentName = userInfo;
    }
    tcp->sendMsg(studentName);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void checkAssignment(TcpSocket* tcp)
{
    tcp->sendMsg(string("checkassignment"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string homeworkName;
    cin >> homeworkName;
    tcp->sendMsg(homeworkName);
    
    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void submitHomework(TcpSocket* tcp, string userInfo)
{
    tcp->sendMsg(string("submithomework"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string homeworkName;
    cin >> homeworkName;
    tcp->sendMsg(homeworkName);

    string studentName;
    if (userInfo == "")
    {
        cin >> studentName;
    } else
    {
        studentName = userInfo;
    }
    tcp->sendMsg(studentName);

    string option = "";
    cin >> option;
    tcp->sendMsg(option);

    string content;
    cin >> content;
    if (option == "1")
    {
        sendFile(tcp, content);
    } else
    {
        tcp->sendMsg(content);
    }

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void deleteHomework(TcpSocket* tcp)
{
    tcp->sendMsg(string("deletehomework"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string homeworkName;
    cin >> homeworkName;
    tcp->sendMsg(homeworkName);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void listCourse(TcpSocket* tcp, string userInfo)
{
    tcp->sendMsg(string("listcourse"));
    
    // infomation
    string userName;
    if (userInfo == "")
    {
        cin >> userName;
    } else
    {
        userName = userInfo;
    }
    //cout << userName << endl;
    tcp->sendMsg(userName);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void listHomework(TcpSocket* tcp, string userInfo)
{
    tcp->sendMsg(string("listhomework"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string userName;
    if (userInfo == "")
    {
        cin >> userName;
    } else
    {
        userName = userInfo;
    }
    tcp->sendMsg(userName);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void createCourse(TcpSocket* tcp)
{
    tcp->sendMsg(string("createcourse"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);
    
    string teacherName;
    cin >> teacherName;
    tcp->sendMsg(teacherName);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void deleteCourse(TcpSocket* tcp)
{
    tcp->sendMsg(string("deletecourse"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void addToCourse(TcpSocket* tcp)
{
    tcp->sendMsg(string("addtocourse"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);
    
    string studentName;
    cin >> studentName;
    tcp->sendMsg(studentName);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void deleteFromCourse(TcpSocket* tcp)
{
    tcp->sendMsg(string("deletefromcourse"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);
    
    string studentName;
    cin >> studentName;
    tcp->sendMsg(studentName);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void listCourseUser(TcpSocket* tcp)
{
    tcp->sendMsg(string("listcourseuser"));
    
    // 基本信息
    string course;
    cin >> course;
    tcp->sendMsg(course);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void listGroupUser(TcpSocket* tcp, string groupInfo)
{
    tcp->sendMsg(string("listgroupuser"));
    
    // 基本信息
    string groupName;
    groupName = groupInfo;
    tcp->sendMsg(groupName);

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void createUser(TcpSocket* tcp)
{
    tcp->sendMsg(string("createuser"));
    
    // 基本信息
    string groupID;
    cin >> groupID;
    tcp->sendMsg(groupID);
    
    string userName;
    cin >> userName;
    tcp->sendMsg(userName);
    
    string password;
    cin >> password;
    tcp->sendMsg(password);
    
    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void deleteUser(TcpSocket* tcp)
{
    tcp->sendMsg(string("deleteuser"));
    
    // 基本信息
    string userName;
    cin >> userName;
    tcp->sendMsg(userName);
    
    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void modifyPassword(TcpSocket* tcp, string userInfo)
{
    tcp->sendMsg(string("modifypassword"));
    
    // 基本信息
    string userName;
    if (userInfo == "")
    {
        cin >> userName;
    } else
    {
        userName = userInfo;
    }
    tcp->sendMsg(userName);

    cout << "Please enter your old password:" << endl;
    string oldPassword;
    cin >> oldPassword;
    tcp->sendMsg(oldPassword);
    
    cout << "Please enter your new password:" << endl;
    string newPassword;
    cin >> newPassword;
    tcp->sendMsg(newPassword);

    cout << "Please enter your new password again:" << endl;
    string confirmPassword;
    cin >> confirmPassword;
    tcp->sendMsg(confirmPassword);
    
    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void backup(TcpSocket* tcp)
{
    tcp->sendMsg(string("backup"));
    
    string versionName;
    cin >> versionName;
    tcp->sendMsg(string(versionName));

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void recover(TcpSocket* tcp)
{
    tcp->sendMsg(string("recover"));
    
    string versionName;
    cin >> versionName;
    tcp->sendMsg(string(versionName));

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void revoke(TcpSocket* tcp)
{
    tcp->sendMsg(string("revoke"));

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void clearsnapshot(TcpSocket* tcp)
{
    tcp->sendMsg(string("clearsnapshot"));

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void checkLog(TcpSocket* tcp)
{
    tcp->sendMsg(string("checklog"));

    string retMsg = tcp->recvMsg();
    cout << retMsg << endl;
}

void administratorSystem(TcpSocket* tcp, string userInfo)
{
    string option = "";
    while (option != "exit")
    {
        // 输出提示
        cout << userInfo << ", please enter your command(enter ""help"" to check all the command):" << endl;

        // 读入操作
        cin >> option;
        if (option == "help")
        {
            // 课程相关
            cout << "About course:" << endl;
            cout << "publishhomework [course name] [homework name] [content(0) or contentpath(1)] [content]: Publish an homework to a certain course." << endl;
            cout << "deletehomework [course name] [homework name]: Delete homework." << endl; 
            cout << "checkhomework [course name] [homework name] [student name]: Check homework." << endl;
            cout << "checkassignment [course name] [homework name]: Check homework assignment." << endl;
            cout << "mark [course name] [homework name] [student name] [score]: Mark homework." << endl;
            cout << "writecomment [course name] [homework name] [student name] [content(0) or contentpath(1)] [content]: Write a comment." << endl;
            cout << "checkscoreandcomment [course name] [homework name] [student name]: Check score and comment." << endl;
            cout << "submithomework [course name] [homework name] [student name] [content(0) or contentpath(1)] [content]: Submit homework." << endl;
            cout << "listcourse [user name]: List all the courses." << endl;
            cout << "listhomework [course name] [user name]: List all the homework." << endl;
            cout << "listcourseuser [course name]: List all teachers and students who have chosen this course." << endl;
            cout << "createcourse [course name] [teacher name]: Create a course." << endl;
            cout << "deletecourse [course name]: Delete a course." << endl;
            cout << "addtocourse [course name] [student name]: Add a student to a certain course." << endl;
            cout << "deletefromcourse [course name] [student name]: delete a student from a certain course." << endl;
            cout << endl;

            // 用户信息相关
            cout << "About user:" << endl;
            cout << "liststudent: List all the students." << endl;
            cout << "listteacher: List all the teachers." << endl;
            cout << "listadmin: List all the administrators." << endl;
            cout << "createuser [administrator(0) or teacher(1) or student(2)] [user name] [password]: Create a user." << endl;
            cout << "deleteuser [user name]: Delete a user." << endl;
            cout << "modifypassword [user name]: Modify password." << endl; 
            cout << endl;

            // 系统相关
            cout << "About system:" << endl;
            cout << "backup [version name]: Backup system." << endl;
            cout << "recover [version name]: Recover system to a certain version." << endl;
            cout << "revoke: Revoke last operation." << endl;
            cout << "checklog: Check log." << endl;
            cout << "clearsnapshot: clear snapshot file." << endl;
            cout << "exit: Exit the system." << endl;
        } else
        if (option == "publishhomework")
        {
            publishHomework(tcp);
        } else
        if (option == "checkhomework")
        {
            checkHomework(tcp, "");
        } else
        if (option == "mark")
        {
            mark(tcp);
        } else
        if (option == "writecomment")
        {
            writeComment(tcp);
        } else 
        if (option == "checkscoreandcomment")
        {
            checkScoreAndComment(tcp, "");
        } else
        if (option == "checkassignment")
        {
            checkAssignment(tcp);
        } else
        if (option == "submithomework")
        {
            submitHomework(tcp, "");
        } else
        if (option == "deletehomework")
        {
            deleteHomework(tcp);
        } else 
        if (option == "listcourse")
        {
            listCourse(tcp, "");
        } else 
        if (option == "listhomework")
        {
            listHomework(tcp, "");
        } else
        if (option == "createcourse")
        {
            createCourse(tcp);
        } else
        if (option == "addtocourse")
        {
            addToCourse(tcp);   
        } else
        if (option == "deletefromcourse")
        {
            deleteFromCourse(tcp);
        } else
        if (option == "deletecourse")
        {
            deleteCourse(tcp);   
        } else
        if (option == "listcourseuser")
        {
            listCourseUser(tcp);
        } else
        if (option == "listadmin")
        {
            listGroupUser(tcp, "admin");
        } else
        if (option == "liststudent")
        {
            listGroupUser(tcp, "student");
        } else
        if (option == "listteacher")
        {
            listGroupUser(tcp, "teacher");
        } else
        if (option == "createuser")
        {
            createUser(tcp);
        } else
        if (option == "deleteuser")
        {
            deleteUser(tcp);
        } else
        if (option == "modifypassword")
        {
            modifyPassword(tcp, "");
        } else
        if (option == "backup")
        {
            backup(tcp);
        } else
        if (option == "recover")
        {
            recover(tcp);
        } else
        if (option == "revoke")
        {
            revoke(tcp);
        } else
        if (option == "checklog")
        {
            checkLog(tcp);
        } else
        if (option == "clearsnapshot")
        {
            clearsnapshot(tcp);
        } else
        if (option == "exit")
        {
            tcp->sendMsg(string("exit"));
            cout << "System has exited!" <<endl;
            break;
        } else
        {
            cout << "Invalid command." << endl;
        }
    }
}

void teacherSystem(TcpSocket* tcp, string userInfo)
{
    string option = "";
    while (option != "exit")
    {
        // 输出提示
        cout << userInfo << ", please enter your command(enter ""help"" to check all the command):" << endl;

        // 读入操作
        cin >> option;
        if (option == "help")
        {
            // 课程相关
            cout << "About course:" << endl;
            cout << "publishhomework [course name] [homework name] [content(0) or contentpath(1)] [content]: Publish an homework to a certain course." << endl;
            cout << "deletehomework [course name] [homework name]: Delete homework." << endl; 
            cout << "checkhomework [course name] [homework name] [student name]: Check homework." << endl;
            cout << "checkassignment [course name] [homework name]: Check homework assignment." << endl;
            cout << "mark [course name] [homework name] [student name] [score]: Mark homework." << endl;
            cout << "writecomment [course name] [homework name] [student name] [content(0) or contentpath(1)] [content]: Write a comment." << endl;
            cout << "checkscoreandcomment [course name] [homework name] [student name]: Check score and comment." << endl;
            cout << "listcourse: List all the courses." << endl;
            cout << "listhomework [course name]: List all the homework." << endl;
            cout << "listcourseuser [course name]: List all teachers and students who have chosen this course." << endl;
            cout << endl;

            // 用户信息相关
            cout << "About user:" << endl;
            cout << "modifypassword: Modify password." << endl; 
            cout << endl;

            // 系统相关
            cout << "About system:" << endl;
            cout << "exit: Exit the system." << endl;
        } else
        if (option == "publishhomework")
        {
            publishHomework(tcp);
        } else
        if (option == "checkhomework")
        {
            checkHomework(tcp, "");
        } else
        if (option == "mark")
        {
            mark(tcp);
        } else
        if (option == "writecomment")
        {
            writeComment(tcp);
        } else 
        if (option == "checkscoreandcomment")
        {
            checkScoreAndComment(tcp, "");
        } else
        if (option == "checkassignment")
        {
            checkAssignment(tcp);
        } else
        if (option == "deletehomework")
        {
            deleteHomework(tcp);
        } else 
        if (option == "listcourse")
        {
            listCourse(tcp, userInfo);
        } else 
        if (option == "listhomework")
        {
            listHomework(tcp, userInfo);
        } else
        if (option == "listcourseuser")
        {
            listCourseUser(tcp);
        } else
        if (option == "modifypassword")
        {
            modifyPassword(tcp, userInfo);
        } else
        if (option == "exit")
        {
            tcp->sendMsg(string("exit"));
            cout << "System has exited!" <<endl;
            break;
        } else
        {
            cout << "Invalid command." << endl;
        }
    }
}

void studentSystem(TcpSocket* tcp, string userInfo)
{
    string option = "";
    while (option != "exit")
    {
        // 输出提示
        cout << userInfo << ", please enter your command(enter ""help"" to check all the command):" << endl;

        // 读入操作
        cin >> option;
        if (option == "help")
        {
            // 课程相关
            cout << "About course:" << endl;
            cout << "checkhomework [course name] [homework name]: Check homework." << endl;
            cout << "checkassignment [course name] [homework name]: Check homework assignment." << endl;
            cout << "checkscoreandcomment [course name] [homework name]: Check score and comment." << endl;
            cout << "submithomework [course name] [homework name] [content(0) or contentpath(1)] [content]: Submit homework." << endl;
            cout << "listcourse: List all the courses." << endl;
            cout << "listhomework [course name]: List all the homework." << endl;
            cout << endl;

            // 用户信息相关
            cout << "About user:" << endl;
            cout << "modifypassword: Modify password." << endl; 
            cout << endl;

            // 系统相关
            cout << "About system:" << endl;
            cout << "exit: Exit the system." << endl;
        } else
        if (option == "checkhomework")
        {
            checkHomework(tcp, userInfo);
        } else
        if (option == "checkscoreandcomment")
        {
            checkScoreAndComment(tcp, userInfo);
        } else
        if (option == "checkassignment")
        {
            checkAssignment(tcp);
        } else
        if (option == "submithomework")
        {
            submitHomework(tcp, userInfo);
        } else
        if (option == "listcourse")
        {
            listCourse(tcp, userInfo);
        } else 
        if (option == "listhomework")
        {
            listHomework(tcp, userInfo);
        } else
        if (option == "modifypassword")
        {
            modifyPassword(tcp, userInfo);
        } else
        if (option == "exit")
        {
            tcp->sendMsg(string("exit"));
            cout << "System has exited!" <<endl;
            break;
        } else
        {
            cout << "Invalid command." << endl;
        }
    }
}

void working(TcpSocket* tcp, string userInfo)
{

    string identityMsg = "";
    identityMsg = tcp->recvMsg();

    cout << "Welcome " << identityMsg << " " << userInfo << "!" << endl;

    if (identityMsg == "admin")
    {
        administratorSystem(tcp, userInfo);
    } else
    if (identityMsg == "teacher")
    {
        teacherSystem(tcp, userInfo);
    } else
    if (identityMsg == "student")
    {
        studentSystem(tcp, userInfo);
    }
}

int main()
{

    // 1. 创建通信的套接字
    TcpSocket* tcp = new TcpSocket;

    // 2. 连接服务器IP port （记得替换）
    int ret = tcp->connectToHost("127.0.0.0", 10000); 
    if (ret == -1)
    {
        //printf("-1\n");
        return -1;
    }
    string welcomeMsg = tcp->recvMsg();
    cout << welcomeMsg << endl;

    // 3. 登录
    string userInfo = "";
    while (!login(tcp, userInfo))
    {
        cout << "Please retry!" << endl;
        tcp->sendMsg(string("retry"));
    }

    // 4. 进入系统
    working(tcp, userInfo);

    delete tcp;

    return 0;
}