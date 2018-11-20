//
// Created by mmiknich on 16.11.18.
//

#ifndef MIKROSHA_CONVEYORELEMENT_H
#define MIKROSHA_CONVEYORELEMENT_H
#define MAX_NUMBER_OF_ARGUMENTS

#define INPUT_MODE 1 // command < fname
#define NEWOUTPUT_MODE 2 // command > fname
#define EXISTINGOUTPUT_MODE 3 // command >> fname

#include <string>

class ConveyorElement {

private:
    char **argv; // an array of arguments
    unsigned long numofArgv = 0;
    pid_t childPid;
    unsigned long maxArgLen = 0;




public:
    ConveyorElement(std::string inputLine);
    ~ConveyorElement();
    //TODO: pipe and others
    int run();// without inp/outp redirection
    int run(std::string FName, int mode);//with redirection of output
    int run(std::string inpFName,std::string outFName, int mode);//with redirection of output and input
    int print();
    pid_t getPID();
};



#endif //MIKROSHA_CONVEYORELEMENT_H
