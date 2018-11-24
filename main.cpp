
//SETTINGS
#define MAX_PIPE_Q 100 // max quantity of


#include <iostream>
#include <sstream>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <map>
#include <vector>
#include <fcntl.h>
#include "ConveyorElement.h"

void parentSignal(int inp);
int lineinterpreter(const std::string input_line);

int main(int argc, char* argv[]){
    signal(SIGINT, SIG_IGN);
    while (true) {

        //--Defining basic data
        passwd *userdata = getpwuid(getuid());

        std::string user_name = userdata->pw_name ; // defining name of start user
        bool isRoot = !(bool) std::strcmp(user_name.c_str(),"root"); // root flag
        char userInvite = isRoot?'!':'>'; // just to make output easy

        char buff[FILENAME_MAX];
        std::string start_dir = getcwd(buff, FILENAME_MAX); //defining start directory


        std::printf("[%s %s]%c ", user_name.c_str(), start_dir.c_str(),
                    userInvite); // invite like: [user /home/user/Desktop]> or [root /home/user/Desktop]!

        std::string inputLine = "";
        std::getline(std::cin, inputLine);

        std::size_t convSimbol = inputLine.find('|');
        std::size_t inpRedirect = inputLine.find('<');
        std::size_t outRedirect = inputLine.find('>');


        //this code is made for  <, >, >>.
        if (convSimbol != std::string::npos) {
            //TODO: parsing | | | and #PATH
            lineinterpreter(inputLine);

        }
        else if (outRedirect != std::string::npos && inpRedirect != std::string::npos)
        {
            std::size_t firstSimb = outRedirect < inpRedirect ? outRedirect : inpRedirect; // looking for first of '<' and '>'
            ConveyorElement conveyorElement1(inputLine.substr(0, firstSimb - 1).c_str());

            if (firstSimb == outRedirect)
            {
                if (std::strcmp(inputLine.substr(outRedirect, 2).c_str(), ">>") == 0) {
                    std::string outfName;
                    std::istringstream(inputLine.substr(outRedirect + 2, inpRedirect - (outRedirect + 2))) >> outfName;
                    std::string inpfName;
                    std::istringstream(inputLine.substr(inpRedirect + 1)) >> outfName;
                    conveyorElement1.run(inpfName,outfName,EXISTINGOUTPUT_MODE);
                    wait(nullptr);
                } else {
                    std::string outfName;
                    std::istringstream(inputLine.substr(outRedirect + 1, inpRedirect - (outRedirect + 1))) >> outfName;
                    std::string inpfName;
                    std::istringstream(inputLine.substr(inpRedirect + 1)) >> outfName;
                    conveyorElement1.run(inpfName,outfName,NEWOUTPUT_MODE);
                    wait(nullptr);
                }
            } else {
                if (std::strcmp(inputLine.substr(outRedirect, 2).c_str(), ">>") == 0) {
                    std::string outfName;
                    std::istringstream(inputLine.substr(outRedirect + 2)) >> outfName;
                    std::string inpfName;
                    std::istringstream(inputLine.substr(inpRedirect + 1, outRedirect - (inpRedirect + 1))) >> outfName;
                    conveyorElement1.run(inpfName, outfName, EXISTINGOUTPUT_MODE);
                    wait(nullptr);
                } else {
                    std::string outfName;
                    std::istringstream(inputLine.substr(outRedirect + 1)) >> outfName;
                    std::string inpfName;
                    std::istringstream(inputLine.substr(inpRedirect + 1, outRedirect - (inpRedirect + 1))) >> outfName;
                    conveyorElement1.run(inpfName, outfName, NEWOUTPUT_MODE);
                    wait(nullptr);
                }
            }

            if (std::strcmp(inputLine.substr(outRedirect, 2).c_str(), ">>")) {

            } else {
                std::string fName;
                std::istringstream(inputLine.substr(outRedirect + 2)) >> fName;
                conveyorElement1.run(fName, EXISTINGOUTPUT_MODE);
                wait(nullptr);
            }

        } else if (outRedirect != std::string::npos) {
            ConveyorElement conveyorElement1(inputLine.substr(0, outRedirect - 1));
            if (std::strcmp(inputLine.substr(outRedirect, 2).c_str(), ">>") == 0) {
                std::string fName;
                std::istringstream(inputLine.substr(outRedirect + 2)) >> fName;
                conveyorElement1.run(fName, EXISTINGOUTPUT_MODE);
                wait(nullptr);
            } else {
                std::string fName;
                std::istringstream(inputLine.substr(outRedirect + 1)) >> fName;
                conveyorElement1.run(fName, NEWOUTPUT_MODE);
                wait(nullptr);
            }
        } else if (inpRedirect != std::string::npos) {
            ConveyorElement conveyorElement1(inputLine.substr(0, inpRedirect - 1));
            std::string fName;
            std::istringstream(inputLine.substr(inpRedirect + 1)) >> fName;
            conveyorElement1.run(fName, INPUT_MODE);
            wait(nullptr);
        } else {
            ConveyorElement conveyorElement(inputLine);
            conveyorElement.run();
            wait(nullptr);
        }


        //TODO: убрать этот колхоз(сделать нормальный выход из программы)
        if(!std::strcmp(inputLine.c_str(),"exit"))
            break;

    }
    return 0;
}



//function, that's interprets line as conveyor
int lineinterpreter(const std::string input_line)
{
    std::istringstream input_stream(input_line);
    std::string inpComm; //не нашел нормального англицкого слова для элемента конвеера
    std::vector<ConveyorElement*> conveyor;
    while (std::getline(input_stream,inpComm,'|'))
    {
        conveyor.push_back(new ConveyorElement(inpComm));
    };
    int cSize = conveyor.size();
    int *pipeline = new int[(cSize-1)*2];

    int p = pipe(pipeline);
    if (p == -1) {
        perror("pipe");
        return 0;
    }

    (*conveyor[0]).intoPipe_O(*(pipeline + 1));

    for (int i = 1; i < cSize - 1; i++)
    {
        if (pipe(pipeline + (2*i))== -1) {
            perror("pipe");
            return 0;
        }

        (*conveyor[i]).intoPipeIO(*(pipeline + (2*i) - 2), *(pipeline + (2*i) +1));
    }
    (*conveyor[cSize - 1]).intoPipeI_(*(pipeline + (2*cSize) - 4));
    for (int i = 0; i < cSize - 1; i++)
    {
        wait(0);
    }
    return 0;
}

void parentSignal(int inp)
{
    return;
}


    //TODO: parsing
    //TODO: разбивание строки по |
    //TODO: реализация time [cmd args]

//TODO:Надо сделать обработку ошибок системных вызовов через errno из libc