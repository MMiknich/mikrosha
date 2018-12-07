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
#include <glob.h>
#include <signal.h>

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

        if (inpArg.find('*') != std::string::npos || inpArg.find('?') != std::string::npos || inpArg.find('~') != std::string::npos)
        {
            glob_t starStruct;
            glob(inpArg.c_str(),GLOB_TILDE,NULL,&starStruct);
            for(unsigned int i=0;i<starStruct.gl_pathc;++i){
                char *inpChar = new char [inpArg.size() + 1];
                std::strcpy(inpChar, std::string(starStruct.gl_pathv[i]).c_str());
                queue.push(inpChar);
            }
        } else {
            char *inpChar = new char [inpArg.size() + 1];
            std::strcpy(inpChar, inpArg.c_str());
            queue.push(inpChar);
        }
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
        //All run() func-s have valgrind problems, but i hope my os now what to do
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
            signal(SIGINT, childSignal);
            if (strcmp(this->argv[0], "pwd") == 0) {
                char buff[FILENAME_MAX];
                std::cout << getcwd(buff, FILENAME_MAX) << '\n';
                return 0;
            } else {
                execvp(this->argv[0], this->argv);
                perror("no such programm");
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
                signal(SIGINT, childSignal);
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
                perror("no such programm");
                _exit(0);
            }
            else //it's the parent code
            {
                this->childPid = pid;
                return 0;
            }
        }
        case NEWOUTPUT_MODE: {
            int pfd;
            pfd = open(fname.c_str(), O_WRONLY | O_CREAT,420);
            if (pfd == -1) {
                perror("");
            }
            pid_t pid = fork();
            if (pid == -1) {
                perror("");
            }
            if (pid == 0) //it's the child code
            {
                signal(SIGINT, childSignal);
                close(1);
                dup(pfd);
                execvp(this->argv[0], this->argv);
                close(pfd);
                perror("no such programm");
                _exit(0);
            }
            else //it's the parent code
            {
                this->childPid = pid;
                return 0;
            }
        }
        case EXISTINGOUTPUT_MODE: {
            int pfd;
            pfd = open(fname.c_str(), O_WRONLY | O_CREAT | O_APPEND,420);
            if (pfd == -1) {
                perror("");
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("");
            }
            if (pid == 0) //it's the child code
            {
                signal(SIGINT, childSignal);
                close(1);
                dup(pfd);
                execvp(this->argv[0], this->argv);
                close(pfd);
                perror("no such programm");
                _exit(0);
            }
            else //it's the parent code
            {
                this->childPid = pid;
                return 0;
            }
        }
    }
    return 0;
}
int ConveyorElement::run(std::string inpfname,std::string outfname, int mode) {
    int pfdOUTPUT;
    int pfdINPUT;
    if (mode == EXISTINGOUTPUT_MODE) {
        pfdOUTPUT = open(outfname.c_str(), O_WRONLY | O_CREAT | O_APPEND, 420);
        pfdINPUT = open(inpfname.c_str(), O_RDONLY);
    }
    else if (mode == NEWOUTPUT_MODE){
        pfdOUTPUT = open(outfname.c_str(), O_WRONLY | O_CREAT, 420);
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
        perror("no such programm");
        _exit(0);
    }
    else //it's the parent code
    {
        this->childPid = pid;
        return 0;
    }
}
int ConveyorElement::intoPipeIO(int I, int O) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("");
    }
    if (pid == 0) //it's the child code
    {
        signal(SIGINT, childSignal);
        if (close(0) == -1 || dup2(I, 0) < 0 || close(I) != 0 || close(1) == -1 || dup2(O, 1) < 0 || close(O) != 0)
        {
            perror(this->argv[0]);
            perror("no such programm");
            exit(1);
        }
        execvp(this->argv[0], this->argv);
        perror("no such programm");
        _exit(0);
    } else //it's the parent code
    {
        this->childPid = pid;
        return 0;
    }
}
int ConveyorElement::intoPipeI_(int I) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("");
    }
    if (pid == 0) //it's the child code
    {
        signal(SIGINT, childSignal);
        if (close(0) == -1 || dup2(I, 0) < 0 || close(I) != 0)
        {
            perror(this->argv[0]);
            perror("no such programm");
            exit(1);
        }
        execvp(this->argv[0], this->argv);
        perror("no such programm");
        _exit(0);
    } else //it's the parent code
    {
        this->childPid = pid;
        return 0;
    }
}
int ConveyorElement::intoPipe_O(int O) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("");
    }
    if (pid == 0) //it's the child code
    {
        signal(SIGINT, childSignal);
        if (close(1) == -1 || dup2(O, 1) < 0 || close(O) != 0)
        {
            perror(this->argv[0]);
            _exit(1);
        }
        execvp(this->argv[0], this->argv);
        perror("no such programm");
        _exit(0);
    } else //it's the parent code
    {
        this->childPid = pid;
        return 0;
    }
}
pid_t ConveyorElement::getPID() {
    return this->childPid;
}

void childSignal(int inp){
    kill(getpid(), SIGKILL);
    return;
}



//TODO: we can return ConveyorElement, not int to make possible .sjknad().ajlksjdal().asmdlk()