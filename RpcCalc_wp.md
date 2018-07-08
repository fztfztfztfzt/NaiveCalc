# RpcCalc Writeup
## 总体流程
除了实现模板中要求的0-4项功能之外，我们还提供了5 6两个选项，而在第二个选项get_result()中也有一定的修改，导致了第一个漏洞的产生：
5：if_you_forget_your_expr_id()
	主要是实现和选项2：get_result()相同的功能
	但是它减弱了getresult的要求：用户即便不提供，也能直接返回当前用户最顶层（如果存在）的两个结果

6：status_extant_for_checker()
	用来专供checker使用：在验证了checker的身份之后打印出当前内存中应该含有flag的字符串，checker去比较开头是不是"CISCN{"，如果比较失败或者直接SEGSEGV，checker就报错

## Double Fetch
在get_result()中，提供了多用户并发返回结果的支持：用户在传递uuid时，可以将多个用户的uuid用';'分开写到一个请求当中，RPC服务器会分别提取中其中的每个uuid，然后分别开子线程来对uuid和相应的corrid进行结果返回的处理。
但这里存在问题：在子线程中的current_user是在.bss上的全局变量，因此多个线程并发的时候，之前过了superusr的线程中的current_user可能会被后来的线程所修改，导致绕过检查从而能够打印出superusr中corr_id="admin'sflag"的计算结果，也就是flag的地址（以十进制数据打印出来）

## Uninitialized Data
得到flag的地址之后，很容易发现有未初始化变量，即usr结构在创建时，未初始化user->exp，又注意到sizeof(struct user)与sizeof(struct expr) 分别为0x1a与0x18，通过malloc(sizeof(struct user)+1)和malloc(sizeof(struct expr)+1)都会得到0x30的fast chunk 堆空间，而expr中对应于user->exp的是expr->res,所以可以构造一个expr结构体,使其res等于一个地址addr,free掉这个结构体后，再创建一个user,对应的exp则得到addr,此时通过发送类型为5的包，就可以得到起始地址为addr+8的8个字节，这样每次读8个字节，就可以读出flag的值。