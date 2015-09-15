SSCCTEST_HEADER=/home/zuolj/sscc_handler/

real_handler_test:
	g++ -fpic -shared -g realhandler_test.cpp -o ip.so -I $(SSCCTEST_HEADER)
	g++ -fpic -shared -g realhandler_test2.cpp -o expires.so -I $(SSCCTEST_HEADER)
	
.PHONY: clean

clean:
	rm *.so
