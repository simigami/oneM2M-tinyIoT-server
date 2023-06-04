import requests, json

cin_data = {'m2m:cin' : {'con':'test'}}
cin_headers = {'Content-Type' : 'application/json; ty=4'}
cnt_data = {'m2m:cnt' : {'rn':'test'}}
cnt_headers = {'Content-Type' : 'application/json; ty=3'}
sub_data = {'m2m:sub' : {'rn' : 'test_sub', 'enc': { 'net' : [1,2,3] },'nu' : ['http://127.0.0.1:5000?ct=json']}}
sub_headers = {'Content-Type' : 'application/json; ty=23'}

class TestObject:
    def __init__(self, _uri):
        self.host = 'http://192.168.200.141:3000'
        self.uri = _uri

    def requestCIN(self):
        res = requests.post(self.host + self.uri, headers=cin_headers, data=json.dumps(cin_data))

        return res

    def createCIN(self, n):
        print('Trying', n, 'CINs Create....')
        before = json.loads(requests.get(self.host + '/test' + self.uri).content)
        
        for _ in range(n) :
            self.requestCIN()

        after = json.loads(requests.get(self.host + '/test' + self.uri).content)

        print('Result :', int(after['cin-size']) - int(before['cin-size']), ' CINs Created')

    def createCNT(self):
        res = requests.post(self.host + self.uri, headers=cnt_headers, data=json.dumps(cnt_data))

    def retrieveObject(self):
        res = requests.get(self.host + self.uri)
        print(res.content) 
    

testCNT = TestObject('/TinyIoT/test_AE1/test_CNT2')
testCNT.createCIN(1000)


