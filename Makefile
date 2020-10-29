CC = gcc
CFLAGS = -Isrc/api -Isrc/cmn -Isrc/core/ibv -Wall -Werror -ldl -D__LINUX__ -D_GNU_SOURCE -fPIC
TMP_OBJS = src/api/ibtrace_api.o src/cmn/ibtrace_cmn.o src/cmn/ibtrace_logger.o src/core/ibv/ibtrace_ibv.o 
TARGET = libibtrace.so

$(TARGET): $(TMP_OBJS)
	$(CC) -shared -o $(TARGET) $(TMP_OBJS) $(CFLAGS)

clean:
	-$(RM) $(TARGET) $(TMP_OBJS)