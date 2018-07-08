#ifndef SYMBOL
#define SYMBOL

typedef struct oper_symbol
{
    char oper; //运算符
    unsigned int index;//运算符在表达式中的位置索引
}oper_symbol;
struct user{
  struct user *next;
  struct expr *exp;
  char uuid[10];
};
struct expr{
  struct expr *next;
  long res;
  char *expr_id;
};

int u32(const char *num);
char  *p32(unsigned int num, char *p);
struct user *find_user(char *uuid);
void construct_result(const void *a, int *b, unsigned int n);
int generate_key(void);
void insert_key(char *user_key);
void call_expr(int len);
void get_result(int len);

double handle(char *string);                      
int error_detection(char *string);                
double bracket(char *string);                     
void muliply_divide(char *string);                
double add_subtract(char *string);                
int arithmetic_character(char c);                 
double front_n(int *First_location,char *string); 
double behind_n(int *Second_location,char *string);
void if_you_forget_expr_id(int len);
long calculation(char *expr);
void status_extant_for_checker(int len);
int md5test(const char *string);
int sha256test(const char *string);
int verify(char *pwd);

static int forget_times;
struct user *user_head;

#endif