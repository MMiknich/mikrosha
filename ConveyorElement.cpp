//
// Created by mmiknich on 16.11.18.
//

#include <iostream>
#include <queue>
#include <cstring>
#include <sstream>
#include <zconf.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include "ConveyorElement.h"

ConveyorElement::ConveyorElement(std::string inputLine) {
    std::istringstream inputStream(inputLine);
    std::queue<char *> queue;
    std::string inpArg;
    //long maxArgLen = 0;

    while (inputStream >> inpArg)
    {
        //std::cout << inpArg << '\n';
        //maxArgLen = inpArg.size() > maxArgLen ? inpArg.size() : maxArgLen;
        char *inpChar = new char [inpArg.size() + 1];
        std::strcpy(inpChar,inpArg.c_str());
        queue.push(inpChar);
    }
    unsigned long numofArgv = queue.size();

    //this->maxArgLen = maxArgLen;
    this->numofArgv = numofArgv;
    char **argv = new char *[numofArgv + 1];

    for (int j = 0; j <= numofArgv; j++){
        *(argv + j) = queue.front();
        queue.pop();
    }

    *(argv + numofArgv) = NULL;

    this->argv = argv;
}
ConveyorElement::~ConveyorElement() {
    for (int i = 0; i < this->numofArgv; ++i) {
        delete(*(this->argv + i));
    }
    delete(this->argv);
}
int ConveyorElement::print() {
    for (int i = 0; i < numofArgv; i++) {
        std::cout << "Argument number "<< i <<" is: " << std::string(*(this->argv+i)) <<"\n";
    }
    return 0;
}
int ConveyorElement::run() {
    if(this->numofArgv > 0) {
        if (strcmp(this->argv[0], "cd") == 0) {
            if (this->numofArgv == 1) {
                passwd *userdata = getpwuid(getuid());
                chdir(userdata->pw_dir);
                return 0;
            } else {
                chdir(this->argv[1]);
                return 0;
            }
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("");
        }
        if (pid == 0) //it's the child code
        {
            if (strcmp(this->argv[0], "pwd") == 0) {
                char buff[FILENAME_MAX];
                std::cout << getcwd(buff, FILENAME_MAX) << '\n';
                return 0;
            } else {
                execvp(this->argv[0], this->argv);
                _exit(0);
            }
        } else //it's the parent code
        {
            this->childPid = pid;
            return 0;
        }
    }
    return 0;
}

int ConveyorElement::run(std::string fname, int mode) {
    switch (mode) {
        case INPUT_MODE: {
            int pfd;
            pfd = open(fname.c_str(), O_RDONLY);
            if (pfd == -1) {
                perror("");
            }
            pid_t pid = fork();
            if (pid == -1) {
                perror("");
            }
            if (pid == 0) //it's the child code
            {
                close(0);
                dup(pfd);
                if(strcmp(this->argv[0], "pwd") == 0)
                {
                    char buff[FILENAME_MAX];
                    perror(" ");
                    std::cout << getcwd(buff, FILENAME_MAX);
                }
                else if(strcmp(this->argv[0], "cd") == 0)
                {
                    if (this->numofArgv == 1) {
                        passwd *userdata = getpwuid(getuid());
                        chdir(userdata->pw_dir);
                    } else
                        chdir(this->argv[1]);
                }
                execvp(this->argv[0], this->argv);
                close(pfd);
                _exit(0);
            }
            else //it's the parent code
            {
                this->childPid = pid;
            }
        }
        case NEWOUTPUT_MODE: {
            int pfd;
            pfd = open(fname.c_str(), O_WRONLY | O_CREAT);
            if (pfd == -1) {
                perror("");
            }
            pid_t pid = fork();
            if (pid == -1) {
                perror("");
            }
            if (pid == 0) //it's the child code
            {
                close(1);
                dup(pfd);
                execvp(this->argv[0], this->argv);
                close(pfd);
                _exit(0);
            }
            else //it's the parent code
            {
                this->childPid = pid;
            }
        }
        case EXISTINGOUTPUT_MODE: {
            int pfd;
            pfd = open(fname.c_str(), O_WRONLY | O_CREAT | O_APPEND);
            if (pfd == -1) {
                perror("");
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("");
            }
            if (pid == 0) //it's the child code
            {
                close(1);
                dup(pfd);
                execvp(this->argv[0], this->argv);
                close(pfd);
                _exit(0);
            }
            else //it's the parent code
            {
                this->childPid = pid;
            }
        }
    }
    return 0;
}
int ConveyorElement::run(std::string inpfname,std::string outfname, int mode)
{
    int pfdOUTPUT;
    int pfdINPUT;
    if (mode == EXISTINGOUTPUT_MODE) {
        pfdOUTPUT = open(outfname.c_str(), O_WRONLY | O_CREAT | O_APPEND);
        pfdINPUT = open(inpfname.c_str(), O_RDONLY);
    }
    else if (mode == NEWOUTPUT_MODE){
        pfdOUTPUT = open(outfname.c_str(), O_WRONLY | O_CREAT);
        pfdINPUT = open(inpfname.c_str(), O_RDONLY);
    }
    else {
        perror("Wrong mode");
        return -1;
    }
    if (pfdOUTPUT == -1 || pfdINPUT == -1) {
        perror("");
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("");
    }
    if (pid == 0) //it's the child code
    {
        close(1);
        dup(pfdOUTPUT);
        close(0);
        dup(pfdINPUT);
        execvp(this->argv[0], this->argv);
        close(pfdINPUT);
        close(pfdOUTPUT);
        _exit(0);
    }
    else //it's the parent code
    {
        this->childPid = pid;
        return 0;
    }
}
pid_t ConveyorElement::getPID() {
    return this->childPid;
}

    //TODO: we can return ConveyorElement, not int to make possible .sjknad().ajlksjdal().asmdlk()