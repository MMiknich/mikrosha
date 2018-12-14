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
#include <sys/times.h>
#include <iomanip>

#include "ConveyorElement.h"

ConveyorElement::ConveyorElement(std::string inputLine) {

    std::istringstream inputStream(inputLine);
    std::queue<char *> queue;
    std::string inpArg;

    bool starFlag = false;
    while (inputStream >> inpArg)
    {
        if (inpArg.find('*') != std::string::npos || inpArg.find('?') != std::string::npos || inpArg.find('~') != std::string::npos)
        {
            glob_t starStruct;
            memset(&starStruct, 0, sizeof(starStruct));

            if(glob(inpArg.c_str(),GLOB_TILDE,NULL,&starStruct) != GLOB_NOMATCH) {
                for (unsigned int i = 0; i < starStruct.gl_pathc; ++i) {

                    std::string globArg(starStruct.gl_pathv[i]);
                    char *inpChar = new char[globArg.size() + 1];
                    std::strcpy(inpChar, globArg.c_str());
                    queue.push(inpChar);
                }
            } else{
                char *inpChar = new char[inpArg.size() + 1];
                std::strcpy(inpChar, inpArg.c_str());
                queue.push(inpChar);
            }
            globfree(&starStruct);
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
    if(strcmp(this->argv[0], "time") == 0)
        this->ctm();
}
ConveyorElement::~ConveyorElement() {
    for (int i = 0; i < this->numofArgv; ++i) {
        free(*(this->argv + i));
    }
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
        static struct tms st_cpu;
        static struct tms en_cpu;
        clock_t startTime;
        clock_t endTime;
        times(&st_cpu);
        startTime = clock();
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
                _exit(0);
            } else {
                execvp(this->argv[0], this->argv);
                perror("error");
                _exit(0);
            }
        } else //it's the parent code
        {
            this->childPid = pid;
            wait(0);
            if(this->timeMode == 1)
            {
                endTime = clock();
                times(&en_cpu);
                std::cerr<< "real:\n" << std::fixed << std::setprecision(6) << (endTime - startTime)/CLOCKS_PER_SEC << "\n";
                std::cerr<< "user:\n" << en_cpu.tms_utime - st_cpu.tms_utime << "\n";
                std::cerr << "sys:\n" << en_cpu.tms_stime - st_cpu.tms_stime<< "\n";

            }
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
                return 0;
            }
            static clock_t st_time;
            static clock_t en_time;
            static struct tms st_cpu;
            static struct tms en_cpu;
            st_time = times(&st_cpu);
            pid_t pid = fork();
            if (pid == -1) {
                perror("");
                _exit(0);
            }
            if (pid == 0) //it's the child code
            {
                signal(SIGINT, childSignal);
                close(0);
                dup(pfd);
                if(strcmp(this->argv[0], "pwd") == 0)
                {
                    char buff[FILENAME_MAX];
                    std::cout << getcwd(buff, FILENAME_MAX) << '\n';
                    _exit(0);
                }
                else if(strcmp(this->argv[0], "cd") == 0)
                {
                    if (this->numofArgv == 1) {
                        passwd *userdata = getpwuid(getuid());
                        chdir(userdata->pw_dir);
                        close(pfd);
                        _exit(0);
                    } else {
                        chdir(this->argv[1]);
                        close(pfd);
                        _exit(0);
                    }
                } else {
                    execvp(this->argv[0], this->argv);
                    close(pfd);
                    perror("error");
                    _exit(0);
                }
            }
            else //it's the parent code
            {
                this->childPid = pid;
                wait(0);
                if(this->timeMode == 1)
                {
                    en_time = times(&en_cpu);
                    printf("Real Time: %jd, User Time %jd, System Time %jd\n",
                           (intmax_t)(en_time - st_time),
                           (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
                           (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
                }
                return 0;
            }
        }
        case NEWOUTPUT_MODE: {
            int pfd;
            pfd = open(fname.c_str(), O_WRONLY | O_CREAT,420);
            if (pfd == -1) {
                perror("");
                return 0;
            }
            static clock_t st_time;
            static clock_t en_time;
            static struct tms st_cpu;
            static struct tms en_cpu;
            st_time = times(&st_cpu);
            pid_t pid = fork();
            if (pid == -1) {
                perror("");
                _exit(0);
            }
            if (pid == 0) //it's the child code
            {
                signal(SIGINT, childSignal);
                close(1);
                dup(pfd);
                if(strcmp(this->argv[0], "pwd") == 0)
                {
                    char buff[FILENAME_MAX];
                    std::cout << getcwd(buff, FILENAME_MAX) << '\n';
                    _exit(0);
                }
                else if(strcmp(this->argv[0], "cd") == 0)
                {
                    if (this->numofArgv == 1) {
                        passwd *userdata = getpwuid(getuid());
                        chdir(userdata->pw_dir);
                        close(pfd);
                        _exit(0);
                    } else{
                        chdir(this->argv[1]);
                        close(pfd);
                        _exit(0);
                    }
                }else {
                    execvp(this->argv[0], this->argv);
                    close(pfd);
                    perror("error");
                    _exit(0);
                }
            }
            else //it's the parent code
            {
                this->childPid = pid;
                wait(0);
                if(this->timeMode == 1)
                {
                    en_time = times(&en_cpu);
                    printf("Real Time: %jd, User Time %jd, System Time %jd\n",
                           (intmax_t)(en_time - st_time),
                           (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
                           (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
                }
                return 0;
            }
        }
        case EXISTINGOUTPUT_MODE: {
            int pfd;
            pfd = open(fname.c_str(), O_WRONLY | O_CREAT | O_APPEND,420);
            if (pfd == -1) {
                perror("");
                return 0;
            }
            static clock_t st_time;
            static clock_t en_time;
            static struct tms st_cpu;
            static struct tms en_cpu;
            st_time = times(&st_cpu);
            pid_t pid = fork();
            if (pid == -1) {
                perror("");
                _exit(0);
            }
            if (pid == 0) //it's the child code
            {
                signal(SIGINT, childSignal);
                close(1);
                dup(pfd);
                if(strcmp(this->argv[0], "pwd") == 0)
                {
                    char buff[FILENAME_MAX];
                    std::cout << getcwd(buff, FILENAME_MAX) << '\n';
                    close(pfd);
                    _exit(0);
                }
                else if(strcmp(this->argv[0], "cd") == 0)
                {
                    if (this->numofArgv == 1) {
                        passwd *userdata = getpwuid(getuid());
                        chdir(userdata->pw_dir);
                        close(pfd);
                        _exit(0);
                    } else
                        chdir(this->argv[1]);
                    close(pfd);
                    _exit(0);
                }else {
                    execvp(this->argv[0], this->argv);
                    close(pfd);
                    perror("error");
                    _exit(0);
                }
            }
            else //it's the parent code
            {
                this->childPid = pid;
                wait(0);
                if(this->timeMode == 1)
                {
                    en_time = times(&en_cpu);
                    printf("Real Time: %jd, User Time %jd, System Time %jd\n",
                           (intmax_t)(en_time - st_time),
                           (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
                           (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
                }
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
        if (pfdOUTPUT == -1 || pfdINPUT == -1) {
            perror("");
            return 0;
        }
    }
    else if (mode == NEWOUTPUT_MODE){
        pfdOUTPUT = open(outfname.c_str(), O_WRONLY | O_CREAT, 420);
        pfdINPUT = open(inpfname.c_str(), O_RDONLY);
        if (pfdOUTPUT == -1 || pfdINPUT == -1) {
            perror("");
            return 0;
        }
    }
    else {
        perror("Wrong mode");
        return -1;
    }
    static clock_t st_time;
    static clock_t en_time;
    static struct tms st_cpu;
    static struct tms en_cpu;
    st_time = times(&st_cpu);
    pid_t pid = fork();
    if (pid == -1) {
        perror("");
        return -1;
    }
    if (pid == 0) //it's the child code
    {
        close(1);
        dup(pfdOUTPUT);
        close(0);
        dup(pfdINPUT);

        if(strcmp(this->argv[0], "pwd") == 0)
        {
            char buff[FILENAME_MAX];
            std::cout << getcwd(buff, FILENAME_MAX);
            _exit(0);
        }
        else if(strcmp(this->argv[0], "cd") == 0)
        {
            if (this->numofArgv == 1) {
                passwd *userdata = getpwuid(getuid());
                chdir(userdata->pw_dir);
                _exit(0);
            } else {
                chdir(this->argv[1]);
                _exit(0);
            }
        }
        else{
            execvp(this->argv[0], this->argv);
            close(pfdINPUT);
            close(pfdOUTPUT);
            perror("error");
            _exit(0);
        }
    }
    else //it's the parent code
    {
        this->childPid = pid;
        wait(0);
        if(this->timeMode == 1)
        {
            en_time = times(&en_cpu);
            printf("Real Time: %jd, User Time %jd, System Time %jd\n",
                   (intmax_t)(en_time - st_time),
                   (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
                   (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
        }
        return 0;
    }
}
int ConveyorElement::intoPipeIO(int I, int O) {
    static clock_t st_time;
    static clock_t en_time;
    static struct tms st_cpu;
    static struct tms en_cpu;
    st_time = times(&st_cpu);
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
            perror("error");
            exit(1);
        }


        if(strcmp(this->argv[0], "pwd") == 0)
        {
            char buff[FILENAME_MAX];
            std::cout << getcwd(buff, FILENAME_MAX);
            _exit(0);
        }
        else if(strcmp(this->argv[0], "cd") == 0)
        {
            if (this->numofArgv == 1) {
                passwd *userdata = getpwuid(getuid());
                chdir(userdata->pw_dir);
                _exit(0);
            } else
                chdir(this->argv[1]);
            _exit(0);
        }
        else{
            execvp(this->argv[0], this->argv);
        }
        perror("error");
        _exit(0);
    } else //it's the parent code
    {
        this->childPid = pid;
        if(this->timeMode == 1)
        {
            en_time = times(&en_cpu);
            printf("Real Time: %jd, User Time %jd, System Time %jd\n",
                   (intmax_t)(en_time - st_time),
                   (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
                   (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
        }
        return 0;
    }
}
int ConveyorElement::intoPipeI_(int I) {
    static clock_t st_time;
    static clock_t en_time;
    static struct tms st_cpu;
    static struct tms en_cpu;
    st_time = times(&st_cpu);
    pid_t pid = fork();
    if (pid == -1) {
        perror("");
    }
    if (pid == 0) //it's the child code
    {
        signal(SIGINT, childSignal);
        if (close(0) == -1 || dup2(I, 0) < 0 || close(I) != 0)
        {
            perror("no such programm");
            exit(1);
        }


        if(strcmp(this->argv[0], "pwd") == 0)
        {
            char buff[FILENAME_MAX];
            std::cout << getcwd(buff, FILENAME_MAX);
            _exit(0);
        }
        else if(strcmp(this->argv[0], "cd") == 0)
        {
            if (this->numofArgv == 1) {
                passwd *userdata = getpwuid(getuid());
                chdir(userdata->pw_dir);
                _exit(0);
            } else
                chdir(this->argv[1]);
            _exit(0);
        }
        else{
            execvp(this->argv[0], this->argv);
        }

        perror("no such programm");
        _exit(0);
    } else //it's the parent code
    {
        this->childPid = pid;
        if(this->timeMode == 1)
        {
            en_time = times(&en_cpu);
            printf("Real Time: %jd, User Time %jd, System Time %jd\n",
                   (intmax_t)(en_time - st_time),
                   (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
                   (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
        }
        return 0;
    }
}
int ConveyorElement::intoPipeI_(int I, int mode , std::string outfName) {

    switch (mode) {
        case NEWOUTPUT_MODE: {
            int pfd;
            pfd = open(outfName.c_str(), O_WRONLY | O_CREAT,420);
            if (pfd == -1) {
                perror("");
            }
            this->intoPipeIO(I, pfd);

        }
        case EXISTINGOUTPUT_MODE: {
            int pfd;
            pfd = open(outfName.c_str(), O_WRONLY | O_CREAT | O_APPEND,420);
            if (pfd == -1) {
                perror("");
            }
            this->intoPipeIO(I, pfd);
        }
    }
    static clock_t st_time;
    static clock_t en_time;
    static struct tms st_cpu;
    static struct tms en_cpu;
    st_time = times(&st_cpu);
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
        if(this->timeMode == 1)
        {
            en_time = times(&en_cpu);
            printf("Real Time: %jd, User Time %jd, System Time %jd\n",
                   (intmax_t)(en_time - st_time),
                   (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
                   (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
        }
        return 0;
    }
}
int ConveyorElement::intoPipe_O(int O) {
    static clock_t st_time;
    static clock_t en_time;
    static struct tms st_cpu;
    static struct tms en_cpu;
    st_time = times(&st_cpu);
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
        if(strcmp(this->argv[0], "pwd") == 0)
        {
            char buff[FILENAME_MAX];
            std::cout << getcwd(buff, FILENAME_MAX);
            _exit(0);
        }
        else if(strcmp(this->argv[0], "cd") == 0)
        {
            if (this->numofArgv == 1) {
                passwd *userdata = getpwuid(getuid());
                chdir(userdata->pw_dir);
                _exit(0);
            } else
                chdir(this->argv[1]);
            _exit(0);
        }
        else{
            execvp(this->argv[0], this->argv);
        }
        perror("no such programm");
        _exit(0);
    } else //it's the parent code
    {
        this->childPid = pid;
        if(this->timeMode == 1)
        {
            en_time = times(&en_cpu);
            printf("Real Time: %jd, User Time %jd, System Time %jd\n",
                   (intmax_t)(en_time - st_time),
                   (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
                   (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
        }
        return 0;
    }
}
int ConveyorElement::intoPipe_O(int O, std::string inpfName) {
    int pfd;
    pfd = open(inpfName.c_str(), O_RDONLY);
    if (pfd == -1) {
        perror("");
    }
    static clock_t st_time;
    static clock_t en_time;
    static struct tms st_cpu;
    static struct tms en_cpu;
    st_time = times(&st_cpu);
    pid_t pid = fork();
    if (pid == -1) {
        perror("");
    }
    if (pid == 0) //it's the child code
    {
        signal(SIGINT, childSignal);
        close(0);
        dup(pfd);
        if (close(1) == -1 || dup2(O, 1) < 0 || close(O) != 0)
        {
            perror(this->argv[0]);
            _exit(0);
        }
        if(strcmp(this->argv[0], "pwd") == 0)
        {
            char buff[FILENAME_MAX];
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
        else{
            execvp(this->argv[0], this->argv);
        }
        perror("no such programm");
        _exit(0);
    } else //it's the parent code
    {
        close(pfd);
        this->childPid = pid;
        if(this->timeMode == 1)
        {
            en_time = times(&en_cpu);
            printf("Real Time: %jd, User Time %jd, System Time %jd\n",
                   (intmax_t)(en_time - st_time),
                   (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
                   (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
        }
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
int ConveyorElement::ctm() {
    this->timeMode = 1;
    if(strcmp(this->argv[0], "time") == 0)
    {
        free(this->argv[0]);
        for (int i = 1; i < this->numofArgv; i++)
        {
            this->argv[i-1] = this->argv[i];
        }
        this->argv[this->numofArgv - 1] = NULL;
        this->numofArgv--;
        return this->numofArgv;
    } else{
        perror("not time");
        return -1;
    }
}



//TODO: we can return ConveyorElement, not int to make possible .sjknad().ajlksjdal().asmdlk()