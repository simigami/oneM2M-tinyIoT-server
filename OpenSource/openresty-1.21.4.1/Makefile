.PHONY: all install clean

all:
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/LuaJIT-2.1-20220411 && $(MAKE) TARGET_STRIP=@: CCDEBUG=-g XCFLAGS='-DLUAJIT_ENABLE_LUA52COMPAT' CC=cc PREFIX=/usr/local/openresty/luajit
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-cjson-2.1.0.10 && $(MAKE) DESTDIR=$(DESTDIR) LUA_INCLUDE_DIR=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/luajit-root/usr/local/openresty/luajit/include/luajit-2.1 LUA_CMODULE_DIR=/usr/local/openresty/lualib LUA_MODULE_DIR=/usr/local/openresty/lualib CJSON_CFLAGS="-g -fpic" CC=cc
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-signal-0.03 && $(MAKE) DESTDIR=$(DESTDIR) LUA_INCLUDE_DIR=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/luajit-root/usr/local/openresty/luajit/include/luajit-2.1 LUA_LIB_DIR=/usr/local/openresty/lualib CC=cc
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-redis-parser-0.13 && $(MAKE) DESTDIR=$(DESTDIR) LUA_INCLUDE_DIR=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/luajit-root/usr/local/openresty/luajit/include/luajit-2.1 LUA_LIB_DIR=/usr/local/openresty/lualib CC=cc
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-rds-parser-0.06 && $(MAKE) DESTDIR=$(DESTDIR) LUA_INCLUDE_DIR=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/luajit-root/usr/local/openresty/luajit/include/luajit-2.1 LUA_LIB_DIR=/usr/local/openresty/lualib CC=cc
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/nginx-1.21.4 && $(MAKE)

install: all
	mkdir -p $(DESTDIR)/usr/local/openresty/
	-cp /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/COPYRIGHT $(DESTDIR)/usr/local/openresty/
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/LuaJIT-2.1-20220411 && $(MAKE) install TARGET_STRIP=@: CCDEBUG=-g XCFLAGS='-DLUAJIT_ENABLE_LUA52COMPAT' CC=cc PREFIX=/usr/local/openresty/luajit DESTDIR=$(DESTDIR)
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-cjson-2.1.0.10 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_INCLUDE_DIR=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/luajit-root/usr/local/openresty/luajit/include/luajit-2.1 LUA_CMODULE_DIR=/usr/local/openresty/lualib LUA_MODULE_DIR=/usr/local/openresty/lualib CJSON_CFLAGS="-g -fpic" CC=cc
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-signal-0.03 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_INCLUDE_DIR=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/luajit-root/usr/local/openresty/luajit/include/luajit-2.1 LUA_LIB_DIR=/usr/local/openresty/lualib CC=cc
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-redis-parser-0.13 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_INCLUDE_DIR=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/luajit-root/usr/local/openresty/luajit/include/luajit-2.1 LUA_LIB_DIR=/usr/local/openresty/lualib CC=cc
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-rds-parser-0.06 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_INCLUDE_DIR=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/luajit-root/usr/local/openresty/luajit/include/luajit-2.1 LUA_LIB_DIR=/usr/local/openresty/lualib CC=cc
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-dns-0.22 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-memcached-0.16 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-redis-0.30 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-mysql-0.25 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-string-0.15 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-upload-0.10 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-websocket-0.09 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-lock-0.08 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-lrucache-0.11 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-core-0.1.23 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-upstream-healthcheck-0.06 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-limit-traffic-0.08 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-resty-shell-0.03 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/lua-tablepool-0.02 && $(MAKE) install DESTDIR=$(DESTDIR) LUA_LIB_DIR=/usr/local/openresty/lualib INSTALL=/home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/opm-0.0.6 && /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install bin/* $(DESTDIR)/usr/local/openresty/bin/
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/resty-cli-0.28 && /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/install bin/* $(DESTDIR)/usr/local/openresty/bin/
	cp /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/resty.index $(DESTDIR)/usr/local/openresty/
	cp -r /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/pod $(DESTDIR)/usr/local/openresty/
	cd /home/eom/Desktop/Sejong-IoT-Project/OpenSource/openresty-1.21.4.1/build/nginx-1.21.4 && $(MAKE) install DESTDIR=$(DESTDIR)
	mkdir -p $(DESTDIR)/usr/local/openresty/site/lualib $(DESTDIR)/usr/local/openresty/site/pod $(DESTDIR)/usr/local/openresty/site/manifest
	ln -sf /usr/local/openresty/nginx/sbin/nginx $(DESTDIR)/usr/local/openresty/bin/openresty

clean:
	rm -rf build *.exe *.dll openresty-*
