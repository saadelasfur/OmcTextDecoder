#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <zlib.h>
#include <OmcTextDecoder.h>

#define CHUNK 16384

static const uint8_t salts[256] = {
    (uint8_t)65, (uint8_t)-59, (uint8_t)33, (uint8_t)-34, (uint8_t)107, (uint8_t)28, (uint8_t)-107, (uint8_t)55,
    (uint8_t)78, (uint8_t)17, (uint8_t)-81, (uint8_t)6, (uint8_t)-80, (uint8_t)-121, (uint8_t)-35, (uint8_t)-23,
    (uint8_t)72, (uint8_t)122, (uint8_t)-63, (uint8_t)-43, (uint8_t)68, (uint8_t)119, (uint8_t)-78, (uint8_t)-111,
    (uint8_t)-60, (uint8_t)31, (uint8_t)60, (uint8_t)57, (uint8_t)92, (uint8_t)-88, (uint8_t)-100, (uint8_t)-69,
    (uint8_t)-106, (uint8_t)91, (uint8_t)69, (uint8_t)93, (uint8_t)110, (uint8_t)23, (uint8_t)93, (uint8_t)53,
    (uint8_t)-44, (uint8_t)-51, (uint8_t)64, (uint8_t)-80, (uint8_t)46, (uint8_t)2, (uint8_t)-4, (uint8_t)12,
    (uint8_t)-45, (uint8_t)80, (uint8_t)-44, (uint8_t)-35, (uint8_t)-111, (uint8_t)-28, (uint8_t)-66, (uint8_t)-116,
    (uint8_t)39, (uint8_t)2, (uint8_t)-27, (uint8_t)-45, (uint8_t)-52, (uint8_t)125, (uint8_t)39, (uint8_t)66,
    (uint8_t)-90, (uint8_t)63, (uint8_t)-105, (uint8_t)-67, (uint8_t)84, (uint8_t)-57, (uint8_t)-4, (uint8_t)-4,
    (uint8_t)101, (uint8_t)-90, (uint8_t)81, (uint8_t)10, (uint8_t)-33, (uint8_t)1, (uint8_t)67, (uint8_t)-57,
    (uint8_t)-71, (uint8_t)18, (uint8_t)-74, (uint8_t)102, (uint8_t)96, (uint8_t)-89, (uint8_t)64, (uint8_t)-17,
    (uint8_t)54, (uint8_t)-94, (uint8_t)-84, (uint8_t)-66, (uint8_t)14, (uint8_t)119, (uint8_t)121, (uint8_t)2,
    (uint8_t)-78, (uint8_t)-79, (uint8_t)89, (uint8_t)63, (uint8_t)93, (uint8_t)109, (uint8_t)-78, (uint8_t)-51,
    (uint8_t)66, (uint8_t)-36, (uint8_t)32, (uint8_t)86, (uint8_t)3, (uint8_t)-58, (uint8_t)-15, (uint8_t)92,
    (uint8_t)58, (uint8_t)2, (uint8_t)-89, (uint8_t)-80, (uint8_t)-13, (uint8_t)-1, (uint8_t)122, (uint8_t)-4,
    (uint8_t)48, (uint8_t)63, (uint8_t)-44, (uint8_t)59, (uint8_t)100, (uint8_t)-42, (uint8_t)-45, (uint8_t)59,
    (uint8_t)-7, (uint8_t)-17, (uint8_t)-54, (uint8_t)34, (uint8_t)-54, (uint8_t)71, (uint8_t)-64, (uint8_t)-26,
    (uint8_t)-87, (uint8_t)-80, (uint8_t)-17, (uint8_t)-44, (uint8_t)-38, (uint8_t)-112, (uint8_t)70, (uint8_t)10,
    (uint8_t)-106, (uint8_t)95, (uint8_t)-24, (uint8_t)-4, (uint8_t)-118, (uint8_t)45, (uint8_t)-85, (uint8_t)-13,
    (uint8_t)85, (uint8_t)25, (uint8_t)-102, (uint8_t)-119, (uint8_t)13, (uint8_t)-37, (uint8_t)116, (uint8_t)46,
    (uint8_t)-69, (uint8_t)59, (uint8_t)42, (uint8_t)-90, (uint8_t)-38, (uint8_t)-105, (uint8_t)101, (uint8_t)-119,
    (uint8_t)-36, (uint8_t)97, (uint8_t)-3, (uint8_t)-62, (uint8_t)-91, (uint8_t)-97, (uint8_t)-125, (uint8_t)17,
    (uint8_t)14, (uint8_t)106, (uint8_t)-72, (uint8_t)-119, (uint8_t)99, (uint8_t)111, (uint8_t)20, (uint8_t)18,
    (uint8_t)-27, (uint8_t)113, (uint8_t)64, (uint8_t)-24, (uint8_t)74, (uint8_t)-60, (uint8_t)-100, (uint8_t)26,
    (uint8_t)56, (uint8_t)-44, (uint8_t)-70, (uint8_t)12, (uint8_t)-51, (uint8_t)-100, (uint8_t)-32, (uint8_t)-11,
    (uint8_t)26, (uint8_t)48, (uint8_t)-117, (uint8_t)98, (uint8_t)-93, (uint8_t)51, (uint8_t)-25, (uint8_t)-79,
    (uint8_t)-31, (uint8_t)97, (uint8_t)87, (uint8_t)-105, (uint8_t)-64, (uint8_t)7, (uint8_t)-13, (uint8_t)-101,
    (uint8_t)33, (uint8_t)-122, (uint8_t)5, (uint8_t)-104, (uint8_t)89, (uint8_t)-44, (uint8_t)-117, (uint8_t)63,
    (uint8_t)-80, (uint8_t)-6, (uint8_t)-71, (uint8_t)-110, (uint8_t)-29, (uint8_t)-105, (uint8_t)116, (uint8_t)107,
    (uint8_t)-93, (uint8_t)91, (uint8_t)-41, (uint8_t)-13, (uint8_t)20, (uint8_t)-115, (uint8_t)-78, (uint8_t)43,
    (uint8_t)79, (uint8_t)-122, (uint8_t)6, (uint8_t)102, (uint8_t)-32, (uint8_t)52, (uint8_t)-118, (uint8_t)-51,
    (uint8_t)72, (uint8_t)-104, (uint8_t)41, (uint8_t)-38, (uint8_t)124, (uint8_t)72, (uint8_t)-126, (uint8_t)-35
};

static const uint8_t shifts[256] = {
    [0] = 1, [1] = 1, [3] = 2, [4] = 2, [5] = 4, [6] = 5, [8] = 4, [9] = 7, [10] = 1, [11] = 6, [12] = 5, [13] = 3, [14] = 3, [15] = 1,
    [16] = 2, [17] = 5, [19] = 6, [20] = 2, [21] = 2, [22] = 4, [23] = 2, [24] = 2, [25] = 3, [27] = 2, [28] = 1, [29] = 2, [30] = 4, [31] = 3,
    [32] = 4, [36] = 3, [37] = 5, [38] = 3, [39] = 1, [40] = 6, [41] = 5, [42] = 6, [43] = 1, [44] = 1, [45] = 1, [48] = 3, [49] = 2, [50] = 7,
    [51] = 7, [52] = 5, [53] = 6, [54] = 7, [55] = 3, [56] = 5, [57] = 1, [59] = 7, [60] = 6, [61] = 3, [62] = 6, [63] = 5, [64] = 4, [65] = 5,
    [66] = 3, [67] = 5, [68] = 1, [69] = 3, [70] = 3, [71] = 1, [72] = 5, [73] = 4, [74] = 1, [77] = 2, [78] = 6, [79] = 6, [80] = 6, [81] = 6,
    [82] = 4, [84] = 1, [85] = 1, [87] = 5, [88] = 5, [89] = 4, [90] = 2, [91] = 4, [92] = 6, [93] = 1, [94] = 7, [95] = 1, [96] = 2, [97] = 1,
    [98] = 1, [99] = 6, [100] = 5, [101] = 4, [102] = 7, [103] = 6, [104] = 5, [105] = 1, [106] = 6, [107] = 7, [109] = 2, [110] = 6, [111] = 3,
    [112] = 1, [113] = 7, [114] = 1, [115] = 1, [116] = 7, [117] = 4, [119] = 4, [120] = 2, [121] = 5, [122] = 3, [123] = 1, [124] = 1, [125] = 5,
    [126] = 6, [128] = 3, [129] = 5, [130] = 3, [131] = 6, [132] = 5, [133] = 7, [134] = 2, [135] = 5, [136] = 6, [137] = 6, [138] = 2, [139] = 2,
    [140] = 3, [141] = 6, [143] = 4, [144] = 3, [145] = 2, [147] = 2, [148] = 2, [149] = 3, [150] = 5, [151] = 3, [152] = 3, [153] = 2, [154] = 5,
    [155] = 5, [156] = 5, [157] = 1, [158] = 3, [159] = 1, [160] = 1, [161] = 1, [162] = 4, [163] = 5, [164] = 1, [165] = 6, [166] = 2, [167] = 4,
    [168] = 7, [169] = 1, [170] = 4, [171] = 6, [173] = 6, [174] = 4, [175] = 3, [176] = 2, [177] = 6, [178] = 1, [179] = 6, [180] = 3, [181] = 2,
    [182] = 1, [183] = 6, [184] = 7, [185] = 3, [186] = 2, [187] = 1, [188] = 1, [189] = 5, [190] = 6, [191] = 7, [192] = 2, [193] = 2, [194] = 2,
    [195] = 7, [196] = 4, [197] = 6, [198] = 7, [199] = 5, [200] = 3, [201] = 1, [202] = 4, [203] = 2, [204] = 7, [205] = 1, [206] = 6, [207] = 2,
    [208] = 4, [209] = 1, [210] = 5, [211] = 6, [212] = 5, [213] = 4, [214] = 5, [216] = 1, [217] = 1, [218] = 6, [219] = 3, [220] = 7, [221] = 2,
    [223] = 2, [224] = 5, [226] = 1, [227] = 3, [228] = 3, [229] = 2, [230] = 6, [231] = 7, [232] = 7, [233] = 2, [234] = 5, [235] = 6, [237] = 4,
    [238] = 1, [239] = 2, [240] = 5, [241] = 3, [242] = 7, [243] = 6, [244] = 5, [245] = 2, [246] = 5, [247] = 2, [249] = 1, [250] = 3, [251] = 1,
    [252] = 4, [253] = 3, [254] = 4, [255] = 2
};

static inline uint8_t rotateLeft(uint8_t val, uint8_t s) {
    if (s == 0)
        return val;
    return (uint8_t)((val << s) | (val >> (8 - s)));
}

static inline uint8_t rotateRight(uint8_t val, uint8_t s) {
    if (s == 0)
        return val;
    return (uint8_t)((val >> s) | (val << (8 - s)));
}

void decodeBuffer(unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uint8_t rotated = rotateLeft(data[i], shifts[i % 256]);
        data[i] = rotated ^ salts[i % 256];
    }
}

void encodeBuffer(unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uint8_t xored = data[i] ^ salts[i % 256];
        data[i] = rotateRight(xored, shifts[i % 256]);
    }
}

unsigned char* decompressGzip(const unsigned char* src, size_t srcLen, size_t* destLen) {
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    strm.next_in = (Bytef*)src;
    strm.avail_in = srcLen;

    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
        return NULL;

    size_t capacity = srcLen * 2 + CHUNK;
    unsigned char* dest = malloc(capacity);
    if (!dest) {
        inflateEnd(&strm);
        return NULL;
    }

    strm.next_out = dest;
    strm.avail_out = capacity;

    while (1) {
        int ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_END)
            break;
        if (ret != Z_OK) {
            free(dest);
            inflateEnd(&strm);
            return NULL;
        }
        if (strm.avail_out == 0) {
            capacity *= 2;
            unsigned char* newDest = realloc(dest, capacity);
            if (!newDest) {
                free(dest);
                inflateEnd(&strm);
                return NULL;
            }
            dest = newDest;
            strm.next_out = dest + strm.total_out;
            strm.avail_out = capacity - strm.total_out;
        }
    }

    *destLen = strm.total_out;
    inflateEnd(&strm);
    return dest;
}

unsigned char* compressGzip(const unsigned char* src, size_t srcLen, size_t* destLen) {
    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    strm.next_in = (Bytef*)src;
    strm.avail_in = srcLen;

    if (deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        return NULL;

    size_t capacity = deflateBound(&strm, srcLen) + 18;
    unsigned char* dest = malloc(capacity);
    if (!dest) {
        deflateEnd(&strm);
        return NULL;
    }

    strm.next_out = dest;
    strm.avail_out = capacity;

    int ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        free(dest);
        deflateEnd(&strm);
        return NULL;
    }

    *destLen = strm.total_out;
    deflateEnd(&strm);
    return dest;
}

int isXmlEncoded(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f)
        return 0;
    char header[256];
    if (fgets(header, sizeof(header), f) == NULL) {
        fclose(f);
        return 0;
    }
    fclose(f);
    if (strstr(header, "<?xml") != NULL)
        return 0;
    return 1;
}

unsigned char* readFile(const char* filename, size_t* length) {
    FILE* f = fopen(filename, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    unsigned char* buf = malloc(size > 0 ? size : 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t readBytes = fread(buf, 1, size, f);
    fclose(f);
    *length = readBytes;
    return buf;
}

void getParentDirectory(const char* path, char* parent) {
    char temp[PATH_MAX];
    snprintf(temp, sizeof(temp), "%s", path);

    for (int i = 0; temp[i]; i++)
        if (temp[i] == '\\')
            temp[i] = '/';

    char* lastSlash = strrchr(temp, '/');
    if (lastSlash) {
        size_t len = lastSlash - temp;
        strncpy(parent, temp, len);
        parent[len] = '\0';
    } else {
        strcpy(parent, "");
    }
}

void makeDirectoryRecursive(const char* path) {
    if (strlen(path) == 0 || strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return;
    char temp[PATH_MAX];
    char* p = NULL;
    size_t len;

    snprintf(temp, sizeof(temp), "%s", path);

    for (int i = 0; temp[i]; i++)
        if (temp[i] == '\\')
            temp[i] = '/';

    len = strlen(temp);
    if (len == 0)
        return;
    if (temp[len - 1] == '/')
        temp[len - 1] = 0;
    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(temp, 0755);
            *p = '/';
        }
    }
    mkdir(temp, 0755);
}

int writeFile(const char* filename, const unsigned char* data, size_t length) {
    char parent[PATH_MAX];
    getParentDirectory(filename, parent);
    makeDirectoryRecursive(parent);

    FILE* f = fopen(filename, "wb");
    if (!f)
        return 0;
    size_t written = fwrite(data, 1, length, f);
    fclose(f);
    return written == length;
}

int isDirectory(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0)
        return 0;
    return S_ISDIR(st.st_mode);
}
