#include"OperatingSystem.h"
#define _CRT_SECURE_NO_WARNINGS
OperatingSystem::OperatingSystem(struct Inode inodeMem[INODE_NUMBER], struct FileBlock blockMem[BLOCK_NUMBER], char blockBitmap[BLOCK_NUMBER / 8]) {
    memcpy(this->inodeMem, inodeMem, sizeof(struct Inode) * INODE_NUMBER);
    memcpy(this->blockMem, blockMem, sizeof(struct FileBlock) * BLOCK_NUMBER);
    memcpy(this->blockBitmap, blockBitmap, BLOCK_NUMBER / 8);
}

OperatingSystem::OperatingSystem() {
	// Initialize the system by reading information from a file
	std::ifstream systemFile("../../OSFile/system.txt", std::ios::binary);

	if (!systemFile.is_open()) {
		std::cerr << "Failed to open system file: system.txt" << std::endl;
		return;
	}

	// Read data from the file and initialize class members
	systemFile.read(reinterpret_cast<char*>(inodeMem), sizeof(struct Inode) * INODE_NUMBER);
	systemFile.read(reinterpret_cast<char*>(blockMem), sizeof(struct FileBlock) * BLOCK_NUMBER);
    systemFile.read(blockBitmap, BLOCK_NUMBER / 8);

	// Close the file
	systemFile.close();

    userBlock = (struct UserBlock*)&OperatingSystem::blockMem[1];
    groupBlock = (struct GroupBlock*)&OperatingSystem::blockMem[2];
    passwordBlock = (struct PasswordBlock*)&OperatingSystem::blockMem[3];

    snapshot = new FileSnapshot();
    systemLog = new Log();

	std::cout << "System initialized from file: system.txt" << std::endl;
}

bool OperatingSystem::writeSystem() {
    ofstream systemFile("../../OSFile/system.txt", ios::binary);
    if (!systemFile.is_open()) {
        cout << "Failed to open backup file." << endl;
        return false;
    }
    systemFile.write(reinterpret_cast<char*>(inodeMem), sizeof(struct Inode) * INODE_NUMBER);
    systemFile.write(reinterpret_cast<char*>(blockMem), sizeof(struct FileBlock) * BLOCK_NUMBER);
    systemFile.write(blockBitmap, BLOCK_NUMBER / 8);
    // Close the backup file
    systemFile.close();
    cout << "System file write successful." << endl;
    return true;
}

OperatingSystem::~OperatingSystem() {
    this->writeSystem();

    delete snapshot;
    delete systemLog;
}

bool OperatingSystem::loginSystems(const string userName, const string password)
{
    for (int i = 0; i < MAX_USER_NUMBER; i++)
    {
        string name(userBlock->users[i].userName);
        if (name == userName)
        {
            string pass(passwordBlock->passwords[i].password);
            if (pass == password)
            {
                cout << "ID:" << passwordBlock->passwords[i].userID << " Login Successfully!" << endl;
                return true;
            }
            else
            {
                cout << "Wrong password, please try again." << endl;
                return false;
            }
        }
    }
    cout << "User doesn't exiest." << endl;
    return false;
}

int OperatingSystem::createUser(const int execUserID, const string user_Name, const string pass_word, const int groupID)
{
    pushBlockAndInode(userBlockID, userInodeID);
    pushBlockAndInode(passwordBlockID, passwordInodeID);

    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to create a user." << endl;
        return -1;
    }
    //�ж���ȡ��group�Ƿ����
    if (groupBlock->groups[groupID].groupID == -1) {
        printf("The group does not exist \n");
        return false;
    }
    //�ж�user�������Ƿ��Ѿ�����
	for (int i = 0; i < MAX_USER_NUMBER; i++)
	{
		string name(userBlock->users[i].userName);
		if (user_Name == name)
		{
			cout << "Name " << name << " has exist." << endl;
			return -1;
		}
	}
	// ����user��password
	User user;
	Password password;
	int find = 0;
    for (short i = 0; i < MAX_USER_NUMBER; i++)
    {
        if (userBlock->users[i].userID == -1)
        {
            find = 1;
            user.userID = i;
            break;
        }
    }
    if (find == 0)
    {
        cout << "Teacher and student's number has reached the maximum." << endl;
        return -1;
    }
	// ����user
	user.groupID = groupID;
	strcpy(user.userName, user_Name.c_str());
	userBlock->users[user.userID] = user;

	// ����password
	password.userID = user.userID;
	strcpy(password.password, pass_word.c_str());
	passwordBlock->passwords[password.userID] = password;
    cout << "UserID is " << user.userID << endl;

	return user.userID;
}

bool OperatingSystem::deleteUser(const int execUserID, const int userID)
{
    pushBlockAndInode(userBlockID, userInodeID);
    pushBlockAndInode(passwordBlockID, passwordInodeID);

    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to delete a user." << endl;
        return -1;
    }
    //�ж�����userID�Ƿ����
    if (userBlock->users[userID].userID == -1) {
        printf("The user does not exist \n");
        return false;
    }
	if ((userID >= 0 && userID < MAX_USER_NUMBER) && userBlock->users[userID].userID != -1){
		string empty = "";

		userBlock->users[userID].groupID = -1;
		userBlock->users[userID].userID = -1;
		strcpy(userBlock->users[userID].userName, empty.c_str());

		passwordBlock->passwords[userID].userID = -1;
		strcpy(passwordBlock->passwords[userID].password, empty.c_str());

		return true;
	}
	else{
		std::cout << "Don't exist user: " << userID << "." << endl;
		return false;
	}
}

bool OperatingSystem::listuser(const int execUserID) {
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        std::cout << "You don't have right to list a user." << endl;
        return -1;
    }
    std::cout << std::setw(15) << "Username" << std::setw(10) << "UserID" << std::setw(15) << "GroupID\n";
    std::cout << "----------------------------------------------\n";

    for (int i = 0; i < MAX_USER_NUMBER; ++i) {
        std::cout << std::setw(15) << userBlock->users[i].userName << std::setw(10) << userBlock->users[i].userID << std::setw(15) << userBlock->users[i].groupID << "\n";
    }

    std::cout << "----------------------------------------------\n";
}

int OperatingSystem::getUserID(const int execUserID, const string userName)
{
	int flag = 0;
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to get user id." << endl;
        return -1;
    }
	for (int i = 0; i < MAX_USER_NUMBER; i++)
	{
		string name(userBlock->users[i].userName);
		if (name == userName)
		{
			flag = 1;
			return userBlock->users[i].userID;
		}
	}
	if (flag == 0)
	{
		cout << "Don't exist user: " << userName << "." << endl;
		return -1;
	}
}

string OperatingSystem::getUserName(const int execUserID, const int userID)
{
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to get user name." << endl;
        return "";
    }
	if ((userID >= 0 && userID < MAX_USER_NUMBER) && userBlock->users[userID].userID != -1)
	{
		string name(userBlock->users[userID].userName);
		return name;
	}
	else
	{
		cout << "Don't exist user: " << userID << "." << endl;
		return "";
	}
}

int OperatingSystem::getUserMainGroup(const int execUserID, const int userID)
{
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to get user main group." << endl;
        return -1;
	}
	if ((userID >= 0 && userID < MAX_USER_NUMBER) && userBlock->users[userID].userID != -1)
	{
		int group(userBlock->users[userID].groupID);
		return group;
	}
	else
	{
		cout << "Don't exist user: " << userID << "." << endl;
		return -1;
	}
}

bool OperatingSystem::modifyUserName(const int execUserID, const int userID, const string userName)
{
    pushBlockAndInode(userBlockID, userInodeID);
    
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to modify user name." << endl;
        return false;
    }
	if ((userID >= 0 && userID < MAX_USER_NUMBER) && userBlock->users[userID].userID != -1)
    {
		strcpy(userBlock->users[userID].userName, userName.c_str());
		return true;
	}
	else{
		cout << "Don't exist user: " << userID << "." << endl;
		return false;
	}
}

bool OperatingSystem::modifyUserPassword(const int execUserID, const int userID, const string password)
{
    pushBlockAndInode(passwordBlockID, passwordInodeID);
    
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to modify user password." << endl;
        return false;
    }
	if ((userID >= 0 && userID < MAX_USER_NUMBER) && userBlock->users[userID].userID != -1){
		strcpy(passwordBlock->passwords[userID].password, password.c_str());
		return true;
	}
	else{
		cout << "Don't exist user: " << userID << "." << endl;
		return false;
	}
}

bool OperatingSystem::modifyUserGroup(const int execUserID, const int userID, const int groupID)
{
    pushBlockAndInode(userBlockID, userInodeID);
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to modify user group." << endl;
        return false;
    }
	if ((userID >= 0 && userID < MAX_USER_NUMBER) && userBlock->users[userID].userID != -1 && groupID >= 0 && groupID < MAX_GROUP_NUMBER){
		short group(groupID);
		userBlock->users[userID].groupID = group;
		return true;
	}
	else{
		cout << "Don't exist user: " << userID << "." << endl;
		return false;
	}
}

bool OperatingSystem::listgroup(const int execUserID) {
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        std::cout << "You don't have right to list a group." << endl;
        return -1;
    }
    std::cout << std::setw(20) << "Group Name" << std::setw(10) << "Group ID" << std::setw(20) << "User IDs\n";
    std::cout << "------------------------------------------------------------\n";

    for (int i = 0; i < MAX_GROUP_NUMBER; ++i) {
        std::cout << std::setw(20) << groupBlock->groups[i].groupName << std::setw(10) << groupBlock->groups[i].groupID << std::setw(20);

        for (int j = 0; j < MAX_USER_NUMBER_IN_GROUP; ++j) {
            if (groupBlock->groups[i].userID[j] != -1) {
                std::cout << groupBlock->groups[i].userID[j] << " ";
            }
        }

        std::cout << "\n";
    }

    std::cout << "------------------------------------------------------------\n";
}

int OperatingSystem::createGroup(const int execUserID, const string groupName)
{
    pushBlockAndInode(groupBlockID, groupInodeID);

    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to create a group." << endl;
        return -1;
    }
	for (int i = 0; i < MAX_GROUP_NUMBER; i++)
	{
		string name(groupBlock->groups[i].groupName);
		if (groupName == name)
		{
			cout << "Name " << name << " has exist." << endl;
			return -1;
		}
	}

	// ����group
	Group group;
	int find = 0;
	for (short i = 0; i < MAX_GROUP_NUMBER; i++)
	{
		if (groupBlock->groups[i].groupID == -1)
		{
			find = 1;
			group.groupID = i;
			break;
		}
	}
	if (find == 0)
	{
		cout << "Group number has reached the maximum." << endl;
		return -1;
	}
    for (int j = 0; j < MAX_USER_NUMBER_IN_GROUP; j++) {
        group.userID[j] = -1;
    }
	strcpy(group.groupName, groupName.c_str());
	groupBlock->groups[group.groupID] = group;

	return group.groupID;
}

bool OperatingSystem::deleteGroup(const int execUserID, const int groupID)
{
    pushBlockAndInode(groupBlockID, groupInodeID);

    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to delete a group." << endl;
        return -1;
    }
	if ((groupID >= 0 && groupID < MAX_GROUP_NUMBER) && groupBlock->groups[groupID].groupID != -1){
		string empty = "";
		groupBlock->groups[groupID].groupID = -1;
		strcpy(groupBlock->groups[groupID].groupName, empty.c_str());
		for (int i = 0; i < MAX_USER_NUMBER_IN_GROUP; i++)
		{
			groupBlock->groups[groupID].userID[i] = -1;
		}

		return true;
	}
	else{
		cout << "Don't exist group: " << groupID << "." << endl;
		return false;
	}
}

int OperatingSystem::getGroupID(const int execUserID, const string groupName)
{
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to get group id." << endl;
        return -1;
    }
    int flag = 0;
	for (int i = 0; i < MAX_GROUP_NUMBER; i++){
		string name(groupBlock->groups[i].groupName);
		if (name == groupName)
		{
			flag = 1;
			return groupBlock->groups[i].groupID;
		}
	}
	if (flag == 0){
		cout << "Don't exist group: " << groupName << "." << endl;
		return -1;
	}
}

string OperatingSystem::getGroupName(const int execUserID, const int groupID)
{
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to get group name." << endl;
        return "";
	}
	if ((groupID >= 0 && groupID < MAX_GROUP_NUMBER) && groupBlock->groups[groupID].groupID != -1)
	{
		string name(groupBlock->groups[groupID].groupName);
		return name;
	}
	else
	{
		cout << "Don't exist group: " << groupID << "." << endl;
		return "";
	}
}

bool OperatingSystem::addUserToGroup(const int execUserID, const int userID, const int groupID)
{
    pushBlockAndInode(groupBlockID, groupInodeID);

    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout <<"You don't have right to add the user to group."  << endl;
        return -1;
    }
	if ((groupID >= 0 && groupID < MAX_GROUP_NUMBER) && groupBlock->groups[groupID].groupID != -1){
		if ((userID >= 0 && userID < MAX_USER_NUMBER) && userBlock->users[userID].userID != -1){
			short id(userID);
			int find = 0;
			for (int i = 0; i < MAX_USER_NUMBER_IN_GROUP; i++){
				if (groupBlock->groups[groupID].userID[i] == -1){
					find = 1;
					groupBlock->groups[groupID].userID[i] = id;
					break;
				}
			}
			if (find == 0){
				cout << "Member in group has reached the maximum." << endl;
				return false;
			}
			return true;
		}
		else
		{
			cout << "Don't exist user: " << userID << "." << endl;
			return false;
		}
	}
	else{
		cout << "Don't exist group: " << groupID << "." << endl;
		return false;
	}
}

bool OperatingSystem::deleteUserFromGroup(const int execUserID, const int userID, const int groupID)
{
    pushBlockAndInode(groupBlockID, groupInodeID);

    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to delete the user from group."  << endl;
        return -1;
    }
	if ((groupID >= 0 && groupID < MAX_GROUP_NUMBER) && groupBlock->groups[groupID].groupID != -1){
		if ((userID >= 0 && userID < MAX_USER_NUMBER) && userBlock->users[userID].userID != -1){
			short id(userID);
			int find = 0;
			for (int i = 0; i < MAX_USER_NUMBER_IN_GROUP; i++)
			{
				if (groupBlock->groups[groupID].userID[i] == id)
				{
					find = 1;
					groupBlock->groups[groupID].userID[i] = -1;
					break;
				}
			}
			if (find == 0){
				cout << "User: " << userID << " not in the group: " << groupID << "." << endl;
				return false;
			}
			return true;
		}
		else{
			cout << "Don't exist user: " << userID << "." << endl;
			return false;
		}
	}
	else{
		cout << "Don't exist group: " << groupID << "." << endl;
		return false;
	}
}

bool OperatingSystem::checkUserFromGroup(const int execUserID, const int userID, const int groupID)
{
    //�ж��Ƿ�Ϊ����Ա
    if (userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to check user from group." << endl;
        return false;
    }
	if ((groupID >= 0 && groupID < MAX_GROUP_NUMBER) && groupBlock->groups[groupID].groupID != -1){
		if ((userID >= 0 && userID < MAX_USER_NUMBER) && userBlock->users[userID].userID != -1){
			short id(userID);
			int find = 0;
			for (int i = 0; i < MAX_USER_NUMBER_IN_GROUP; i++){
				if (groupBlock->groups[groupID].userID[i] == id){
					find = 1;
                    cout << "user is in the block" << endl;
					break;
				}
			}
			if (find == 0) return false;
			return true;
		}
		else{
			cout << "Don't exist user: " << userID << "." << endl;
			return false;
		}
	}
	else{
		cout << "Don't exist group: " << groupID << "." << endl;
		return false;
	}
}

bool OperatingSystem::createDirectory(const int execUserID, string path, const int userID, const int groupID, const char ownerPermission, const char groupPermission, const char otherPermission) {
    cout << "1" << endl;
    if (userBlock->users[execUserID].userID == -1) {
        printf("The user does not exist \n");
        return false;
    }
    if (userBlock->users[userID].userID == -1 || userID < 0 || userID >= MAX_USER_NUMBER) {
        printf("The user does not exist \n");
        return false;
    }
    if (groupBlock->groups[groupID].groupID == -1 || groupID < 0 || groupID >= MAX_GROUP_NUMBER) {
        printf("The group does not exist \n");
        return false;
    }
    if (ownerPermission >= 255 | ownerPermission < 0 | groupPermission >= 255 | groupPermission < 0 | otherPermission >= 255 | otherPermission < 0) {
        printf("The permission can not be inputted like this \n");
        return false;
    }
    int i, j, flag;
    char* directory, * parent, * target;
    const char delimiter[2] = "/";
    struct DirectoryBlock* block;
    struct Inode* pointer = this->inodeMem;
    char paths[100];
    strcpy(paths, path.c_str());
    parent = NULL;
    directory = strtok(paths , delimiter);
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return false;
    }
    while (directory != NULL) {
        if (parent != NULL) {
            block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
            flag = 0;
            for (i = 0; i < ENTRY_NUMBER; i++) {
                if (strcmp(block->fileName[i], parent) == 0) {
                    if (!checkWritePermission(execUserID, block->inodeID[i])) {
                        printf("Permission denied;\n");
                        return false;
                    }
                    flag = 1;
                    pointer = &inodeMem[block->inodeID[i]];
                    break;
                }
            }
            if (flag == 0 || pointer->fileType == 1) {
                // If the parent directory does not exist or is not a directory file
                printf("The path does not exist!\n");
                return false;
            }
        }
        parent = directory;
        directory = strtok(NULL, delimiter);
    }

    int entry, n_block = -1, n_inode = -1;
    target = parent;
    block = (struct DirectoryBlock*)&blockMem[pointer->blockID];

    cout << "mkdir:" << endl;
    cout << pointer->blockID << endl;
    pushBlockAndInode(pointer->blockID, pointer->inodeNumber);

    for (int i = 0; i < ENTRY_NUMBER; i++) {
        if (strcmp(block->fileName[i], target) == 0) {
            printf("The directory already exists\n");
            return false;
        }
        if (block->inodeID[i] == -1) {
            entry = i;
            break;
        }
    }

    if (entry >= 0) {
        // Find a unused block
        for (i = 0; i < BLOCK_NUMBER / 8; i++) {
            for (j = 0; j < 8; j++) {
                if ((blockBitmap[i] & (1 << j)) == 0) {
                    cout << i << " " << j << endl;
                    n_block = i * 8 + j;
                    break;
                }
            }
            if (n_block != -1) {
                break;
            }
        }
        if (n_block == -1) {
            printf("The block is full!\n");
            return false;
        }
        cout << "n_block:" << n_block << endl;

        // Find a unused inode
        flag = 0;
        for (i = 0; i < INODE_NUMBER; i++) {
            if (inodeMem[i].blockID == -1) {
                flag = 1;
                n_inode = i;
                break;
            }
        }
        if (n_inode == -1) {
            printf("The inode is full!\n");
            return false;
        }

        cout << "n_block:" << n_block << endl;
        pushBlockAndInode(n_block, n_inode);

        // init inode
        inodeMem[n_inode].groupPermission = groupPermission;
        inodeMem[n_inode].ownerPermission = ownerPermission;
        inodeMem[n_inode].otherPermission = otherPermission;
        inodeMem[n_inode].blockID = n_block;
        inodeMem[n_inode].fileType = 0;
        inodeMem[n_inode].groupID = groupID;
        inodeMem[n_inode].userID = userID;

        // init block
        block->inodeID[entry] = n_inode;
        strcpy(block->fileName[entry], target);
        blockBitmap[n_block / 8] |= 1 << (n_block % 8);
        block = (struct DirectoryBlock*)&blockMem[n_block];
        for (i = 0; i < ENTRY_NUMBER; i++) {
            block->inodeID[i] = -1;
            memset(block->fileName[i], '\0', sizeof(block->fileName[i]));
        }

        printf("Directory created.\n");
    }
    else {
        struct IndexBlock* indirectBlock = (struct IndexBlock*)&blockMem[pointer->blockID];
        if (pointer->fileType != 2)
        {
            for (int i = 0; i < INDEX_SIZE; ++i)
            {
                indirectBlock->blockPointers[i] = -1;
            }
            pointer->fileType = 2;

            for (int i = 0; i < 2; ++i)
            {
                int blockID = findBlock();
                if (blockID == -1) {
                    printf("Failed to allocate block.\n");
                    return false;
                }
                else
                {
                    blockBitmap[blockID / 8] |= 1 << (blockID % 8);
                }
                indirectBlock->blockPointers[i] = blockID;
            }

            // 复制初始目录块到 blockPointers[0]
            memcpy(&blockMem[indirectBlock->blockPointers[0]], block, sizeof(DirectoryBlock));
        }

        struct DirectoryBlock* newBlock = (struct DirectoryBlock*)&blockMem[indirectBlock->blockPointers[1]];

        for (int i = 0; i < ENTRY_NUMBER; i++) {
            if (strcmp(newBlock->fileName[i], target) == 0) {
                printf("The file already exists or has a same name directory!\n");
                return false;
            }
            if (newBlock->inodeID[i] == -1) {
                entry = i;
                break;
            }
        }

        // Find a unused block
        for (int i = 0; i < BLOCK_NUMBER / 8; i++) {
            for (int j = 0; j < 8; j++) {
                if ((blockBitmap[i] & (1 << j)) == 0) {
                    n_block = i * 8 + j;
                    break;
                }
            }
            if (n_block != -1) {
                break;
            }
        }
        if (n_block == -1) {
            printf("The block is full!\n");
            return false;
        }

        // Find a unused inode
        flag = 0;
        for (int i = 0; i < INODE_NUMBER; i++) {
            if (inodeMem[i].blockID == -1) {
                flag = 1;
                inodeMem[i].groupPermission = groupPermission;
                inodeMem[i].ownerPermission = ownerPermission;
                inodeMem[i].otherPermission = otherPermission;
                inodeMem[i].blockID = n_block;
                inodeMem[i].fileType = 0;
                inodeMem[i].groupID = groupID;
                inodeMem[i].userID = userID;
                n_inode = i;
                break;
            }
        }
        if (n_inode == -1) {
            printf("The inode is full!\n");
            return false;
        }
        newBlock->inodeID[entry] = n_inode;
        strcpy(newBlock->fileName[entry], target);
        blockBitmap[n_block / 8] |= 1 << (n_block % 8);
        struct DirectoryBlock* new_block = (struct DirectoryBlock*)&blockMem[n_block];
        for (i = 0; i < ENTRY_NUMBER; i++) {
            block->inodeID[i] = -1;
            memset(block->fileName[i], '\0', sizeof(block->fileName[i]));
        }
        printf("File created.\n");
        return true;
    }
    return true;
}

void OperatingSystem::deleteAllFileInDirectory(struct DirectoryBlock* block)
{
    for (int i = 0; i < ENTRY_NUMBER; i++)
    {
        if (block->inodeID[i] != -1)
        {
            int inodeToDelete = block->inodeID[i];
            pushBlockAndInode( inodeMem[inodeToDelete].blockID, inodeToDelete);

            if (inodeMem[inodeToDelete].fileType == 0)
            {
                struct DirectoryBlock* subDirBlock = (struct DirectoryBlock*)&blockMem[inodeMem[inodeToDelete].blockID];

                // Delete Directory.
                deleteAllFileInDirectory(subDirBlock);

                // Clear the directory entry and free the inode and block.
                memset(block->fileName[i], '\0', sizeof(block->fileName[i]));
                blockBitmap[inodeMem[inodeToDelete].blockID / 8] &= ~(1 << (inodeMem[inodeToDelete].blockID % 8));
                inodeMem[inodeToDelete].blockID = -1;
                inodeMem[inodeToDelete].blockID = -1;
                inodeMem[inodeToDelete].fileType = -1;
                inodeMem[inodeToDelete].groupID = -1;
                inodeMem[inodeToDelete].userID = -1;
                inodeMem[inodeToDelete].groupPermission = 0;
                inodeMem[inodeToDelete].otherPermission = 0;
                inodeMem[inodeToDelete].ownerPermission = 0;
                block->inodeID[i] = -1;
            }
            else
            {
                struct FileBlock* fileBlock = (struct FileBlock*)&blockMem[inodeMem[inodeToDelete].blockID];
                // Clear the file and free the inode and block.
                memset(block->fileName[i], '\0', sizeof(block->fileName[i]));
                blockBitmap[inodeMem[inodeToDelete].blockID / 8] &= ~(1 << (inodeMem[inodeToDelete].blockID % 8));
                inodeMem[inodeToDelete].blockID = -1;
                inodeMem[inodeToDelete].fileType = -1;
                inodeMem[inodeToDelete].groupID = -1;
                inodeMem[inodeToDelete].userID = -1;
                inodeMem[inodeToDelete].groupPermission = 0;
                inodeMem[inodeToDelete].otherPermission = 0;
                inodeMem[inodeToDelete].ownerPermission = 0;
                block->inodeID[i] = -1;
            }
        }
    }
}

bool OperatingSystem::deleteDirectory(const int execUserID, string path) {
    char* directory, * parent, * target;
    const char delimiter[2] = "/";
    struct Inode* pointer = this->inodeMem;
    struct DirectoryBlock* block;
    char paths[100];
    strcpy(paths, path.c_str());
    parent = NULL;
    directory = strtok(paths, delimiter);
    if (directory == NULL)   printf("The root directory can't be deleted.");
    while (directory != NULL) {
        if (parent != NULL) {
            block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
            int flag = 0;
            for (int i = 0; i < ENTRY_NUMBER; i++) {
                if (strcmp(block->fileName[i], parent) == 0) {
                    if (!checkWritePermission(execUserID, block->inodeID[i])) {
                        printf("Permission denied;");
                        return false;
                    }
                    flag = 1;
                    pointer = &inodeMem[block->inodeID[i]];
                    break;
                }
            }
            if (flag == 0 || pointer->fileType == 1) {
                printf("The path does not exist or is not a directory.\n");
                return false;
            }
        }
        parent = directory;
        directory = strtok(NULL, delimiter);
    }

    pushBlockAndInode(pointer->blockID, pointer->inodeNumber);

    target = parent;
    block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
    int i;
    for (i = 0; i < ENTRY_NUMBER; i++) {
        if (strcmp(block->fileName[i], target) == 0) {
            int inodeToDelete = block->inodeID[i];
            pushBlockAndInode( inodeMem[inodeToDelete].blockID, inodeToDelete);

            // Check if the directory is empty.
            struct DirectoryBlock* subDirBlock = (struct DirectoryBlock*)&blockMem[inodeMem[inodeToDelete].blockID];

            // Clear file in directory.
            deleteAllFileInDirectory(subDirBlock);

            // Clear the directory entry and free the inode and block.
            memset(block->fileName[i], '\0', sizeof(block->fileName[i]));
            blockBitmap[inodeMem[inodeToDelete].blockID / 8] &= ~(1 << (inodeMem[inodeToDelete].blockID % 8));
            inodeMem[inodeToDelete].blockID = -1;
            inodeMem[inodeToDelete].fileType = -1;
            inodeMem[inodeToDelete].groupID = -1;
            inodeMem[inodeToDelete].userID = -1;
            inodeMem[inodeToDelete].groupPermission = 0;
            inodeMem[inodeToDelete].otherPermission = 0;
            inodeMem[inodeToDelete].ownerPermission = 0;
            block->inodeID[i] = -1;
            printf("Directory deleted.\n");
            return true;
        }
    }
    printf("Directory not found.\n");
    return false;
}

vector<DirectoryBlock> OperatingSystem::listFiles(const int execUserID, string path) {
    int flag;
    char* directory, * parent, * target;
    const char delimiter[2] = "/";
    struct Inode* pointer = this->inodeMem;
    struct DirectoryBlock* block;
    vector<DirectoryBlock>blocks;
    char paths[100];
    strcpy(paths, path.c_str());
    parent = NULL;
    printf("The directory includes the following files:\n");

    directory = strtok(paths, delimiter);
    while (directory != NULL) {
        block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
        flag = 0;
        for (int i = 0; i < ENTRY_NUMBER; i++) {
            if (strcmp(block->fileName[i], directory) == 0) {
                flag = 1;
                pointer = &inodeMem[block->inodeID[i]];
                break;
            }
        }
        if (flag == 0 || pointer->fileType == 1) {
            // If the parent directory does not exist or is not a directory file
            printf("The path does not exist!\n");
            DirectoryBlock null;
            return blocks;
        }
        directory = strtok(NULL, delimiter);
    }
    printf("INode\tisDir\tFile Name\tuserID\tgroupID\townerPermission\tgroupPermission\totherPermission\n");

    if (pointer->fileType == 0)
    {
        block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
        for (int i = 0; i < ENTRY_NUMBER; i++) {
            if (block->inodeID[i] != -1) {
                printf("%-6d\t%-5d\t%-15s\t%-5d\t%-6d\t",
                    block->inodeID[i],
                    1 - inodeMem[block->inodeID[i]].fileType,
                    block->fileName[i],
                    inodeMem[block->inodeID[i]].userID,
                    inodeMem[block->inodeID[i]].groupID);

                // Print binary representation for char values
                printf(" ");
                printBinary(inodeMem[block->inodeID[i]].ownerPermission);
                printf("\t");
                printBinary(inodeMem[block->inodeID[i]].groupPermission);
                printf("\t");
                printBinary(inodeMem[block->inodeID[i]].otherPermission);
                printf("\n");
            }
        }
        blocks.push_back(*block);
    }
    if (pointer->fileType == 2)
    {
        struct IndexBlock* indirectBlock = (struct IndexBlock*)&blockMem[pointer->blockID];
        for (int j = 0; j < 2; j++)
        {
            block = (struct DirectoryBlock*)&blockMem[indirectBlock->blockPointers[j]];
            for (int i = 0; i < ENTRY_NUMBER; i++) {
                if (block->inodeID[i] != -1) {
                    printf("%-6d\t%-5d\t%-15s\t%-5d\t%-6d\t",
                        block->inodeID[i],
                        1 - inodeMem[block->inodeID[i]].fileType,
                        block->fileName[i],
                        inodeMem[block->inodeID[i]].userID,
                        inodeMem[block->inodeID[i]].groupID);

                    // Print binary representation for char values
                    printf(" ");
                    printBinary(inodeMem[block->inodeID[i]].ownerPermission);
                    printf("\t");
                    printBinary(inodeMem[block->inodeID[i]].groupPermission);
                    printf("\t");
                    printBinary(inodeMem[block->inodeID[i]].otherPermission);
                    printf("\n");
                }
            }
            blocks.push_back(*block);
        }
    }
    return blocks;
}

void OperatingSystem::printBinary(char ch) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (ch & (1 << i)) ? 1 : 0);
    }
}

bool OperatingSystem::createFile(const int execUserID, string path, const int userID, const int groupID, const char ownerPermission, const char groupPermission, const char otherPermission) {
    if (userBlock->users[execUserID].userID == -1) {
        printf("The user does not exist \n");
        return false;
    }
    if (userBlock->users[userID].userID == -1 || userID < 0 || userID >= MAX_USER_NUMBER) {
        printf("The user does not exist \n");
        return false;
    }
    if (groupBlock->groups[groupID].groupID == -1 || groupID < 0 || groupID >= MAX_GROUP_NUMBER) {
        printf("The group does not exist \n");
        return false;
    }
    if (ownerPermission >= 255 | ownerPermission < 0 | groupPermission >= 255 | groupPermission < 0 | otherPermission >= 255 | otherPermission < 0) {
        printf("The permission can not be inputted like this \n");
        return false;
    }
    int flag;
    char* directory, * parent, * target;
    const char delimiter[2] = "/";
    struct Inode* pointer = this->inodeMem;
    struct DirectoryBlock* block;
    char paths[100];
    strcpy(paths, path.c_str());
    parent = NULL;
    directory = strtok(paths, delimiter);
    while (directory != nullptr) {
        if (parent != NULL) {
            flag = 0;
            block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
            for (int i = 0; i < ENTRY_NUMBER; i++) {
                if (strcmp(block->fileName[i], parent) == 0) {
                    if (!checkWritePermission(execUserID, block->inodeID[i])) {
                        printf("Permission denied; \n");
                        return false;
                    }
                    flag = 1;
                    pointer = &inodeMem[block->inodeID[i]];
                    break;
                }
            }
            if (flag == 0 || pointer->fileType == 1) {
                printf("Path isn't exists. \n");
                return false;
            }
        }
        parent = directory;
        directory = strtok(NULL, delimiter);
    }

    pushBlockAndInode(pointer->blockID, pointer->inodeNumber);

    int entry = -1, n_block = -1, n_inode = -1;
    target = parent;
    block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
    for (int i = 0; i < ENTRY_NUMBER; i++) {
        if (strcmp(block->fileName[i], target) == 0) {
            printf("The file already exists or has a same name directory!\n");
            return false;
        }
        if (block->inodeID[i] == -1) {
            entry = i;
            break;
        }
    }
    if (entry >= 0) {
        // Find a unused block
        for (int i = 0; i < BLOCK_NUMBER / 8; i++) {
            for (int j = 0; j < 8; j++) {
                if ((blockBitmap[i] & (1 << j)) == 0) {
                    n_block = i * 8 + j;
                    break;
                }
            }
            if (n_block != -1) {
                break;
            }
        }
        if (n_block == -1) {
            printf("The block is full!\n");
            return false;
        }

        // Find a unused inode
        flag = 0;
        for (int i = 0; i < INODE_NUMBER; i++) {
            if (inodeMem[i].blockID == -1) {
                flag = 1;
                n_inode = i;
                break;
            }
        }
        if (n_inode == -1) {
            printf("The inode is full!\n");
            return false;
        }

        pushBlockAndInode(n_block, n_inode);

        // Initialize the new normal file
        inodeMem[n_inode].groupPermission = groupPermission;
        inodeMem[n_inode].ownerPermission = ownerPermission;
        inodeMem[n_inode].otherPermission = otherPermission;
        inodeMem[n_inode].blockID = n_block;
        inodeMem[n_inode].fileType = 1;
        inodeMem[n_inode].groupID = groupID;
        inodeMem[n_inode].userID = userID;

        block->inodeID[entry] = n_inode;
        strcpy(block->fileName[entry], target);
        blockBitmap[n_block / 8] |= 1 << (n_block % 8);
        struct FileBlock* new_block = (struct FileBlock*)&blockMem[n_block];
        memset(new_block->content, '\0', sizeof(new_block->content));
        printf("File created.\n");
    }
    else {
        struct IndexBlock* indirectBlock = (struct IndexBlock*)&blockMem[pointer->blockID];
        if (pointer->fileType != 2)
        {
            for (int i = 0; i < INDEX_SIZE; ++i)
            {
                indirectBlock->blockPointers[i] = -1;
            }
            pointer->fileType = 2;

            for (int i = 0; i < 2; ++i)
            {
                int blockID = findBlock();
                if (blockID == -1) {
                    printf("Failed to allocate block.\n");
                    return false;
                }
                else
                {
                    blockBitmap[blockID / 8] |= 1 << (blockID % 8);
                }
                indirectBlock->blockPointers[i] = blockID;
            }

            // 复制初始目录块到 blockPointers[0]
            memcpy(&blockMem[indirectBlock->blockPointers[0]], block, sizeof(DirectoryBlock));
        }

        struct DirectoryBlock* newBlock = (struct DirectoryBlock*)&blockMem[indirectBlock->blockPointers[1]];

        for (int i = 0; i < ENTRY_NUMBER; i++) {
            if (strcmp(newBlock->fileName[i], target) == 0) {
                printf("The file already exists or has a same name directory!\n");
                return false;
            }
            if (newBlock->inodeID[i] == -1) {
                entry = i;
                break;
            }
        }

        // Find a unused block
        for (int i = 0; i < BLOCK_NUMBER / 8; i++) {
            for (int j = 0; j < 8; j++) {
                if ((blockBitmap[i] & (1 << j)) == 0) {
                    n_block = i * 8 + j;
                    break;
                }
            }
            if (n_block != -1) {
                break;
            }
        }
        if (n_block == -1) {
            printf("The block is full!\n");
            return false;
        }

        // Find a unused inode
        flag = 0;
        for (int i = 0; i < INODE_NUMBER; i++) {
            if (inodeMem[i].blockID == -1) {
                flag = 1;
                inodeMem[i].groupPermission = groupPermission;
                inodeMem[i].ownerPermission = ownerPermission;
                inodeMem[i].otherPermission = otherPermission;
                inodeMem[i].blockID = n_block;
                inodeMem[i].fileType = 1;
                inodeMem[i].groupID = groupID;
                inodeMem[i].userID = userID;
                n_inode = i;
                break;
            }
        }
        if (n_inode == -1) {
            printf("The inode is full!\n");
            return false;
        }
        newBlock->inodeID[entry] = n_inode;
        strcpy(newBlock->fileName[entry], target);
        blockBitmap[n_block / 8] |= 1 << (n_block % 8);
        struct FileBlock* new_block = (struct FileBlock*)&blockMem[n_block];
        memset(new_block->content, '\0', sizeof(new_block->content));
        printf("File created.\n");
        return true;
    }
    return true;
}

bool OperatingSystem::deleteFile(const int execUserID, string path) {
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory, * parent, * target;
    const char delimiter[2] = "/";
    struct Inode* pointer = inodeMem;
    struct DirectoryBlock* block;
    parent = NULL;
    directory = strtok(paths, delimiter);
    if (directory == NULL) {
        printf("You cannot delete the root directory!\n");
        return false;
    }
    while (directory != NULL) {
        if (parent != NULL) {
            block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
            int flag = 0;
            int i;
            for (i = 0; i < ENTRY_NUMBER; i++) {
                if (strcmp(block->fileName[i], parent) == 0) {
                    if (!checkWritePermission(execUserID, block->inodeID[i])) {
                        printf("Permission denied;");
                        return false;
                    }
                    flag = 1;
                    pointer = &inodeMem[block->inodeID[i]];
                    break;
                }
            }
            if (flag == 0 || pointer->fileType == 1) {
                printf("The path does not exist.\n");
                return false;
            }
        }
        parent = directory;
        directory = strtok(NULL, delimiter);
    }

    target = parent;
    // Now, you have located the file to be deleted.
    pushBlockAndInode(pointer->blockID, pointer->inodeNumber);
    block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
    int i, j;
    if (pointer->fileType != 2)
    {
        for (i = 0; i < ENTRY_NUMBER; i++) {
            if (strcmp(block->fileName[i], target) == 0) {
                int inodeToDelete = block->inodeID[i];

                pushBlockAndInode(inodeMem[inodeToDelete].blockID, inodeToDelete);
                
                if (inodeMem[inodeToDelete].fileType == 0)
                {
                    printf("Can not delete a directory.\n");
                    return false;
                }

                // Clear the file and free the inode and block.
                block->inodeID[i] = -1;
                memset(block->fileName[i], '\0', sizeof(block->fileName[i]));

                if (inodeMem[inodeToDelete].fileType == 2)
                {
                    struct IndexBlock* indirectBlock = (struct IndexBlock*)&blockMem[inodeMem[inodeToDelete].blockID];
                    for (i = 0; i < INDEX_SIZE; i++)
                    {
                        if (indirectBlock->blockPointers[i] != -1)
                        {
                            int blockToDelete = indirectBlock->blockPointers[i];

                            pushBlockAndInode(blockToDelete, inodeToDelete);

                            blockBitmap[blockToDelete / 8] &= ~(1 << (blockToDelete % 8));
                            indirectBlock->blockPointers[i] = -1;
                        }
                    }

                }
                inodeMem[inodeToDelete].fileType = 0;
                inodeMem[inodeToDelete].groupID = -1;
                inodeMem[inodeToDelete].userID = -1;
                inodeMem[inodeToDelete].groupPermission = 0;
                inodeMem[inodeToDelete].otherPermission = 0;
                inodeMem[inodeToDelete].ownerPermission = 0;
                blockBitmap[inodeMem[inodeToDelete].blockID / 8] &= ~(1 << (inodeMem[inodeToDelete].blockID % 8));
                inodeMem[inodeToDelete].blockID = -1;
                printf("File deleted.\n");
                return true;
            }
        }
    }
    else
    {
        struct IndexBlock* indirectBlock = (struct IndexBlock*)&blockMem[pointer->blockID];
        for (int j = 0; j < 2; j++)
        {
            block = (struct DirectoryBlock*)&blockMem[indirectBlock->blockPointers[j]];
            for (int i = 0; i < ENTRY_NUMBER; i++) {
                if (strcmp(block->fileName[i], target) == 0)
                {
                    int inodeToDelete = block->inodeID[i];
                    if (inodeMem[inodeToDelete].fileType == 0)
                    {
                        printf("Can not delete a directory.\n");
                        return false;
                    }

                    // Clear the file and free the inode and block.
                    block->inodeID[i] = -1;
                    memset(block->fileName[i], '\0', sizeof(block->fileName[i]));

                    if (inodeMem[inodeToDelete].fileType == 2)
                    {
                        struct IndexBlock* indirectBlock = (struct IndexBlock*)&blockMem[inodeMem[inodeToDelete].blockID];
                        for (i = 0; i < INDEX_SIZE; i++)
                        {
                            if (indirectBlock->blockPointers[i] != -1)
                            {
                                int blockToDelete = indirectBlock->blockPointers[i];
                                blockBitmap[blockToDelete / 8] &= ~(1 << (blockToDelete % 8));
                                indirectBlock->blockPointers[i] = -1;
                            }
                        }

                    }
                    inodeMem[inodeToDelete].fileType = 0;
                    inodeMem[inodeToDelete].groupID = -1;
                    inodeMem[inodeToDelete].userID = -1;
                    inodeMem[inodeToDelete].groupPermission = 0;
                    inodeMem[inodeToDelete].otherPermission = 0;
                    inodeMem[inodeToDelete].ownerPermission = 0;
                    blockBitmap[inodeMem[inodeToDelete].blockID / 8] &= ~(1 << (inodeMem[inodeToDelete].blockID % 8));
                    inodeMem[inodeToDelete].blockID = -1;
                    printf("File deleted.\n");
                    return true;
                }
            }
        }
    }
    printf("File not found.\n");
    return false;
}

//��������������Ϊ�ڲ����Ա�����ָ���ʱ���õģ��������ô�
Inode OperatingSystem::getinode() {
    return OperatingSystem::inodeMem[2];
}

GroupBlock OperatingSystem::getblock() {
    struct GroupBlock* block = (struct GroupBlock*)&OperatingSystem::blockMem[2];
    GroupBlock thisblock = *block;
    return thisblock;
}

char OperatingSystem::blockbit() {
    return OperatingSystem::blockBitmap[0];
}

void OperatingSystem::listBlock()
{
    for (int i = 0; i < BLOCK_NUMBER / 8; i++)
    {
        for (int j = 0; j < 8; j++) 
        {
            if ((blockBitmap[i] & (1 << j)) != 0)
            {
                int n_block = i * 8 + j;
                cout << n_block << endl;
            }
        }
    }
}

int OperatingSystem::findBlock()
{
    for (int i = 0; i < BLOCK_NUMBER / 8; i++) {
        for (int j = 0; j < 8; j++) {
            if ((blockBitmap[i] & (1 << j)) == 0) {
                blockBitmap[i] |= (1 << j);
                return i * 8 + j;
            }
        }
    }
    return -1;
}

int OperatingSystem::findInode()
{
    int n_inode = -1;
    for (int i = 0; i < INODE_NUMBER; i++)
    {
        if (inodeMem[i].blockID == -1)
        {
            n_inode = i;
            break;
        }
    }
    return n_inode;
}

// ��path�¶��ļ�
vector<FileBlock> OperatingSystem::readFile(const int execUserID, string path) {
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory;
    const char delimiter[2] = "/";
    directory = strtok(paths, delimiter);
    vector<FileBlock>fileBlocks;
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return fileBlocks;
    }
    int inodeToRead = getReadInodeID(execUserID, path);
    if (inodeToRead == -1) {
        printf("The path is wrong");
        return fileBlocks;
    }
    if (!checkReadPermission(execUserID, inodeToRead)) {
        printf("You don not have read permission.\n");
        return fileBlocks;
    }

    struct Inode inode = inodeMem[inodeToRead];

    if (inode.fileType == 1)
    {
        struct FileBlock fileBlock = (struct FileBlock)blockMem[inodeMem[inodeToRead].blockID];
        fileBlocks.push_back(fileBlock);
        printf(fileBlock.content);
        printf("\n");
    }
    else if (inode.fileType == 2)
    {
        struct IndexBlock* indirectBlock = (struct IndexBlock*)&blockMem[inodeMem[inodeToRead].blockID];
        for (int i = 0; i < INDEX_SIZE; ++i)
        {
            if (indirectBlock->blockPointers[i] != -1)
            {
                struct FileBlock fileBlock = (struct FileBlock)blockMem[indirectBlock->blockPointers[i]];
                fileBlocks.push_back(fileBlock);
                printf(fileBlock.content);
            }
        }
        printf("\n");
    }
    return fileBlocks;
}
// ��path��д�ļ����ļ������Ժ���ν��---------------�ųڣ�

bool OperatingSystem::writeFile(const int execUserID, string path, string content) {
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory;
    const char delimiter[2] = "/";
    directory = strtok(paths, delimiter);
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return false;
    }
    int inodeToWrite = getWriteInodeID(execUserID, path);
    if (inodeToWrite == -1) {
        printf("The path is wrong");
        return false;
    }
    if (!checkWritePermission(execUserID, inodeToWrite)) {
        printf("You don not have read permission.\n");
        return false;
    }
    
    pushBlockAndInode(inodeMem[inodeToWrite].inodeNumber, inodeToWrite);

    struct Inode* inode = &inodeMem[inodeToWrite];

    if (inode->fileType == 2)
    {
        struct IndexBlock* indirectBlock = (struct IndexBlock*)&blockMem[inodeMem[inodeToWrite].blockID];
        for (int i = 0; i < INDEX_SIZE; i++)
        {
            int blockID = indirectBlock->blockPointers[i];
            if (blockID == -1) continue;

            pushBlockAndInode(blockID, inodeToWrite);

            blockBitmap[blockID / 8] &= ~(1 << (blockID % 8));
        }
        inode->fileType = 1;
        memset(indirectBlock, '\0', sizeof(FileBlock));
    }

    // 计算需要的块数
    int totalBlocksNeeded = content.length() / (BLOCK_SIZE -1) + 1;

    if (totalBlocksNeeded == 1)
    {
        struct FileBlock* fileBlock = (struct FileBlock*)&blockMem[inodeMem[inodeToWrite].blockID];
        strcpy(fileBlock->content, content.c_str());
        return true;
    }
    else
    {
        inode->fileType = 2; // 表示其是一级索引块
        struct IndexBlock* indirectBlock = (struct IndexBlock*)&blockMem[inodeMem[inodeToWrite].blockID];
        
        for (int i = 0; i < INDEX_SIZE; ++i)
        {
            indirectBlock->blockPointers[i] = -1;
        }
        
        for (int i = 0; i < INDEX_SIZE && i < totalBlocksNeeded; ++i)
        {
            int blockID = findBlock();

            if (blockID == -1) {
                printf("Failed to allocate block.\n");
                return false;
            }
            else
            {
                pushBlockAndInode(blockID, inodeToWrite);
                blockBitmap[blockID / 8] |= 1 << (blockID % 8);
            }
            indirectBlock->blockPointers[i] = blockID;

            // 复制数据到块的内容
            int bytesToCopy = 0;
            if (BLOCK_SIZE > content.length())
            {
                bytesToCopy = content.length();
            }
            else
            {
                bytesToCopy = BLOCK_SIZE - 1;
            }
            struct FileBlock* fileBlock = (struct FileBlock*)&blockMem[blockID];
            memcpy(fileBlock->content, content.c_str(), bytesToCopy);
            content = content.substr(bytesToCopy);
        }
        return true;
    }
}

// ��path��д�ļ�(��ȡһ���飩
bool OperatingSystem::writeFile(const int execUserID, string path, FileBlock newblock) {
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory;
    const char delimiter[2] = "/";
    directory = strtok(paths, delimiter);
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return false;
    }
    int inodeToWrite = getWriteInodeID(execUserID,path);

    pushBlockAndInode(inodeMem[inodeToWrite].blockID, inodeToWrite);

    if (inodeToWrite == -1) {
        printf("The path is wrong or you don not have the permission");
        return false;
    }
    if (!checkWritePermission(execUserID, inodeToWrite)) {
        printf("You don not have read permission.\n");
        return false;
    }
    blockMem[inodeMem[inodeToWrite].blockID] = newblock;
    return true;
}
// �ж�path�Ƿ���ڣ���������ڷ���-1�����ļ��з���0���ļ�����1��(�о��˴��ô����Ǻܴ��Ȳ�д��

int OperatingSystem::checkPath(const int execUserID, string path) {
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory;
    const char delimiter[2] = "/";
    directory = strtok(paths, delimiter);
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return 0;
    }
    int inodeToRead = getReadInodeID(execUserID, path);
    if (inodeToRead == -1) {
        printf("The path is wrong");
        return -1;
    }
    if (!checkReadPermission(execUserID, inodeToRead)) {
        printf("You don not have read permission.\n");
        return -1;
    }
    if (inodeMem[inodeToRead].fileType == 0) return 0;
    if (inodeMem[inodeToRead].fileType == 1 || inodeMem[inodeToRead].fileType == 2) return inodeMem[inodeToRead].fileType;
        else return -1;

}

// Ȩ�޹���
// �޸�path�µ��ļ����ļ��е�permission
bool OperatingSystem::modifyPermission(const int execUserID, string path, const char ownerPermission, const char groupPermission, const char otherPermission) {
    //�ж��Ƿ�Ϊ����Ա
    if (ownerPermission >= 255 | ownerPermission < 0 | groupPermission >= 255 | groupPermission < 0 | otherPermission >= 255 | otherPermission < 0) {
        printf("The permission can not be inputted like this \n");
        return false;
    }

    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to create a user." << endl;
        return -1;
    }
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory;
    const char delimiter[2] = "/";
    directory = strtok(paths, delimiter);
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return false;
    }
    if (getWriteInodeID(execUserID, path) == -1) {
        printf("The path is wrong\n");
        return false;
    }
    int inodeToDo = getWriteInodeID(execUserID,path);

    pushBlockAndInode(inodeMem[inodeToDo].blockID, inodeToDo);

    inodeMem[inodeToDo].ownerPermission = ownerPermission;
    inodeMem[inodeToDo].groupPermission = groupPermission;
    inodeMem[inodeToDo].otherPermission = otherPermission;
    return true;
}
// �޸�path�µ��ļ����ļ��е�������

bool OperatingSystem::modifyOwner(const int execUserID, string path, const int userID) {
    //�жϹ���Աid�Ƿ����
    if (userBlock->users[execUserID].userID == -1 || execUserID < 0 || execUserID >= MAX_USER_NUMBER) {
        printf("The user does not exist \n");
        return false;
    }
    // //�ж�����userID�Ƿ����
    if (userBlock->users[userID].userID == -1 || userID < 0 || userID >= MAX_USER_NUMBER) {
        printf("The user does not exist \n");
        return false;
    }
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to create a user." << endl;
        return -1;
    }
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory;
    const char delimiter[2] = "/";
    directory = strtok(paths, delimiter);
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return false;
    }
    if (getWriteInodeID(execUserID, path) == -1) {
        printf("The path is wrong\n");
        return false;
    }
    int inodeToDo = getWriteInodeID(execUserID,path);
    pushBlockAndInode(inodeMem[inodeToDo].blockID, inodeToDo);
    inodeMem[inodeToDo].userID = userID;
    return true;
}

// �޸�path�µ��ļ����ļ��е�������
bool OperatingSystem::modifyGroup(const int execUserID, string path, const int groupID) {
    //�жϹ���ԱID�Ƿ����
    if (userBlock->users[execUserID].userID == -1 || execUserID < 0 || execUserID >= MAX_USER_NUMBER) {
        printf("The user does not exist \n");
        return false;
    }
    //�ж�������groupID�Ƿ����
    if (groupBlock->groups[groupID].groupID == -1 || groupID < 0 || groupID >= MAX_GROUP_NUMBER) {
        printf("The group does not exist \n");
        return false;
    }
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to create a user." << endl;
        return -1;
    }
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory;
    const char delimiter[2] = "/";
    directory = strtok(paths, delimiter);
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return false;
    }
    if (getWriteInodeID(execUserID, path) == -1) {
        printf("The path is wrong\n");
        return false;
    }
    int inodeToDo = getWriteInodeID(execUserID,path);
    pushBlockAndInode(inodeMem[inodeToDo].blockID, inodeToDo);
    inodeMem[inodeToDo].groupID = groupID;
    return true;
}
// ��ȡpath�µ��ļ���Ӧ��inode(����

int OperatingSystem::getReadInodeID(const int execUserID,string path) {
    int i, j, flag;
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory, * parent, * target;
    const char delimiter[2] = "/";
    struct Inode* pointer = inodeMem;
    struct DirectoryBlock* block;

    // Access the pointer of parent directory recursively
    parent = NULL;
    directory = strtok(paths, delimiter);
   /* if (directory == NULL) {
        printf("It is the root directory!\n");
        return false;
    }*/
    while (directory != NULL) {
        if (parent != NULL) {
            block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
            flag = 0;
            for (i = 0; i < ENTRY_NUMBER; i++) {
                if (strcmp(block->fileName[i], parent) == 0) {
                    if (!checkReadPermission(execUserID, block->inodeID[i])) {
                        printf("Permission denied;\n");
                        return -1;
                    }
                    flag = 1;
                    pointer = &inodeMem[block->inodeID[i]];
                    break;
                }
            }
            if (flag == 0 || pointer->fileType == 1) {
                // If the parent directory does not exist or is not a directory file
                printf("The path does not exist!\n");
                return -1;
            }
        }
        parent = directory;
        directory = strtok(NULL, delimiter);
    }

    // Read the target directory
    target = parent;
    block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
    //0 is root,1 is UserBlock,2 is GroupBlock,3 is PasswordBlock
    for (i = 0; i < ENTRY_NUMBER; i++) {
        if (strcmp(block->fileName[i], target) == 0) {
            int inodeToDo = block->inodeID[i];
            return inodeToDo;
        }
    }
    return -1;
}
// ��ȡpath�µ��ļ���Ӧ��inode��д��

int OperatingSystem::getWriteInodeID(const int execUserID, string path) {
    int i, j, flag;
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory, * parent, * target;
    const char delimiter[2] = "/";
    struct Inode* pointer = inodeMem;
    struct DirectoryBlock* block;

    // Access the pointer of parent directory recursively
    parent = NULL;
    directory = strtok(paths, delimiter);
    /* if (directory == NULL) {
         printf("It is the root directory!\n");
         return false;
     }*/
    while (directory != NULL) {
        if (parent != NULL) {
            block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
            flag = 0;
            for (i = 0; i < ENTRY_NUMBER; i++) {
                if (strcmp(block->fileName[i], parent) == 0) {
                    if (!checkWritePermission(execUserID, block->inodeID[i])) {
                        printf("Permission denied;\n");
                        return -1;
                    }
                    flag = 1;
                    pointer = &inodeMem[block->inodeID[i]];
                    break;
                }
            }
            if (flag == 0 || pointer->fileType == 1) {
                // If the parent directory does not exist or is not a directory file
                printf("The path does not exist!\n");
                return -1;
            }
        }
        parent = directory;
        directory = strtok(NULL, delimiter);
    }

    // Read the target directory
    target = parent;
    block = (struct DirectoryBlock*)&blockMem[pointer->blockID];
    //0 is root,1 is UserBlock,2 is GroupBlock,3 is PasswordBlock
    for (i = 0; i < ENTRY_NUMBER; i++) {
        if (strcmp(block->fileName[i], target) == 0) {
            int inodeToDo = block->inodeID[i];
            return inodeToDo;
        }
    }
    return -1;
}
// ��ȡpath�µ��ļ����ļ��е�������

int OperatingSystem::getOwner(const int execUserID, string path) {
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory;
    const char delimiter[2] = "/";
    directory = strtok(paths, delimiter);
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return -1;
    }
    int inodeToDo = getReadInodeID(execUserID,path);
    if (inodeToDo == -1) {
        printf("The path is wrong");
        return -1;
    }
    return inodeMem[inodeToDo].userID;
}

// ��ȡpath�µ��ļ����ļ��е�������(-1��ʾû���ң�
int OperatingSystem::getGroup(const int execUserID, string path) {
    char paths[100];
    strcpy(paths, path.c_str());
    char* directory;
    const char delimiter[2] = "/";
    directory = strtok(paths, delimiter);
    if (directory == NULL) {
        printf("It is the root directory!\n");
        return -1;
    }
    int inodeToDo = getReadInodeID(execUserID,path);
    if (inodeToDo == -1) {
        printf("The path is wrong");
        return -1;
    }
    return inodeMem[inodeToDo].groupID;
}

// �ж�execUser��û�ж�Ȩ��
bool OperatingSystem::checkReadPermission(const int execUserID, const int inodeID) {
    struct Inode pointer = inodeMem[inodeID];
    if (OperatingSystem::userBlock->users[execUserID].groupID == 0) {
        return true;
    }
    if ((execUserID == pointer.userID) & ((pointer.ownerPermission & (1 << 0)) != 0)) {
        return true;
    }
    else if ((checkUserFromGroup(0, execUserID, pointer.groupID)) & ((pointer.groupPermission & (1 << 0)) != 0)) {
        return true;
    }
    else if ((pointer.otherPermission & (1 << 0)) != 0) {
        return true;
    }
    return false;
}
// �ж�execUser��û��дȨ��

bool OperatingSystem::checkWritePermission(const int execUserID, const int inodeID) {
    struct Inode pointer = inodeMem[inodeID];
    if (OperatingSystem::userBlock->users[execUserID].groupID == 0) {
        return true;
    }
    if ((execUserID == pointer.userID) & ((pointer.ownerPermission & (1 << 1)) != 0)) {
        return true;
    }
    else if ((checkUserFromGroup(0, execUserID, pointer.groupID)) & ((pointer.groupPermission & (1 << 1)) != 0)) {
        return true;
    }
    else if ((pointer.otherPermission & (1 << 1)) != 0) {
        return true;
    }
    return false;
}

// �ж�execUser��û��ִ��Ȩ�ޣ��������ûʲô�ã�
bool OperatingSystem::checkExecutePermission(const int execUserID, const int inodeID) {
    struct UserBlock* block;
    block = (struct UserBlock*)&blockMem[1];
    struct Inode pointer = inodeMem[inodeID];
    if ((execUserID == pointer.userID) & ((pointer.ownerPermission & (1 << 2)) == 1)) {
        return true;
    }
    else if ((checkUserFromGroup(0, execUserID, pointer.groupID)) & ((pointer.groupPermission & (1 << 2)) == 1)) {
        return true;
    }
    else if ((pointer.otherPermission & (1 << 2)) == 1) {
        return true;
    }
    return false;
}

// ϵͳ
// ϵͳ����
bool OperatingSystem::backupSystem(const int execUserID, const string version) {
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to create a user." << endl;
        return -1;
    }


    // Create the backup file name
    stringstream backupFileName;
    backupFileName << "version" << version << ".backup";

    // Open the backup file for writing
    ofstream backupFile("../../OSFile/BackUp/" + backupFileName.str(), ios::binary);
    if (!backupFile.is_open()) {
        cout << "Failed to open backup file." << endl;
        return false;
    }

    // Write inodeMem, blockMem, and blockBitmap to the backup file
    backupFile.write(reinterpret_cast<char*>(inodeMem), sizeof(struct Inode) * INODE_NUMBER);
    backupFile.write(reinterpret_cast<char*>(blockMem), sizeof(struct FileBlock) * BLOCK_NUMBER);
    backupFile.write(blockBitmap, BLOCK_NUMBER / 8);

    // Close the backup file
    backupFile.close();

    cout << "System backup successful. Version: " << version << endl;
    return true;
}

// System Recovery
bool OperatingSystem::recoverySystem(const int execUserID, const string version) {
    //�ж��Ƿ�Ϊ����Ա
    if (OperatingSystem::userBlock->users[execUserID].groupID != 0) {
        cout << "You don't have right to create a user." << endl;
        return -1;
    }
    // Create the backup file name
    stringstream backupFileName;
    backupFileName << "version" << version << ".backup";

    // Open the backup file for reading
    ifstream backupFile("../../OSFile/BackUp/" + backupFileName.str(), ios::binary);
    if (!backupFile.is_open()) {
        cout << "Failed to open backup file." << endl;
        return false;
    }

    for (int i = 0; i < BLOCK_NUMBER; i++)
    {
        pushBlockAndInode(i, i);
    }

    // Read data from the backup file to restore the system state
    backupFile.read(reinterpret_cast<char*>(inodeMem), sizeof(struct Inode) * INODE_NUMBER);
    backupFile.read(reinterpret_cast<char*>(blockMem), sizeof(struct FileBlock) * BLOCK_NUMBER);
    backupFile.read(blockBitmap, BLOCK_NUMBER / 8);
    backupFile.close();
    userBlock = (struct UserBlock*)&OperatingSystem::blockMem[1];
    groupBlock = (struct GroupBlock*)&OperatingSystem::blockMem[2];
    passwordBlock = (struct PasswordBlock*)&OperatingSystem::blockMem[3];

    ofstream systemFile("../../OSFile/system.txt", ios::binary);
    if (!systemFile.is_open()) {
        cout << "Failed to open backup file." << endl;
        return false;
    }
    systemFile.write(reinterpret_cast<char*>(inodeMem), sizeof(struct Inode) * INODE_NUMBER);
    systemFile.write(reinterpret_cast<char*>(blockMem), sizeof(struct FileBlock) * BLOCK_NUMBER);
    systemFile.write(blockBitmap, BLOCK_NUMBER / 8);
    // Close the backup file
    systemFile.close();
    cout << "System recovery successful. Restored from version: " << version << endl;
    return true;
}

// System shot
bool OperatingSystem::initializeSnapshot()
{
    int ret = snapshot->init() & systemLog->init();
    return ret;
}

void OperatingSystem::pushBlockAndInode(int blockID, int inodeID)
{
    cout << "blockID: "  << blockID << "; inode ID: " << inodeID << "; ";
    if (blockID < 0 || blockID > BLOCK_NUMBER || inodeID < 0 || inodeID > INODE_NUMBER)
        return;
    
    BlockAndInode tmp;
    tmp.inodeID = inodeID;
    tmp.blockID = blockID;
    tmp.blockbit = ((blockBitmap[blockID / 8] >> (blockID % 8)) & 1);
    cout << "blockbit: " << tmp.blockbit << endl;
    memcpy(&tmp.block, &blockMem[blockID], sizeof(struct FileBlock));
    memcpy(&tmp, &inodeMem[inodeID], sizeof(Inode));

    snapshot->pushcc(tmp);
}

void OperatingSystem::pushTag(string command)
{
    BlockAndInode tmp;
    memset(tmp.block.content, '\0', sizeof(tmp.block.content));
    tmp.inode.blockID = tmp.inode.fileType = tmp.inode.groupID = tmp.inode.groupPermission = tmp.inode.inodeNumber = tmp.inode.otherPermission = tmp.inode.ownerPermission = tmp.inode.userID = 0;
    tmp.blockID = tmp.inodeID = tmp.blockbit = -1;
    
    snapshot->pushcc(tmp);
    snapshot->writeToFile();
    
    cout << "command: " << endl;
    systemLog->push(command);
}

bool OperatingSystem::revoke()
{
    if (snapshot->nowPointer <= 0) return false;
    snapshot->pop();
    while (snapshot->snapshotStack[snapshot->nowPointer].blockbit != -1 && snapshot->snapshotStack[snapshot->nowPointer].blockID != -1 && snapshot->snapshotStack[snapshot->nowPointer].inodeID != -1)
    {
        BlockAndInode* ptop = &snapshot->snapshotStack[snapshot->nowPointer];
        BlockAndInode tmp;
        tmp.inodeID = ptop->inodeID;
        tmp.blockID = ptop->blockID;
        
        // revoke block
        memcpy(&tmp.block, &blockMem[ptop->blockID], sizeof(struct FileBlock));
        memcpy(&blockMem[ptop->blockID], &ptop->block, sizeof(struct FileBlock));
        tmp.blockbit = ((blockBitmap[ptop->blockID / 8] >> (ptop->blockID % 8)) & 1);
        if (ptop->blockbit == 0)
        {
            blockBitmap[ptop->blockID / 8]  &= ~(1 << (ptop->blockID % 8));
        } else
        {
            blockBitmap[ptop->blockID / 8]  |= (1 << (ptop->blockID % 8));
        }
        
        // revoke Inode
        memcpy(&tmp, &inodeMem[ptop->inodeID], sizeof(Inode));
        memcpy(&inodeMem[ptop->inodeID], &(ptop->inode), sizeof(Inode));

        snapshot->pop();
    }
    snapshot->writeToFile();
    systemLog->pop();
    return true;
}

bool OperatingSystem::recover()
{
    if (snapshot->nowPointer >= snapshot->maxPointer) return false;
    snapshot->revokePop();
    while (snapshot->snapshotStack[snapshot->nowPointer].blockbit != -1 && snapshot->snapshotStack[snapshot->nowPointer].blockID != -1 && snapshot->snapshotStack[snapshot->nowPointer].inodeID != -1)
    {
        BlockAndInode* ptop = &snapshot->snapshotStack[snapshot->nowPointer];
        BlockAndInode tmp;
        tmp.inodeID = ptop->inodeID;
        tmp.blockID = ptop->blockID;

        // recover block
        memcpy(&tmp.block, &blockMem[ptop->blockID], sizeof(struct FileBlock));
        memcpy(blockMem[ptop->blockID].content, ptop->block.content, sizeof(struct FileBlock));
        tmp.blockbit = ((blockBitmap[ptop->blockID / 8] >> (ptop->blockID % 8)) & 1);
        if (ptop->blockbit == 0)
        {
            blockBitmap[ptop->blockID / 8]  &= ~(1 << (ptop->blockID % 8));
        } else
        {
            blockBitmap[ptop->blockID / 8]  |= (1 << (ptop->blockID % 8));
        }
        
        // recover Inode
        memcpy(&tmp, &inodeMem[ptop->inodeID], sizeof(Inode));
        memcpy(&inodeMem[ptop->inodeID], &(ptop->inode), sizeof(Inode));

        snapshot->revokePop();
    }
    snapshot->writeToFile();
    return true;
}

void OperatingSystem::printSnapshot()
{
    cout << "Print all snapshot: " << endl;
    int num = snapshot->topPointer;
    for (int i = 0; i <= num; i++)
    {
        cout << snapshot->snapshotStack[i].blockID << endl;
    }
    cout << "nowpointer: " << snapshot->nowPointer << endl;
    cout << "maxpointer: " << snapshot->maxPointer << endl;
}

void OperatingSystem::printLog()
{
    systemLog->printLog();
}

string OperatingSystem::checkLog()
{
    return systemLog->checkLog();
}