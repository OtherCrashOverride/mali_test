all:
	g++ -g -std=c++11 -O2 test_window.cpp X11Window.cpp Egl.cpp -o test_window -L/usr/lib/arm-linux-gnueabihf/mali-egl/ -lmali -lX11
	g++ -g -std=c++11 -O2 test_egl_ext_image_dma_buf_import.cpp X11Window.cpp Egl.cpp Matrix4.cpp Vector3.cpp -o test_egl_ext_image_dma_buf_import -I/usr/include/pixman-1/ -I/usr/include/X11/dri/ -L/usr/lib/arm-linux-gnueabihf/mali-egl/ -lmali -lX11 -ldri2 `pkg-config --cflags --libs libdrm`
	gcc -o eglinfo eglinfo.c -L/usr/lib/arm-linux-gnueabihf/mali-egl/ -lmali
