import os
from time import sleep

while(1):
	sleep(1)
	result = os.popen('free').read()
	result = result[106:116]
	print(result)
