#include "FileSnapshot.h"

FileSnapshot::FileSnapshot()
{

    this->readFromFile();
}

FileSnapshot::~FileSnapshot()
{
    writeToFile();
}

bool FileSnapshot::readFromFile()
{

    ifstream snapshotFile("../../OSFile/FileSnapshot/blockstack", ios::in | ios::binary);

	if (!snapshotFile.is_open())
    {
		cerr << "Failed to open file: blockstack" << std::endl;
		return false;
	}

    nowPointer = topPointer =  maxPointer = 0;
    snapshotStack.clear();
    snapshotFile.read(reinterpret_cast<char*> (&nowPointer), sizeof(int));
    snapshotFile.read(reinterpret_cast<char*> (&maxPointer), sizeof(int));
    snapshotFile.read(reinterpret_cast<char*> (&topPointer), sizeof(int));
    for (int i = 0; i <= topPointer; i++)
    {
        BlockAndInode tmp;
        snapshotFile.read(reinterpret_cast<char*> (&tmp), sizeof(struct BlockAndInode));
        snapshotStack.push_back(tmp);
    }


	// Close the file
	snapshotFile.close();

	std::cout << "Snapshot read from file: blockstack" << std::endl;
    return true;
}

bool FileSnapshot::writeToFile()
{
    ofstream snapshotFile("../../OSFile/FileSnapshot/blockstack", ios::out | ios::binary | ios::trunc);
    if (!snapshotFile.is_open())
    {
        cout << "Failed to open blockstack." << endl;
        return false;
    }

    cout << "open file!" << endl;
    snapshotFile.write(reinterpret_cast<char*>(&nowPointer), sizeof(int));
    snapshotFile.write(reinterpret_cast<char*> (&maxPointer), sizeof(int));
    snapshotFile.write(reinterpret_cast<char*>(&topPointer), sizeof(int));
    for (int i = 0; i <= topPointer; i++)
    {
        snapshotFile.write(reinterpret_cast<char*> (&snapshotStack[i]), sizeof(struct BlockAndInode));
    }
    
    snapshotFile.close();
    cout << "Snapshot file write successful." << endl;
    return true;
}

bool FileSnapshot::init()
{
    snapshotStack.clear();
    topPointer = nowPointer = maxPointer = 0;
    
    BlockAndInode tmp;
    memset(tmp.block.content, '\0', sizeof(tmp.block.content));
    tmp.inode.blockID = tmp.inode.fileType = tmp.inode.groupID = tmp.inode.groupPermission = tmp.inode.inodeNumber = tmp.inode.otherPermission = tmp.inode.ownerPermission = tmp.inode.userID = 0;
    tmp.blockID = tmp.inodeID = tmp.blockbit = -1;

    snapshotStack.push_back(tmp);

    
    return writeToFile();
}

bool FileSnapshot::pushcc(const BlockAndInode& blockAndInode)
{
    if (nowPointer == topPointer)
    {
        snapshotStack.push_back(blockAndInode);
        topPointer++;
        nowPointer++;
    } else
    {
        nowPointer++;
        snapshotStack[nowPointer] = blockAndInode;
    }
    maxPointer = nowPointer;

    return true;
}

bool FileSnapshot::pop()
{
    if (nowPointer < 1) return false;
    nowPointer--;

    return true;
}

bool FileSnapshot::revokePop()
{
    if (nowPointer >= maxPointer) return false;
    nowPointer++;

    return true;
}