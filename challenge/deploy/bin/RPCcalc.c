#include <stdio.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "calc.h"
#define LEN_FLAG 40

extern char error_packet[13];
extern char retry_packet[13];
extern char done_packet[13];
extern char MAGIC_SEND[5];
extern char MAGIC_RECV[5];



void add_flag(char *usr,char *expr_id, char *expr){
  struct user *usr1=find_user(usr);
  usr1->exp=malloc(sizeof(struct expr)+1);
  usr1->exp->next=NULL;
  usr1->exp->expr_id=(char*)malloc(0x20);
  strcpy(usr1->exp->expr_id,expr_id);
  usr1->exp->res=(long)malloc(strlen(expr)+1);
  memcpy((char *)(usr1->exp->res),expr,strlen(expr));
  *((char *)(usr1->exp->res)+strlen(expr))='\x00';
}

void init(){
  char admin[]="superusr";
  char expr_id[]="admin'sflag";
  insert_key(admin);
  int fd=open("./flag",0);
  char flag[LEN_FLAG+1]="";
  if(!read(fd,flag,LEN_FLAG)){
    perror("File missing, please contact the organizer");
    exit(1);
  }
  close(fd);
  add_flag(admin,expr_id,flag);
}


int main(int argc, char const *argv[])
{
  forget_times=0;
  user_head=NULL;
  int a,len,flag,usr_key;
  a=0;
  flag=0;
  char s[9]="";
  char buf[0x200];
  init();
  do
  {
    do
      read(0, buf, 4);
    while ( memcmp(buf, MAGIC_SEND, 4) );
    read(0, buf, 4);
    len = u32(buf);
    if(len<12)
      exit(1);
    read(0, buf, 4);
    flag = 0;
    switch ( (unsigned int)u32(buf) )
    {
      case 0:
        if ( a )
        {
          write(1, error_packet, 0xC);
        }
        else
        {
          a = 1;
          write(1, done_packet, 0xC);
        }
        break;
      case 1:
        usr_key = generate_key();
        sprintf(s, "%08x", usr_key);
        insert_key(s);
        construct_result(s, (int *)buf, 8);
        write(1, buf, 0x18);
        break;
      case 2:
        get_result(len-12);
        break;
      case 3:
        call_expr(len-12);
        break;
      case 4:
        flag = 1;
        break;
      case 5:
        if(forget_times++<2)
        	if_you_forget_expr_id(len-12);
      	break;
      case 6:
      	status_extant_for_checker(len-12);
      	break;
      default:
        write(1, error_packet, 0xC);
        break;
    }
  }
  while ( !flag );
  return 0;
}



