#include "user_interface.h"

int main()
{

        init_user_interface();
        char* input, command;

    while(1){
        input = get_user_input();
        command = parse_command(input);
        process_command(command);
    }
        free(input);
        free(command);
    
    return 0;
}