import requests
import json
import time
import multiprocessing

# target_ip = "192.168.0.96"
# target_ip = "127.0.0.1"
target_ip = "192.168.1.44"
target_port = 3000

# target_ip = "34.22.78.31"
# target_port = 7579

# cb = "Mobius"
cb = "TinyIoT"
AE_name = "ae_test"
AE_Origin = "SOrigin"
CNT_name = "cnt_test"

process_arr = []

def deleteAE():
    url = f'http://{target_ip}:{target_port}/{cb}/{AE_name}'
    headers = {'Accept': 'application/json', 'X-M2M-RI': '12345', 'X-M2M-Origin': f'{AE_Origin}'}
    res = requests.delete(url, headers=headers)
    if res.status_code == 200:
        return True
    else:
        return False

def setup():
    deleteAE()
    if makeAE() is not True:
        print("AE 생성 실패")
        exit(1)
    if makeCNT() is not True:
        print("CNT 생성 실패")
        exit(1)

    pass

def makeAE():
    url = f'http://{target_ip}:{target_port}/{cb}'
    headers = {'Content-Type': 'application/json;ty=2', 'Accept': 'application/json', 'X-M2M-RI': '12345', 'X-M2M-Origin': f'{AE_Origin}'}
    data = {'m2m:ae': {'api': 'NmyApp1', 'rr': True, 'rn': f'{AE_name}'}}
    res = requests.post(url, headers=headers, data=json.dumps(data))
    if res.status_code == 201:
        return True
    else:
        return False


def makeCNT():
    url = f'http://{target_ip}:{target_port}/{cb}/{AE_name}'
    headers = {'Content-Type': 'application/json;ty=3', 'Accept': 'application/json', 'X-M2M-RI': '12345', 'X-M2M-Origin': f'{AE_Origin}'}
    data = {'m2m:cnt': {'rn': f'{CNT_name}', 'mni': 1100}}
    res = requests.post(url, headers=headers, data=json.dumps(data))

    if res.status_code == 201:
        return True
    else:
        return False


def makeCIN():
    url = f'http://{target_ip}:{target_port}/{cb}/{AE_name}/{CNT_name}'
    headers = {'Content-Type': 'application/json;ty=4', 'Accept': 'application/json', 'X-M2M-RI': '12345', 'X-M2M-Origin': f'{AE_Origin}'}
    data = {"m2m:cin": {"con": "Hello, CIN!"}}
    data = json.dumps(data)
    start_time = time.time()
    res = requests.post(url, headers=headers, data=data)
    end_time = time.time()
    return end_time - start_time


def runTest(cnt:int, proc_idx:int, queue: multiprocessing.Queue):
    time_arr = []
    for _ in range(cnt):
        time_arr.append(makeCIN())
    queue.put(time_arr)

            


if __name__ == "__main__":
    cin_cnt = 100
    process_cnt = 5
    setup()
    time_arr = {}
    total_time = []
    if process_cnt > 1:
        for i in range(process_cnt):
            time_arr[i] = multiprocessing.Queue()
            process_arr.append(multiprocessing.Process(target=runTest, args=(cin_cnt, i, time_arr[i])))
            process_arr[i].start()


        for i in range(process_cnt):
            process_arr[i].join()
            temp = time_arr[i].get()
            print(f"Process {i}")
            print(f"\tmin: {min(temp)}")
            print(f"\tmax: {max(temp)}")
            print(f"\tavg: {sum(temp) / len(temp)}")
            total_time += temp
    else:
        for i in range(cin_cnt):
            total_time.append(makeCIN())

    print(f"total")
    print(f"\tmin: {min(total_time)}")
    print(f"\tmax: {max(total_time)}")
    print(f"\tavg: {sum(total_time) / len(total_time)}")