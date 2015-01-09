#include "fmibinaryreader.h"

/* uint*_t */
#include <stdint.h>
/* memcpy */
#include <string.h>

#include <type_traits>
#include <functional>
#include <limits>
#include <sserialize/utility/utilmath.h>

/* make sure be32toh and be64toh are present */
#if defined(__linux__)
#  if defined(__ANDROID__)
#    include <sys/endian.h>
#    define be16toh(x) betoh16(x)
#    define be32toh(x) betoh32(x)
#    define be64toh(x) betoh64(x)
#  else
#    include <endian.h>
#  endif
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
#  define be16toh(x) betoh16(x)
#  define be32toh(x) betoh32(x)
#  define be64toh(x) betoh64(x)
#else
/* htons/htonl */
#include <arpa/inet.h>
#  define htobe16(x) htons(x)
#  define be16toh(x) htons(x)
#  define htobe32(x) htonl(x)
#  define be32toh(x) htonl(x)
#  define htobe64(x) my_htobe64(x)
#  define be64toh(x) htobe64(x)
static uint64_t my_htobe64(uint64_t x) {
	if (htobe32(1) == 1) return x;
	return ((uint64_t)htobe32(x)) << 32 | htobe32(x >> 32);
}

#endif
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>

namespace OsmGraphWriter {

FmiBinaryReader::FmiBinaryReader() {}
FmiBinaryReader::~FmiBinaryReader() {}
bool FmiBinaryReader::read(char * path) {
	int fd = open(path, O_RDONLY);
	off_t fileSize = 0;
	if (fd < 0) {
		return false;
	}
	struct ::stat stFileInfo;
	if (::fstat(fd,&stFileInfo) == 0) {
		fileSize = stFileInfo.st_size;
	}
	else {
		return false;
	}

	int param = MAP_SHARED;
	void * data = ::mmap(0, fileSize, PROT_READ | PROT_WRITE, param, fd, 0);

	if (data == MAP_FAILED) {
		::close(fd);
		return false;
	}
	
	bool ok = readGraph((uint8_t*)data, ((uint8_t*)data)+fileSize);
	
	::munmap(data, fileSize);
	close(fd);
	return ok;
}

int32_t FmiBinaryReader::getInt32(uint8_t*& offset) {
	int32_t tmp;
	memmove(&tmp, offset, 4);
	offset += 4;
	return be32toh(tmp);
}

int64_t FmiBinaryReader::getInt64(uint8_t*& offset) {
	int64_t tmp;
	memmove(&tmp, offset, 8);
	offset += 4;
	return be64toh(tmp);
}

double FmiBinaryReader::getDouble(uint8_t*& offset) {
	union {
		double d;
		int64_t i;
	} tmp;
	tmp.i = getInt64(offset);
	return tmp.d;
}

}//end namespace
