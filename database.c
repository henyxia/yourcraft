#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include "database.h"

void checkDatabaseDir()
{
	DIR* dir = opendir("databases");
	if(dir)
		return;
	else if(errno == ENOENT)
	{
		if(mkdir("databases", 0644)==0)
			return;
		else
		{
			perror("Unable to create database directory\n");
			exit(2);
		}
	}
	else
	{
		perror("Unable to access to database directory\n");
		exit(1);
	}

}

void loadDatabase()
{
	checkDatabaseDir();
	//TODO
}
