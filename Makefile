MD5_DIR = ./md5
TARGET = jpush
SOURCE = jpush.o $(MD5_DIR)/md5.o

$(TARGET):$(SOURCE)
	gcc -o $(TARGET) $(SOURCE)


.PHONY:clean
clean:
	rm $(TARGET) $(SOURCE)
