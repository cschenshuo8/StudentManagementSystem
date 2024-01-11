#include "Log.h"

Log::Log()
{
    readFromFile();
}

Log::~Log()
{
    writeToFile();
}

bool Log::readFromFile()
{
    ifstream logFile("../../OSFile/FileSnapshot/Log", ios::in);

	if (!logFile.is_open())
    {
		cerr << "Failed to open file: Log" << std::endl;
		return false;
	}

    string st = "";
    commandStack.clear();
    while(getline(logFile, st))
    {
        commandStack.push_back(st);
    }

	// Close the file
	logFile.close();

	cout << "Log read from file: Log" << std::endl;
    return true;
}

bool Log::writeToFile()
{
    ofstream logFile("../../OSFile/FileSnapshot/Log", ios::out | ios::trunc);
    if (!logFile.is_open())
    {
        cout << "Failed to open Log." << endl;
        return false;
    }

    for (int i = 0; i < commandStack.size(); i++)
    {
        logFile << commandStack[i] << "\n";
    }
    
    logFile.close();
    cout << "Log file write successful." << endl;
    return true;
}

bool Log::init()
{
    cout << "zzz" << endl;
    commandStack.clear();
    return writeToFile();
}

bool Log::push(string command)
{
    commandStack.push_back(command);
    cout << "command" << endl;
    writeToFile();
    return true;
}

bool Log::pop()
{
    if (commandStack.size() == 0) return false;
    commandStack.pop_back();
    writeToFile();
    return true;
}

void Log::printLog()
{
    cout << "Log:" << endl;
    for (int i = 0; i < commandStack.size(); i++)
        cout << commandStack[i] << endl;
}

string Log::checkLog()
{
    string retMsg = "Log: \n";
    for (int i = 0; i < commandStack.size(); i++)
        retMsg +=  commandStack[i] + "\n";
    return retMsg;
}