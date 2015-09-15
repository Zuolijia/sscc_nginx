CORE_DIR=/home/zuolj/nginx-1.9.2/src/core/
EVENT_DIR=/home/zuolj/nginx-1.9.2/src/event/
EVEVT_MODULE_DIR=/home/zuolj/nginx-1.9.2/src/event/modules/
HTTP_DIR=/home/zuolj/nginx-1.9.2/src/http/
HTTP_MODULE_DIR=/home/zuolj/nginx-1.9.2/src/http/modules/
HTTP_MODULE_PERL_DIR=/home/zuolj/nginx-1.9.2/src/http/modules/perl/
OS_UNIX_DIR=/home/zuolj/nginx-1.9.2/src/os/unix/
STREAM_DIR=/home/zuolj/nginx-1.9.2/src/stream/
SSCCTEST_DIR=/home/zuolj/sscctest_module/
SSCCHEADER=/home/zuolj/sscc_handler/
OBJ_DIR=/home/zuolj/nginx-1.9.2/objs/

mylib.so:
	g++ -fpic -shared -g handler_example.cpp -o mylib.so -I $(CORE_DIR) -I $(HTTP_DIR) -I $(SSCCTEST_DIR) -I $(EVENT_DIR) \
	-I $(EVEVT_MODULE_DIR) -I $(HTTP_MODULE_DIR) -I $(HTTP_MODULE_PERL_DIR) -I $(OS_UNIX_DIR) \
	-I $(STREAM_DIR) -I $(OBJ_DIR) -I $(SSCCHEADER)
	
.PHONY: clean

clean:
	rm *.so
