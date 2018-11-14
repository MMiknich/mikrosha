// DEBUG
#define DEBUG false // DEBUG mode


//SETTINGS
#define MAX_PIPE_Q 100 // max quantity of

#include <sstream>
#include <iostream>
#include <dirent.h>
#include <stdio.h>  /* defines FILENAME_MAX */
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <sstream>

bool debug = false;

int lineinterpreter(const std::string input_line);

int main(int argc, char* argv[]){

    //-<DEBUG-CODE-- initializing of debug
    //*
    if (argc > 1) debug = !(bool) strcmp("DEBUG",argv[1]);
    if (DEBUG || debug) std::cout << "\n\nYou in DEBUG mode now\n\n\n";
    //*
    //-<DEBUG-CODE--


    //--Defining basic data
    passwd *userdata = getpwuid(getuid());
    std::string user_name = userdata->pw_name; // defining name of start user
    bool is_root = !(bool) std::strcmp(user_name.c_str(),"root"); // root flag
    char user_invite = is_root?'!':'>'; // just to make output easer

    char buff[FILENAME_MAX];
    std::string start_dir = getcwd(buff, FILENAME_MAX); //defining start directory

    //-<DEBUG-CODE--
    //*
    std::string root_out = is_root ? "ordinary user\n" : "root detected\n";
    if (DEBUG || debug) std::cout << root_out;
    //*
    //-<DEBUG-CODE--


    while (true)
    {
        // invite like: [user /home/user/Desktop]> or [root /home/user/Desktop]!
        std::printf("[%s %s]%c ",user_name.c_str(), start_dir.c_str(), user_invite);
        std::string input_line = "";
        std::getline(std::cin, input_line);
        lineinterpreter(input_line);


        //-<DEBUG-CODE--
        //*
        if(debug || DEBUG) {
            std::cout << "\n---------DEBUG------------\nINPUT: " << input_line << "\n---------DEBUG------------\n\n";
        }
        //*

        //-<DEBUG-CODE--

        //TODO: убрать этот колхоз(сделать нормальный выход из программы)
        if(!std::strcmp(input_line.c_str(),"exit"))
            break;

        //TODO: execl("hi", "-jh",);
    }
    return 0;
}


//function, that's interprets line as conveyor
int lineinterpreter(const std::string input_line)
{
    std::istringstream input_stream(input_line);
    std::string conv_elem; //не нашел нормального англицкого слова для элемента конвеера
    while (std::getline(input_stream,conv_elem,'|'))
    {
        std::cout << conv_elem << '\n';
    };
    return 0;
}