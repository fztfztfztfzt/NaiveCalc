#!/usr/bin/python3
import my_client
import random
import time
SYMBOLS = [
    '+',
    '-',
    '*',
    '/',
]

def test_case(func):
    def inner_func(*args, **kwargs):
        print('running %s' % func.__name__)
        func(*args, **kwargs)
    return inner_func

def rand():
    return random.randint(0, 10000)


def gen_add_expr():
    return '{} + {}'.format(
        rand(),
        rand()
    )


def gen_random_expr():
    while True :
        expr = str(rand())
        for i in range(random.randint(0, 10)):
            expr += '{} {}'.format(
                random.choice(SYMBOLS),
                rand()
            )
                
        true = int(eval(expr))
        if(expr.count('*')<5) :
            break
    
    return expr


@test_case
def basic_test():
    client = my_client.RpcClient('localhost')
    expr = '{} * {} + {} - {} * ({} + {})'.format(
        rand(),
        rand(),
        rand(),
        rand(),
        rand(),
        rand(),
    )
    res = client.call(expr)
    
    assert(res == int(eval(expr)))
    client.close()


@test_case
def test_flag():
    client = my_client.RpcClient('localhost')
    ###############
    pwd=b'Ph0t1n1a\'S L0NG LONG PassW0rD'
    sttr=b'RPCM'
    sttrr=my_client.p32(6)
    dat=my_client.p32(len(pwd))+pwd
    check=sttr+my_client.p32(12+len(dat))+sttrr+dat
    res=client.conn.send_with_result(check)
    if(str(res.result_bytes).count('CISCN{')==0):
        raise my_client.NO_FLAG_EXCEPTION("The flag is ruined!")
    client.close()
    ###############
@test_case
def test_func5():
    client = my_client.RpcClient('localhost')
    expr = gen_add_expr()
    id1 = client.call_request(expr)
    sttr=b'RPCM'
    sttrr=my_client.p32(5)
    dat=my_client.p32(len(client.reply_to))+client.reply_to.encode('utf-8')
    check=sttr+my_client.p32(12+len(dat))+sttrr+dat
    res=client.conn.send_with_result(check)
    assert int(res.result_bytes.decode('utf-8'))==eval(expr)        
    client.close()

@test_case
def test_multi_user():
    corrid='phot'
    conn=my_client.RpcConnection('localhost',1337)
    uuid1=conn.send_with_result(b'RPCM\x00\x00\x00\x0c\x00\x00\x00\x01')
    uuid2=conn.send_with_result(b'RPCM\x00\x00\x00\x0c\x00\x00\x00\x01')
    assert (uuid1.packet_type==b'\x00\x00\xbe\xf2' and uuid2.packet_type==b'\x00\x00\xbe\xf2')
    uuid1=uuid1.result_bytes
    uuid2=uuid2.result_bytes
    expr1=gen_add_expr()
    expr2=gen_add_expr()
    ret1=conn.send_with_type(b'RPCM'+my_client.p32(len(expr1)+12+12+8+4)+b'\x00\x00\x00\x03\x00\x00\x00\x08'+uuid1+b'\x00\x00\x00\x04phot'+my_client.p32(len(expr1))+bytes(expr1, encoding = "utf8"))
    ret2=conn.send_with_type(b'RPCM'+my_client.p32(len(expr2)+12+12+8+4)+b'\x00\x00\x00\x03\x00\x00\x00\x08'+uuid1+b'\x00\x00\x00\x04phot'+my_client.p32(len(expr2))+bytes(expr2, encoding = "utf8"))
    pactype,data=conn.send_with_raw_reply(b'RPCM\x00\x00\x00)\x00\x00\x00\x02\x00\x00\x00\x11'+uuid1+b';'+uuid1+b'\x00\x00\x00\x04phot')
    # print(str(data[4:],encoding = "utf8"))
    # print(str(eval(expr1))+';'+str(eval(expr2)))
    assert (pactype==b'\x00\x00\xbe\xf1' or (pactype== b'\x00\x00\xbe\xf2' and  str(data[4:],encoding = "utf8")==str(eval(expr2))+';'+str(eval(expr1))) or ((pactype== b'\x00\x00\xbe\xf2' and  str(data[4:],encoding = "utf8")==str(eval(expr1))+';'+str(eval(expr2)))))
    conn.close()
    
@test_case
def basic_queue_test():
    client = my_client.RpcClient('localhost')
    expr1 = gen_add_expr()
    expr2 = gen_add_expr()
    id1 = client.call_request(expr1)
    id2 = client.call_request(expr2)
    time.sleep(0.1)
    try:
        client.try_retrieve(id2)
        assert(False)
    except my_client.RpcPacketUnavailableException:
        pass
    res = client.try_retrieve(id1)
    assert(int(res.result_bytes) == eval(expr1))
    res = client.try_retrieve(id2)
    assert(int(res.result_bytes) == eval(expr2))
    client.close()


@test_case
def random_queue_test(client=None):
    if client is None:
        client = my_client.RpcClient('localhost')
    queue = []
    for i in range(random.randint(5, 20)):
        if random.random() < 0.25 and len(queue) > 0:
            time.sleep(1)
            cur_head = queue[0]
            cur_id = cur_head[0]
            cur_expr = cur_head[1]
            res = client.try_retrieve(cur_id)
            assert(int(res.result_bytes) == eval(cur_expr))
            queue = queue[1:]
        else:
            expr=gen_random_expr()
            queue.append((client.call_request(expr), expr.replace('/', '//')))
    while len(queue) > 0:
        time.sleep(1)
        cur_head = queue[0]
        cur_id = cur_head[0]
        cur_expr = cur_head[1]
        res = client.try_retrieve(cur_id)
        assert(int(res.result_bytes) == eval(cur_expr))
        queue = queue[1:]

    client.close()


def random_queue_stoppable_test(client=None):
    if client is None:
        client = my_client.RpcClient('localhost')
    queue = []
    for i in range(random.randint(5, 20)):
        if random.random() < 0.5:
            yield
        if random.random() < 0.25 and len(queue) > 0:
            time.sleep(1)
            cur_head = queue[0]
            cur_id = cur_head[0]
            cur_expr = cur_head[1]
            res = client.try_retrieve(cur_id)
            assert(int(res.result_bytes) == eval(cur_expr))
            queue = queue[1:]
        else:
            expr = gen_random_expr()
            queue.append((client.call_request(expr), expr.replace('/', '//')))
    while len(queue) > 0:
        time.sleep(1)
        cur_head = queue[0]
        cur_id = cur_head[0]
        cur_expr = cur_head[1]
        res = client.try_retrieve(cur_id)
        assert(int(res.result_bytes) == eval(cur_expr))
        queue = queue[1:]

    client.close()


@test_case
def seperated_provider_test():
    client1 = my_client.RpcClient('localhost')
    client2 = my_client.RpcClient('localhost')
    random_queue_test(client1)
    random_queue_test(client2)


@test_case
def crossed_provider_test():
    client1 = my_client.RpcClient('localhost')
    client2 = my_client.RpcClient('localhost')
    test1 = random_queue_stoppable_test(client1)
    test2 = random_queue_stoppable_test(client2)
    next(test1)
    next(test2)

    stop1 = False
    stop2 = False
    while not stop1 or not stop2:
        if not stop1:
            try:
                print('executing thread1')
                next(test1)
            except StopIteration:
                stop1 = True
        if not stop2:
            try:
                print('executing thread2')
                next(test2)
            except StopIteration:
                stop2 = True



def test_all():
    test_flag()
    test_func5()
    basic_test()
    basic_queue_test()
    random_queue_test()
    seperated_provider_test()
    test_multi_user()
    crossed_provider_test()

if __name__ == '__main__':
        test_all()
