#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "calc.h"
#include "WjCryptLib_Sha256.h"
#include "WjCryptLib_Md5.h"

#define MAX_SIZE 1024                       /*数组长度*/  
#define SYMBOLSIZE 50
#define N 80

char retry_packet[13]="RPCN\x00\x00\x00\x0c\x00\x00\xbe\xf1\x00";
char done_packet[13]="RPCN\x00\x00\x00\x0c\x00\x00\xbe\xef\x00";
char MAGIC_SEND[5]="RPCM\x00";
char MAGIC_RECV[5]="RPCN\x00";
char error_packet[13]="RPCN\x00\x00\x00\x0c\x00\x00\xbe\xf0\x00";
char dest[MAX_SIZE]={0};

char sha256[]="e30d9ca28b3d7affced15bfc90e85cf426bc42413692e437c586a5ee2a1bcea2";
char md5[]="8b8431584f7f4b4119b89d6f6ef6523e";


long insert_operand(long *operand , long * top_num ,long num){
	(*top_num) ++;  
	operand[*top_num] = num;
	return 0;
}  
long insert_oper (char * oper , long *top_oper , char ch){  
	(*top_oper)++;
	oper[*top_oper] = ch;
	return 0;
}	 
long compare(char *oper , long *top_oper , char ch){     
	if ((oper[*top_oper] == '-' || oper[*top_oper] == '+') && (ch == '*' || ch == '/'))
		return 0;
	else if(*top_oper == -1 || ch == '('|| (oper[*top_oper] == '(' && ch != ')'))
		return 0;
	else if (oper[*top_oper] =='(' && ch == ')' ){  
		(*top_oper)--;  
		return 1;
	}  
	else return -1;
}  


long deal_date(long *operand ,char *oper ,long *top_num, long *top_oper)    /*进行数据运算*/  
{  
	long num_1 = operand[*top_num];              /*取出数据栈中两个数据*/  
	long num_2 = operand[*top_num - 1];  
	long value = 0;  
	
	if(oper[*top_oper] == '+')                  /*加法操作*/  
		value = num_1 + num_2;
	else if(oper[*top_oper] == '-')             /*减法操作*/  
		value = num_2 - num_1;
	else if(oper[*top_oper] == '*')             /*乘法操作*/   
		value = num_2 * num_1;
	else if(oper[*top_oper] == '/')             /*除法操作*/  
		value = num_2 / num_1;
	(*top_num) --;                              /*将数据栈顶下移一位*/  
	operand[*top_num] = value;                  /*将得到的值压入数据栈*/  
	(*top_oper) --;                             /*将操作符栈顶下移一位*/  
}  


long calculation(char *str)

{  
	int len=strlen(str);
	if(len>=MAX_SIZE)
		return 0;
	while(str[0]=='+'||str[0]=='*'||str[0]=='/') {str++;len--;}
	for(int i=0;i<len;i++)
		if ((str[i]=='+'||str[i]=='-'||str[i]=='*'||str[i]=='/')&&(str[i+1]=='+'||str[i+1]=='-'||str[i+1]=='*'||str[i+1]=='/'))
			if ( str[i]=='-'&&str[i+1]=='+')
				str[i+1]=' ';
			else str[i]=' ';
	for(int i=0;i<len;i++){
		if(str[i]==' '||str[i]=='\t'||str[i]=='\n'){
				len--;
				for(int j=i;j<len;j++)
						str[j]=str[j+1];
				str[len]='\x00';
		}
	}
	char oper[MAX_SIZE] = {0};                  /*操作符栈，初始化*/  
	long operand[MAX_SIZE] = {0};                /*数据栈，初始化*/  
	long  top_num = -1;  
	long top_oper = -1;  
	char* temp;  
	long num = 0;  
	long i = 0;  


	int flag=0;
	for(int i=0 ;i<strlen(str);i++){
		if(str[i]<'0'||str[i]>'9'){
			flag=1;
			break;
		}
	}
	if(!flag)
		return atoi(str);
	while(*str != '\0')  
	{  
		temp = dest;
		while(*str !='+' && *str != '-' && *str != '*' && *str != '/' && *str != '(' && *str != ')' && *str != '\x00' && *str != '\n')
		{  
			*temp = *str;  
			 str ++;  
			 temp ++;
		}                               /*遇到符号退出*/  
		
		if(*str != '(' && *(temp - 1) != '\0')      /*判断符号是否为'('*/  
		{  
			 *temp = '\0';  
			 num = atoi(dest);               /*将字符串转为数字*/  
			 insert_operand(operand, &top_num,num);      /*将数据压入数据栈*/  
		}  

		while(1)  
		{  
			i = compare(oper,&top_oper,*str);      /*判断操作符优先级*/  
			if(i == 0)  
			{  
				insert_oper(oper,&top_oper,*str);   /*压入操作符*/  
				break;  
			} 
			else if(i == 1)                         /*判断括号内的表达式是否结束*/  
				str++;
			else if(i == -1)                        /*进行数据处理*/  
				deal_date(operand,oper,&top_num,&top_oper);  
		}  
		str ++;                 /*指向表达式下一个字符*/  
	} 
	return operand[0];                       /*正常退出*/  
} 

int u32(const char *num)
{
	int i; 
	unsigned int tmp = 0;
	for ( i = 0; i <= 3; ++i )
		tmp = *(unsigned char *)(i + num) + (tmp << 8);
	return tmp;
}
char *p32(unsigned int num, char *p)
{
	unsigned int i; 
	int tmp=num;
	char *result;
	for ( i = 0; i <= 3; ++i )
	{
		result = 3 - i + p;
		*result = (char)tmp;
		tmp >>= 8;
	}
	return result;
}
struct user *find_user(char *uuid){
	struct user *tmp=user_head;
	while(tmp){
		if(!strcmp(uuid,tmp->uuid))
			return tmp;
		tmp=tmp->next;
	}
	return NULL;
}
void construct_result(const void *a, int *b, unsigned int n)
{
	int tmp; 
	*b =*(int *)MAGIC_RECV;
	p32(n + 16, (char *)&tmp);
	b[1] = tmp;
	p32(0xBEF2u, (char *)&tmp);
	b[2] = tmp;
	p32(n, (char *)&tmp);
	b[3] = tmp;
	memcpy(b + 4, a, n);
}
int generate_key()
{
	unsigned int buf;
	char *s=malloc(16);
	int fd; 
	fd = open("/dev/urandom", O_RDONLY);
	read(fd, &buf, 4);
	sprintf(s, "%08x", buf);
	while ( find_user(s) ){
		read(fd, &buf, 4);
		sprintf(s, "%08x", buf);
	}
	free(s);
	close(fd);
	return buf;
}
void insert_key(char *user_key){
	struct user *tmp=user_head;
	if(tmp==NULL){
		user_head=(struct user*)malloc(sizeof(struct user)+1);
		user_head->next=NULL;
		// user_head->exp=NULL;
		memcpy(user_head->uuid,user_key,8);
		user_head->uuid[8]='\x00';
		return;
	}
	if(tmp->uuid==user_key)
		return;
	while(tmp->next!=NULL && tmp->next->uuid!=user_key){
		tmp=tmp->next;
	}
	if(tmp->next==NULL){
		tmp->next=(struct user*)malloc(sizeof(struct user)+1);
		tmp->next->next=NULL;
		// tmp->next->exp=NULL;
		memcpy(tmp->next->uuid,user_key,8);
		tmp->next->uuid[8]='\x00';
		return;
	}
	
}

void if_you_forget_expr_id(int len){
	struct user *current_user;
	void *p=malloc(len+1);
	// 
	if(read(0,p,len)!=len)
		exit(-1);
	char *tmp=p;
	int uuid_len=u32(tmp);
	if(uuid_len!=8)
		exit(-1);
	tmp+=4;
	char *uuid=(char *)malloc(uuid_len+1);
	memcpy(uuid,tmp,uuid_len);
	if(!strcmp(uuid,"superusr")){
		exit(1);
	}
	*(uuid+uuid_len)='\x00';
	if(!(current_user = find_user(uuid))){
		write(1,error_packet,0xc);
		return;
	}
	char *res=malloc(0x30);
	sprintf(res,"%ld",current_user->exp->res);
	char *result1=malloc(0x100);
	construct_result(res,(int *)result1,strlen(res));
	write(1,result1,16+strlen(res));
	struct expr *tmpp=current_user->exp;
	current_user->exp=current_user->exp->next;
	free(tmpp->expr_id);
	free(tmpp);
	return;
}

void call_expr(int len){
	struct user *current_user;
	void *p=malloc(len+1);
	// 
	if(read(0,p,len)!=len)
		exit(-1);
	char *tmp=p;
	int uuid_len=u32(tmp);
	if(uuid_len!=8)
		exit(-1);
	tmp+=4;
	char *uuid=(char *)malloc(uuid_len+1);
	memcpy(uuid,tmp,uuid_len);
	uuid[uuid_len]='\x00';
	if(!(current_user = find_user(uuid))){
		write(1,error_packet,0xc);
		return;
	}
	tmp+=uuid_len;
	int corr_id_len=u32(tmp);
	tmp+=4;
	if (corr_id_len<0)
		exit(-1);
	char *corr_id=(char *)malloc(corr_id_len+1);
	memcpy(corr_id,tmp,corr_id_len);
	corr_id[corr_id_len]='\x00';
	tmp+=corr_id_len;
	int expr_len=u32(tmp);
	tmp+=4;
	if (expr_len<0)
		exit(-1);
	char *expr=(char *)malloc(expr_len+1);
	memcpy(expr,tmp,expr_len);
	expr[expr_len]='\x00';
	long result=calculation(expr);
	if(current_user->exp==NULL){
		current_user->exp=malloc(sizeof(struct expr)+1);
		current_user->exp->next=NULL;
		current_user->exp->expr_id=malloc(corr_id_len+1);
		memcpy(current_user->exp->expr_id,corr_id,corr_id_len);
		*(current_user->exp->expr_id+corr_id_len)='\x00';
		current_user->exp->res=result;
	}
	else{
		struct expr *tmp1=current_user->exp;
		while(tmp1->next!=NULL){
			tmp1=tmp1->next;
		}
		tmp1->next=malloc(sizeof(struct expr)+1);
		tmp1->next->next=NULL;
		tmp1->next->expr_id=malloc(corr_id_len+1);
		memcpy(tmp1->next->expr_id,corr_id,corr_id_len);
		*(tmp1->next->expr_id+corr_id_len)='\x00';
		tmp1->next->res=result;
	}
	free(expr);
	free(corr_id);
	free(uuid);
	free(p);
	write(1,done_packet,0xc);
}

static struct user *current_user;

struct thread_struct{
	char * uuid;
	char * corr_id;
	char * answer;
};
void* ReSolve0(void* args){			/*Use thread to complete complicated work simutaneously*/
	if(!(current_user = find_user(((struct thread_struct*)args)->uuid)))
		exit(-1);
	if(!strcmp(current_user->uuid,"superusr")){
		// write(1,retry_packet,0xC);
		((struct thread_struct*)args)->answer=NULL;	
		return NULL;
	}
	sleep(0);
	if(!current_user->exp){
		write(1,retry_packet,0xC);
		((struct thread_struct*)args)->answer=(char*)-1;
		return NULL;
	}
	if(strcmp(current_user->exp->expr_id,((struct thread_struct*)args)->corr_id)){
		write(1,retry_packet,0xC);
		((struct thread_struct*)args)->answer=(char*)-1;
		return NULL;
	}
	((struct thread_struct*)args)->answer=(char*)malloc(0x30);
	sprintf(((struct thread_struct*)args)->answer,"%ld",current_user->exp->res);
	struct expr *tmpp=current_user->exp;
	current_user->exp=current_user->exp->next;
	free(tmpp->expr_id);
	free(tmpp);
}

void get_result(int len){
	void *p=malloc(len+1);
	// read all the data containing uuid_len(4),uuid,corr_id_len(4),corr_id
	if(read(0,p,len)!=len){
		write(1,error_packet,0xc);
		return;
	}

	char *tmp=p;
	int uuid_len=u32(tmp);
	tmp+=4;
	if (uuid_len<0)
		exit(-1);
	char *uuid=(char *)malloc(uuid_len+1);
	memcpy(uuid,tmp,uuid_len);
	uuid[uuid_len]='\x00';
	
	tmp+=uuid_len;
	int corr_id_len=u32(tmp);
	tmp+=4;
	if (corr_id_len<0)
		exit(-1);
	char *corr_id=(char *)malloc(corr_id_len+1);
	memcpy(corr_id,tmp,corr_id_len);
	corr_id[corr_id_len]='\x00';

	char* result1=(char *)malloc(20000);
	char* result2=(char *)malloc(20000);
	char *splitnota;
	memset(result1,0,20000);
	memset(result2,0,20000);
	//split the users by ';'
	int threat_cnt=0;
	pthread_t ppthread[1000];
	struct thread_struct stru[1000]={0};
	char* uuidtmp=uuid;
	while ((splitnota=strstr(uuid,";"))){ 	
		*splitnota='\x00';
		stru[threat_cnt].uuid=uuid;
		stru[threat_cnt].corr_id=corr_id;
		if (pthread_create(&ppthread[threat_cnt],NULL,ReSolve0,(void*)&stru[threat_cnt])){
			perror("pthread_create");
			exit(-1);
		}
		uuid=splitnota+1;
		threat_cnt++;
		if (threat_cnt>=990) break;
	}
	stru[threat_cnt].uuid=uuid;
	stru[threat_cnt].corr_id=corr_id;
	if (pthread_create(&ppthread[threat_cnt],NULL,ReSolve0,(void*)&stru[threat_cnt])){
		perror("Pthread_create");
		exit(-1);
	}
	for (int i=0;i<=threat_cnt;i++)
		pthread_join(ppthread[i],NULL);

	for (int i=0;i<threat_cnt;i++){
		if (!stru[i].answer)
			continue;
		else if(stru[i].answer==(char*)-1)
			goto final;
		strcat(result1,stru[i].answer);
		strcat(result1,";");
		free(stru[i].answer);
	}
	if(stru[threat_cnt].answer==(char*)-1)
		goto final;
	else if (stru[threat_cnt].answer){
		strcat(result1,stru[threat_cnt].answer);
		free(stru[threat_cnt].answer);
	} 
	construct_result(result1,(int *)result2,strlen(result1)); 
	write(1,result2,16+strlen(result1));

	final:
	uuid=uuidtmp;
	free(result2);
	free(result1);
	free(corr_id);
	free(uuid);
	free(p);
}


int verify(char *pwd){
	if(sha256test(pwd) && md5test(pwd))
		return 1;
	else
		return 0;
}

int sha256test(const char *string){
    Sha256Context   sha256Context;
    SHA256_HASH     sha256Hash;
    int        i;
    char str[0x100]="";
    char tmp[3]="";
    Sha256Initialise( &sha256Context );
    Sha256Update( &sha256Context, (unsigned char*)string, (uint32_t)strlen(string) );
    Sha256Finalise( &sha256Context, &sha256Hash );

    for( i=0; i<sizeof(sha256Hash); i++ )
    {
        sprintf( tmp,"%2.2x", sha256Hash.bytes[i] );
        strcat(str,tmp);
    }
    if(strcmp(str,sha256))
    	return 0;
    else
    	return 1;
}

int md5test(const char *string){
    Md5Context      md5Context;
    MD5_HASH        md5Hash;
    int i;
    char str[0x100]="";
    char tmp[3]="";
    Md5Initialise( &md5Context );
    Md5Update( &md5Context, string, (uint32_t)strlen(string) );
    Md5Finalise( &md5Context, &md5Hash );

    for( i=0; i<sizeof(md5Hash); i++ )
    {
        sprintf( tmp,"%2.2x", md5Hash.bytes[i] );
        strcat(str,tmp);
    }
    if(strcmp(str,md5))
    	return 0;
    else
    	return 1;
}

void status_extant_for_checker(int len){
	void *p=malloc(len+1);
	if(read(0,p,len)!=len){
		write(1,error_packet,0xc);
		exit(-1);
	}
	char *tmp=p;
	int pwd_len=u32(tmp);
	tmp+=4;
	char *pwd=(char *)malloc(pwd_len+1);
	memcpy(pwd,tmp,pwd_len);
	if(verify(pwd)){
		struct user *admin=find_user("superusr");
		if(!admin){
			write(1,error_packet,0xc);
			return;
		}
		char *result=malloc(0x200);
		construct_result((char *)admin->exp->res,(int *)result,strlen((char *)admin->exp->res));
		write(1,result,strlen((char *)admin->exp->res)+16);
	}
	else{
		puts("Don't try to break this!");
		exit(1);
	}
	return;
}