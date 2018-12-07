//
// Created by mmiknich on 16.11.18.
//

#ifndef MIKROSHA_CONVEYORELEMENT_H
#define MIKROSHA_CONVEYORELEMENT_H


#define INPUT_MODE 1 // command < fname
#define NEWOUTPUT_MODE 2 // command > fname
#define EXISTINGOUTPUT_MODE 3 // command >> fname

#include <string>

void childSignal(int inp);

class ConveyorElement {

private:
    char **argv; // an array of arguments
    unsigned long numofArgv = 0;
    pid_t childPid;


public:
    ConveyorElement(std::string inputLine);
    ~ConveyorElement();
    //TODO: pipe and others
    int run();// without inp/outp redirection
    int intoPipeIO(int I, int O); // pipe cut
    int intoPipe_O(int O); // input pipe plug
    int intoPipeI_(int I); // output pipe plug
    int run(std::string fname, int mode);//with redirection of output
    int run(std::string inpFName,std::string outFName, int mode);//with redirection of output and input
    pid_t getPID();
};



#endif //MIKROSHA_CONVEYORELEMENT_H
