#include "command_input.h"

int CommandInputSplitCommand(char *plain_text, char* command, char args[][INPUT_LEN], unsigned int max_arg_num)
{
    char *buf = plain_text;

    memset(buf,0,INPUT_LEN);
    memset(command,0,INPUT_LEN);
    for(unsigned int i = 0; i<max_arg_num; i++)
        memset(args[i],0,INPUT_LEN);


    if(!fgets(buf,INPUT_LEN,stdin))
        return -1;
    int success = sscanf(buf,"%1023s",command);
    if(success<=0)
        return -1;

    char* arg_str = buf+strlen(command);
    unsigned int arg_number = 0;
    for(unsigned int i = 0; i<max_arg_num; i++)
    {
        success = sscanf(arg_str,"%1023s",args[i]);
        if(success<=0)
            break;
        arg_number+=success;
        arg_str+=strlen(args[i])+1;
    }
    return arg_number;
}