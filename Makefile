
CC=/usr/bin/cc -w
LDFLAGS=-lcrypto -lpynq -lcma -lpthread -lm

SOURCES_SHA2_SW = sha2/sha2_sw/crypto.c sha2/sha2_sw/sha2_224.c sha2/sha2_sw/sha2_256.c sha2/sha2_sw/sha2_384.c sha2/sha2_sw/sha2_512.c sha2/sha2_sw/sha2_512_224.c sha2/sha2_sw/sha2_512_256.c
HEADERS_SHA2_SW = sha2/sha2_sw/crypto.h sha2/sha2_sw/sha2_224.h sha2/sha2_sw/sha2_256.h sha2/sha2_sw/sha2_384.h sha2/sha2_sw/sha2_512.h sha2/sha2_sw/sha2_512_224.h sha2/sha2_sw/sha2_512_256.h

SOURCES_SHA2 = sha2/sha2_hw.c sha2/sha2_sw.c
HEADERS_SHA2 = sha2/sha2_hw.h sha2/sha2_sw.h

SOURCES_SHA3 = sha3/sha3.c sha3/sha3_hw.c sha3/sha3_sw.c
HEADERS_SHA3 = sha3/sha3.h sha3/sha3_hw.h sha3/sha3_sw.h

SOURCES_COMMON = common/Funciones_HW.c common/Funcion_Test.c common/xor.c
HEADERS_COMMON = common/Funciones_HW.h common/Funcion_Test.h common/xor.h

SOURCES_PUF = puf/mmio.c puf/puf4r4.c puf/puf.c
HEADERS_PUF = puf/mmio.h puf/puf4r4.h puf/puf.h

SOURCES_HMAC = HMAC.c demo.c
HEADERS_HMAC = HMAC.h

all: HMAC

HMAC: $(HEADERS_SHA2_SW) $(HEADERS_SHA2) $(HEADERS_SHA3) $(HEADERS_PUF) $(HEADERS_COMMON) $(HEADERS_HMAC) $(SOURCES_SHA2_SW) $(SOURCES_SHA2) $(SOURCES_SHA3) $(SOURCES_PUF) $(SOURCES_COMMON) $(SOURCES_HMAC)
	$(CC) -o $@ $(SOURCES_SHA2_SW) $(SOURCES_SHA2) $(SOURCES_SHA3) $(SOURCES_PUF) $(SOURCES_COMMON) $(SOURCES_HMAC) $(LDFLAGS)

.PHONY: all HMAC clean
clean:
	-rm HMAC
