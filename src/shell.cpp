#include <iostream>
#include <sstream>
#include <cstring>
#include "shellfunctions.h"
#include "filesystem.h"
#include "bitmap.h"
#include <math.h>

const int MAXCOMMANDS = 8;
const int NUMAVAILABLECOMMANDS = 16;

std::string availableCommands[NUMAVAILABLECOMMANDS] = {
    "quit","format","ls","create","cat","createImage","restoreImage",
    "rm","cp","append","mv","mkdir","cd","pwd","help","chmod"
};

/* Takes usercommand from input and returns number of commands, commands are stored in strArr[] */
int parseCommandString(const std::string &userCommand, std::string strArr[]);
int findCommand(std::string &command);
bool quit();
std::string help();

/* More functions ... */

int main(void) {
	// <DEBUG>
	ShellFunctions f;
	// </DEBUG>

	std::string userCommand, commandArr[MAXCOMMANDS];
	std::string user = "[user@DV1492";    // Change this if you want another user to be displayed
	std::string currentDir = "/";    // current directory, used for output

    bool bRun = true;
    bool a,b;

    do {
		currentDir = f.currentDirName();

    	for(size_t i = 0; i < MAXCOMMANDS; i++)
    	{
			commandArr[i] = "";
    	}
        std::cout << user << " " << currentDir << "]$ ";
        getline(std::cin, userCommand);

        int nrOfCommands = parseCommandString(userCommand, commandArr);
        if (nrOfCommands > 0) {
            std::string s;
            int cIndex = findCommand(commandArr[0]);
            switch(cIndex) {

			case 0: //quit
				bRun = quit();
                break;
            case 1: // format
                f.format();
                break;
            case 2: // ls
                std::cout << "Listing directory" << std::endl;
                std::cout << f.ls(commandArr[1]);
                break;
            case 3: // create
                s = f.create(commandArr[1]);
                if(s == "")
                    std::cout << "Could not create file" << std::endl;
                break;
            case 4: // cat
                std::cout << f.cat(commandArr[1]) << std::endl;
                break;
            case 5: // createImage
                f.createImage(commandArr[1]);
                break;
            case 6: // restoreImage
                f.restoreImage(commandArr[1]);
                break;
            case 7: // rm
            	f.remove(commandArr[1]);
                break;
            case 8: // cp
                f.cp(commandArr[1], commandArr[2]);
                break;
            case 9: // append
                f.append(commandArr[1], commandArr[2]);
                break;
            case 10: // mv
                f.mv(commandArr[1],commandArr[2]);
                break;
            case 11: // mkdir
                s = f.mkdir(commandArr[1]);
            	if(!s.length())
            		std::cout << "Could not create directory" << std::endl;
                break;
            case 12: // cd
                s = f.cd(commandArr[1]);
            	if(!s.length())
            		std::cout << "Directory not found" << std::endl;
                break;
            case 13: // pwd
                std::cout << f.pwd() << std::endl;
                break;
            case 14: // help
                std::cout << help() << std::endl;
                break;
            case 15:
                //chmod
                if(commandArr[2] == "true")
                {
                    a = true;
                }
                else
                {
                    a = false;
                }
                if(commandArr[3] == "true")
                {
                    b = true;
                }
                else
                {
                    b = false;
                }
                s = f.chmod(commandArr[1], a , b);
                std::cout << s << std::endl;
                break;
            default:
                std::cout << "Unknown command: " << commandArr[0] << std::endl;
            }
        }
    } while (bRun == true);

    return 0;
}

int parseCommandString(const std::string &userCommand, std::string strArr[]) {
    std::stringstream ssin(userCommand);
    int counter = 0;
    while (ssin.good() && counter < MAXCOMMANDS) {
        ssin >> strArr[counter];
        counter++;
    }
    if (strArr[0] == "") {
        counter = 0;
    }
    return counter;
}
int findCommand(std::string &command) {
    int index = -1;
    for (int i = 0; i < NUMAVAILABLECOMMANDS && index == -1; ++i) {
        if (command == availableCommands[i]) {
            index = i;
        }
    }
    return index;
}

bool quit() {
	std::cout << "Exiting\n";
	return false;
}

std::string help() {
    std::string helpStr;
    helpStr += "OSD Disk Tool .oO Help Screen Oo.\n";
    helpStr += "-----------------------------------------------------------------------------------\n" ;
    helpStr += "* quit:                             Quit OSD Disk Tool\n";
    helpStr += "* format;                           Formats disk\n";
    helpStr += "* ls     <path>:                    Lists contents of <path>.\n";
    helpStr += "* create <path>:                    Creates a file and stores contents in <path>\n";
    helpStr += "* cat    <path>:                    Dumps contents of <file>.\n";
    helpStr += "* createImage  <real-file>:         Saves disk to <real-file>\n";
    helpStr += "* restoreImage <real-file>:         Reads <real-file> onto disk\n";
    helpStr += "* rm     <file>:                    Removes <file>\n";
    helpStr += "* cp     <source> <destination>:    Copy <source> to <destination>\n";
    helpStr += "* append <source> <destination>:    Appends contents of <source> to <destination>\n";
    helpStr += "* mv     <old-file> <new-file>:     Renames <old-file> to <new-file>\n";
    helpStr += "* mkdir  <directory>:               Creates a new directory called <directory>\n";
    helpStr += "* cd     <directory>:               Changes current working directory to <directory>\n";
    helpStr += "* pwd:                              Get current working directory\n";
    helpStr += "* help:                             Prints this help screen\n";
    helpStr += "* chmod: <directory> <read> <write>: Changes filerights\n";
    return helpStr;
}

/* Insert code for your shell functions and call them from the switch-case */
