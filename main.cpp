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

int lineInterpreter(const std::string &input_line);

int main(int argc, char* argv[]){
    while (true) {
        signal(SIGINT, SIG_IGN); // we ignore ctrl+C;

        signal(EOF, childSignal); // in ctrl+D case we exit;


        //--Defining basic data
        passwd *userdata = getpwuid(getuid());

        // defining name of start user
        std::string user_name = userdata->pw_name;
        bool isRoot = !(bool) std::strcmp(user_name.c_str(), "root"); // root flag
        char userInvite = isRoot ? '!' : '>';

        char buff[FILENAME_MAX];
        std::string start_dir = getcwd(buff, FILENAME_MAX); //defining start directory

        // invite like: [user /home/user/Desktop]> or [root /home/user/Desktop]!
        std::printf("[%s %s]%c ", user_name.c_str(), start_dir.c_str(), userInvite);


        //making out using ^D in start of string and setting inputString
        char breaker = (char) getchar();
        std::string inputLine;
        if(breaker == EOF)
        {
            printf("\n");
            break;
        }
        else if(breaker == '\n')
            continue;
        else{
            while (breaker  != '\n') {
                inputLine += breaker;
                breaker = getchar();
            }
        }


        // Defining  type of expression
        std::size_t convSimbol = inputLine.find('|');
        std::size_t inpRedirect = inputLine.find('<');
        std::size_t outRedirect = inputLine.find('>');

        //check for mistake
        if(outRedirect != std::string::npos) {
            if (inputLine.size() == outRedirect + 1 || inputLine.substr(outRedirect + 2).find('>') != std::string::npos)
            {
                std::cout << ("Wrong syntax\n");
                continue;
            }
        }
        if(inpRedirect != std::string::npos) {
            if (inputLine.size() == inpRedirect + 1 ||inputLine.substr(inpRedirect + 1).find('<') != std::string::npos)
            {
                std::cout << ("Wrong syntax\n");
                continue;
            }
        }

        //this code made for pipe
        if (convSimbol != std::string::npos) {
            lineInterpreter(inputLine);

        }//this code is made for  <, >, >>.
        else if (outRedirect != std::string::npos && inpRedirect != std::string::npos) // in case both < >
        {
            std::size_t firstSimb =
                    outRedirect < inpRedirect ? outRedirect : inpRedirect; // looking for first of '<' and '>'
            ConveyorElement conveyorElement1(inputLine.substr(0, firstSimb - 1));

            if (firstSimb == outRedirect) //first symbol is '>' so:
            {
                if (std::strcmp(inputLine.substr(outRedirect, 2).c_str(), ">>") == 0) { // '>>' case
                    std::string outfName;
                    std::istringstream(inputLine.substr(outRedirect + 2, inpRedirect - (outRedirect + 2))) >> outfName;
                    std::string inpfName;
                    std::istringstream(inputLine.substr(inpRedirect + 1)) >> inpfName;

                    //check for existing of all names:
                    if (outfName.empty() || inpfName.empty()) {
                        std::cout << ("Wrong syntax\n");
                        continue;
                    }

                    conveyorElement1.run(inpfName, outfName, EXISTINGOUTPUT_MODE);

                } else {                                                                 // '>' case
                    std::string outfName;
                    std::istringstream(inputLine.substr(outRedirect + 1, inpRedirect - (outRedirect + 1))) >> outfName;
                    std::string inpfName;
                    std::istringstream(inputLine.substr(inpRedirect + 1)) >> inpfName;

                    //check for existing of all names:
                    if (outfName.empty() || inpfName.empty()) {
                        std::cout << ("Wrong syntax\n");
                        continue;
                    }

                    conveyorElement1.run(inpfName, outfName, NEWOUTPUT_MODE);
                }
            } else { //first symbol is '<' so:

                if (std::strcmp(inputLine.substr(outRedirect, 2).c_str(), ">>") == 0) { // '>>' case
                    std::string outfName;
                    std::istringstream(inputLine.substr(outRedirect + 2)) >> outfName;
                    std::string inpfName;
                    std::istringstream(inputLine.substr(inpRedirect + 1, outRedirect - (inpRedirect + 1))) >> inpfName;

                    //check for existing of all names:
                    if (outfName.empty() || inpfName.empty()) {
                        std::cout << ("Wrong syntax\n");
                        continue;
                    }

                    conveyorElement1.run(inpfName, outfName, EXISTINGOUTPUT_MODE);
                } else {                                                                // '>' case
                    std::string outfName;
                    std::istringstream(inputLine.substr(outRedirect + 1)) >> outfName;
                    std::string inpfName;
                    std::istringstream(inputLine.substr(inpRedirect + 1, outRedirect - (inpRedirect + 1))) >> inpfName;

                    //check for existing of all names:
                    if (outfName.empty() || inpfName.empty()) {
                        std::cout << ("Wrong syntax\n");
                        continue;
                    }

                    conveyorElement1.run(inpfName, outfName, NEWOUTPUT_MODE);
                }
            }
            // now we have only one type of redirection
        }else if (outRedirect != std::string::npos) { // in case > or >>

            ConveyorElement conveyorElement1(inputLine.substr(0, outRedirect - 1));

            if (std::strcmp(inputLine.substr(outRedirect, 2).c_str(), ">>") == 0) { // '>>'
                std::string outfName;
                std::istringstream(inputLine.substr(outRedirect + 2)) >> outfName;

                //check for existing of all names:
                if (outfName.empty()) {
                    std::cout << ("Wrong syntax\n");
                    continue;
                }

                conveyorElement1.run(outfName, EXISTINGOUTPUT_MODE);

            } else { // case of only '>' redirection
                std::string outfName;
                std::istringstream(inputLine.substr(outRedirect + 1)) >> outfName;

                //check for existing of all names:
                if (outfName.empty()) {
                    std::cout << ("Wrong syntax\n");
                    continue;
                }

                conveyorElement1.run(outfName, NEWOUTPUT_MODE);
                wait(nullptr);
            }
        } else if (inpRedirect != std::string::npos) { // in case of only <
            ConveyorElement conveyorElement1(inputLine.substr(0, inpRedirect));
            std::string inpfName;
            std::istringstream(inputLine.substr(inpRedirect + 1)) >> inpfName;

            //check for existing of all names:
            if (inpfName.empty()) {
                std::cout << ("Wrong syntax\n");
                continue;
            }

            conveyorElement1.run(inpfName, INPUT_MODE);
        } else { // no redirection case
            ConveyorElement conveyorElement(inputLine);
            conveyorElement.run();
        }


        //TODO: я знаю что жто нечитабельно


    }
    return 0;
}



//function, that's interprets line as conveyor
int lineInterpreter(const std::string &input_line) {
    std::istringstream input_stream(input_line);
    std::string inpComm;
    std::vector<ConveyorElement *> conveyor;
    int inpFlag = 0;
    int outFlag = 0;
    std::string outfName;
    std::string inpfName;

    while (std::getline(input_stream, inpComm, '|')) {
        size_t inpRedirect = inpComm.find('<');
        size_t outRedirect = inpComm.find('>');

        // in case if firs conveyer element have <  redirection
        if (inpRedirect != std::string::npos)
        {
            if(!conveyor.empty())
            {
                std::cout << "Wrong input" << '\n';
                return -1;
            }

            std::string constr = inpComm.substr(0, inpRedirect);
            ConveyorElement *conveyorElement1 = new ConveyorElement(constr);
            conveyor.push_back(conveyorElement1);
            std::istringstream(inpComm.substr(inpRedirect + 1)) >> inpfName;
            //check for existing of all names:
            if (inpfName.empty()) {
                std::cout << ("Wrong syntax\n");
                continue;
            }
            //INPUT_MODE
            inpFlag = 1;
        // in case if last conveyer element have > or >> redirection
        }else if (outRedirect != std::string::npos)
        {
            if (outRedirect + 1 == inpComm.size() )
            {
                std::cout << "Wrong input" << '\n';
                return -1;
            }

            std::string constr = inpComm.substr(0, outRedirect - 1);
            ConveyorElement *conveyorElement1 = new ConveyorElement(constr);
            conveyor.push_back(conveyorElement1);

            if (std::strcmp(inpComm.substr(outRedirect, 2).c_str(), ">>") == 0) { // '>>'
                std::istringstream(inpComm.substr(outRedirect + 2)) >> outfName;
                //check for existing of all names:
                if (outfName.empty()) {
                    std::cout << ("Wrong syntax\n");
                    continue;
                }

                //EXISTINGOUTPUT_MODE
                outFlag = 2;

            } else { // '>'
                std::istringstream(inpComm.substr(outRedirect + 1)) >> outfName;

                //check for existing of all names:
                if (outfName.empty()) {
                    std::cout << ("Wrong syntax\n");
                    continue;
                }
                //NEWOUTPUT_MODE
                outFlag = 1;
            }
        } else
            conveyor.push_back(new ConveyorElement(inpComm));
    };
    unsigned long cSize = conveyor.size();
    int *pipeline = new int[(cSize - 1) * 2];
    //  . i>>>>>l ^
    int p = pipe(pipeline);   //i>>>>>o i>>>>>o
    if (p == -1) {            //0 1 2 3 4 5 6 7
        perror("pipe");       //o<i o<i o<i
        return 0;             //pipe model
    }

    if (inpFlag == 0)
    {
        (*conveyor[0]).intoPipe_O(*(pipeline + 1));
        close(*(pipeline + 1));
    } else{
        (*conveyor[0]).intoPipe_O(*(pipeline + 1), inpfName);
        close(*(pipeline + 1));
    }

    for (int i = 1; i < cSize - 1; i++) {
        if (pipe(pipeline + (2 * i)) == -1) {
            perror("pipe");
            return 0;
        }

        (*conveyor[i]).intoPipeIO(*(pipeline + (2 * i) - 2), *(pipeline + (2 * i) + 1));
        close(*(pipeline + (2 * i) - 2));
        close(*(pipeline + (2 * i) + 1));
    }
    if (outFlag == 0)
    {
        (*conveyor[cSize - 1]).intoPipeI_(*(pipeline + (2 * cSize) - 4));
    } else if (outFlag == 1){
        (*conveyor[cSize - 1]).intoPipeI_(*(pipeline + (2 * cSize) - 4), NEWOUTPUT_MODE, outfName);
    } else
    {
        (*conveyor[cSize - 1]).intoPipeI_(*(pipeline + (2 * cSize) - 4), EXISTINGOUTPUT_MODE, outfName);
    }
    for (int i = 0; i < cSize; i++) {
        wait(nullptr);
    }
    for (auto &i : conveyor) {
        free(i);
    }
    return 0;
}



    //TODO: parsing
    //TODO: разбивание строки по |
    //TODO: реализация time [cmd args]

//TODO:Надо сделать обработку ошибок системных вызовов через errno из libc