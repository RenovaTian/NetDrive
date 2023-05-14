#ifndef REQUEST_VALIDATOR_H
#define REQUEST_VALIDATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int request_validator(int access_level, int opt_level){
    //根据权限等级和操作类型判断，这里先用等级大小比较代替
    if(access_level >= opt_level){
        return 0;
    }else{
        printf("您无权执行该操作！");
        return -1;
    }
}

#endif