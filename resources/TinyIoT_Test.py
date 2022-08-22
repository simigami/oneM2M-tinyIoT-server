import requests, json

data = {'m2m:cin' : {'con':'test'}}
headers = {'Content-Type' : 'application/json; ty=4'}

class TestObject:
    def __init__(self, _uri):
        self.host = 'http://192.168.200.141:3000'
        self.uri = _uri

    def createCIN(self):
        res = requests.post(self.host + self.uri, headers=headers, data=json.dumps(data))

    def cinSizeisEqualsTo(self, n):
        res = requests.get(self.host + '/test' + self.uri)
        if int(res.content) == n :
            print('OK')
    

testCNT = TestObject('/TinyIoT/test_AE1/test_CNT1')
