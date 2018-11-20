
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


int main(int argc, char* argv[]){

    //--Defining basic data
    passwd *userdata = getpwuid(getuid());

    std::string user_name = userdata->pw_name ; // defining name of start user
    bool is_root = !(bool) std::strcmp(user_name.c_str(),"root"); // root flag
    char user_invite = is_root?'!':'>'; // just to make output easy

    char buff[FILENAME_MAX];
    std::string start_dir = getcwd(buff, FILENAME_MAX); //defining start directory

    //Here the program starts.
    while (true) {
        std::printf("[%s %s]%c ", user_name.c_str(), start_dir.c_str(),
                    user_invite); // invite like: [user /home/user/Desktop]> or [root /home/user/Desktop]!

        std::string input_line = "";
        std::getline(std::cin, input_line);

        std::size_t convSimbol = input_line.find('|');
        std::size_t inpRedirect = input_line.find('<');
        std::size_t outRedirect = input_line.find('>');

        if (convSimbol != std::string::npos) {
            //TODO: parsing | | | and #PATH
        } else if (outRedirect != std::string::npos && inpRedirect != std::string::npos) {
            std::size_t firstSimb = outRedirect < inpRedirect ? outRedirect : inpRedirect; // looking for first of '<' and '>'
            ConveyorElement conveyorElement1(input_line.substr(0, firstSimb - 1).c_str());

            if (firstSimb == outRedirect)
            {
                if (std::strcmp(input_line.substr(outRedirect, 2).c_str(), ">>") == 0) {
                    std::string outfName;
                    std::istringstream(input_line.substr(outRedirect + 2, inpRedirect - (outRedirect + 2))) >> outfName;
                    std::string inpfName;
                    std::istringstream(input_line.substr(inpRedirect + 1)) >> outfName;
                    conveyorElement1.run(inpfName,outfName,EXISTINGOUTPUT_MODE);
                    wait(0);
                } else {
                    std::string outfName;
                    std::istringstream(input_line.substr(outRedirect + 1, inpRedirect - (outRedirect + 1))) >> outfName;
                    std::string inpfName;
                    std::istringstream(input_line.substr(inpRedirect + 1)) >> outfName;
                    conveyorElement1.run(inpfName,outfName,NEWOUTPUT_MODE);
                    wait(0);
                }
            } else{
                if (std::strcmp(input_line.substr(outRedirect, 2).c_str(), ">>") == 0) {
                    std::string outfName;
                    std::istringstream(input_line.substr(outRedirect + 2)) >> outfName;
                    std::string inpfName;
                    std::istringstream(input_line.substr(inpRedirect + 1, outRedirect - (inpRedirect + 1))) >> outfName;
                    conveyorElement1.run(inpfName,outfName,EXISTINGOUTPUT_MODE);
                    wait(0);
                } else {
                    std::string outfName;
                    std::istringstream(input_line.substr(outRedirect + 1)) >> outfName;
                    std::string inpfName;
                    std::istringstream(input_line.substr(inpRedirect + 1, outRedirect - (inpRedirect + 1))) >> outfName;
                    conveyorElement1.run(inpfName,outfName,NEWOUTPUT_MODE);
                    wait(0);
                }
            }

            if (std::strcmp(input_line.substr(outRedirect, 2).c_str(), ">>")) {

            } else {
                std::string fName;
                std::istringstream(input_line.substr(outRedirect + 2)) >> fName;
                conveyorElement1.run(fName, EXISTINGOUTPUT_MODE);
                wait(0);
            }

        } else if (outRedirect != std::string::npos) {
            ConveyorElement conveyorElement1(input_line.substr(0, outRedirect - 1));
            if (std::strcmp(input_line.substr(outRedirect, 2).c_str(), ">>") == 0) {
                std::string fName;
                std::istringstream(input_line.substr(outRedirect + 2)) >> fName;
                conveyorElement1.run(fName, EXISTINGOUTPUT_MODE);
                wait(0);
            } else {
                std::string fName;
                std::istringstream(input_line.substr(outRedirect + 1)) >> fName;
                conveyorElement1.run(fName, NEWOUTPUT_MODE);
                wait(0);
            }
        } else if (inpRedirect != std::string::npos) {
            ConveyorElement conveyorElement1(input_line.substr(0, inpRedirect - 1));
            std::string fName;
            std::istringstream(input_line.substr(inpRedirect + 1)) >> fName;
            conveyorElement1.run(fName, INPUT_MODE);
            wait(0);
        } else {
            ConveyorElement conveyorElement(input_line);
            conveyorElement.run();
            wait(0);
        }


        //TODO: убрать этот колхоз(сделать нормальный выход из программы)
        if(!std::strcmp(input_line.c_str(),"exit"))
            break;

    }
    return 0;
}



//function, that's interprets line as conveyor
int lineinterpreter(const std::string input_line, char argv[], const std::string start_dir)
{
    std::istringstream input_stream(input_line);
    std::string conv_elem; //не нашел нормального англицкого слова для элемента конвеера
    while (std::getline(input_stream,conv_elem,'|'))
    {
        conv_elem = start_dir + "/"+ conv_elem;
        std::cout << conv_elem << '\n';
       execl(conv_elem.c_str(), NULL);
    };
    return 0;
}

int executer(const std::string input_line)
{
    std::istringstream input_stream(input_line);
    std::string str_elem;
    std::vector<std::string> conveer;
    while (std::getline(input_stream,str_elem,'|')) conveer.push_back(str_elem);   //заполняем вектор элементов конвеера
    //теперь для каждый элемент надо выполнить:
    for(int i = 0; i < conveer.size(); i++)
        std::cout << conveer[i] << '\n';

    if (conveer.size() == 1)
    {
        pid_t pid =fork();
        if(pid == -1)
            return -1;
        else if(pid == 0)
        {
            //there is no conveyor
            std::istringstream commandStream(conveer[0]);
            std::string comndString = "";
            std::getline(commandStream,comndString);
            if (!(bool)std::strcmp("pwd", comndString.c_str()))
            {
                char buff[FILENAME_MAX];
                std::string start_dir = getcwd(buff, FILENAME_MAX);
                std::cout << buff << '\n';
            }
        }
    }
    else{

    }
//    conv_elem = start_dir + "/"+ conv_elem;
//    std::cout << conv_elem << '\n';
//    execl(conv_elem.c_str(), NULL);
    //TODO: parsing
    //TODO: разбивание строки по |
    //TODO: реализация time [cmd args]
    return 0;
}

//TODO:Надо сделать обработку ошибок системных вызовов через errno из libc