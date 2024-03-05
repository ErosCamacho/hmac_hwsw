
CC=/usr/bin/cc -w
LDFLAGS=-lcrypto -lpynq -lcma -lpthread -lm

ROT_FOLDER = RoT/

SOURCES_SHA2_SW = RoT/sha2/sha2_sw/crypto.c RoT/sha2/sha2_sw/sha2_224.c RoT/sha2/sha2_sw/sha2_256.c RoT/sha2/sha2_sw/sha2_384.c RoT/sha2/sha2_sw/sha2_512.c RoT/sha2/sha2_sw/sha2_512_224.c RoT/sha2/sha2_sw/sha2_512_256.c
HEADERS_SHA2_SW = RoT/sha2/sha2_sw/crypto.h RoT/sha2/sha2_sw/sha2_224.h RoT/sha2/sha2_sw/sha2_256.h RoT/sha2/sha2_sw/sha2_384.h RoT/sha2/sha2_sw/sha2_512.h RoT/sha2/sha2_sw/sha2_512_224.h RoT/sha2/sha2_sw/sha2_512_256.h

SOURCES_SHA2 = RoT/sha2/sha2_hw.c RoT/sha2/sha2_sw.c
HEADERS_SHA2 = RoT/sha2/sha2_hw.h RoT/sha2/sha2_sw.h

SOURCES_SHA3 = RoT/sha3/sha3.c RoT/sha3/sha3_hw.c RoT/sha3/sha3_sw.c
HEADERS_SHA3 = RoT/sha3/sha3.h RoT/sha3/sha3_hw.h RoT/sha3/sha3_sw.h

SOURCES_COMMON = RoT/common/Funciones_HW.c RoT/common/Funcion_Test.c RoT/common/xor.c
HEADERS_COMMON = RoT/common/Funciones_HW.h RoT/common/Funcion_Test.h RoT/common/xor.h

SOURCES_PUF = RoT/puf/mmio.c RoT/puf/puf4r4.c RoT/puf/puf.c
HEADERS_PUF = RoT/puf/mmio.h RoT/puf/puf4r4.h RoT/puf/puf.h

SOURCES_ROT = $(SOURCES_SHA2_SW) $(SOURCES_SHA2) $(SOURCES_SHA3) $(SOURCES_PUF) $(SOURCES_COMMON) $(SOURCES_HMAC)
HEADERS_ROT = $(HEADERS_SHA2_SW) $(HEADERS_SHA2) $(HEADERS_SHA3) $(HEADERS_PUF) $(HEADERS_COMMON) $(HEADERS_HMAC)

SOURCES_HMAC = HMAC.c
HEADERS_HMAC = HMAC.h

all: HMAC

HMAC:	$(SOURCES_ROT) $(HEADERS_ROT)
		$(CC) -o $@ $(SOURCES_ROT) demo.c $(LDFLAGS)

.PHONY: all HMAC clean
clean:
	-rm HMAC
