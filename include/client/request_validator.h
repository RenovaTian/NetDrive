#ifndef REQUEST_VALIDATOR_H
#define REQUEST_VALIDATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int request_validator(int access_level, int opt_level){
    //����Ȩ�޵ȼ��Ͳ��������жϣ��������õȼ���С�Ƚϴ���
    if(access_level >= opt_level){
        return 0;
    }else{
        printf("����Ȩִ�иò�����");
        return -1;
    }
}

#endif