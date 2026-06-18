#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <OmcTextDecoder.h>

void processFile(const char* inFile, const char* outFile, int decodeMode);
void processPath(const char* inPath, const char* outPath, int decodeMode);
void processDirectory(const char* inDir, const char* outDir, int decodeMode);

static char** decodedFiles = NULL;
static int decodedFilesCount = 0;
static int decodedFilesCapacity = 0;
static int hasDecodedList = 0;
static const char* rootInDir = NULL;

static int getRelativePath(const char* root, const char* path, char* relPath, size_t maxLen) {
    char realRoot[PATH_MAX];
    if (!realpath(root, realRoot))
        snprintf(realRoot, sizeof(realRoot), "%s", root);
    for (int i = 0; realRoot[i]; i++)
        if (realRoot[i] == '\\') realRoot[i] = '/';

    char realPath[PATH_MAX];
    if (!realpath(path, realPath))
        snprintf(realPath, sizeof(realPath), "%s", path);
    for (int i = 0; realPath[i]; i++)
        if (realPath[i] == '\\') realPath[i] = '/';

    size_t rootLen = strlen(realRoot);
    if (rootLen > 0 && realRoot[rootLen - 1] == '/') {
        realRoot[rootLen - 1] = '\0';
        rootLen--;
    }

    if (strlen(realPath) >= rootLen && strncasecmp(realPath, realRoot, rootLen) == 0) {
        const char* p = realPath + rootLen;
        while (*p == '/')
            p++;
        snprintf(relPath, maxLen, "%s", p);
        return 1;
    }
    return 0;
}

static void addDecodedFile(const char* relPath) {
    if (decodedFilesCount >= decodedFilesCapacity) {
        decodedFilesCapacity = decodedFilesCapacity == 0 ? 64 : decodedFilesCapacity * 2;
        char** new_arr = realloc(decodedFiles, decodedFilesCapacity * sizeof(char*));
        if (!new_arr)
            return;
        decodedFiles = new_arr;
    }

    char normRel[PATH_MAX];
    snprintf(normRel, sizeof(normRel), "%s", relPath);
    for (int i = 0; normRel[i]; i++)
        if (normRel[i] == '\\')
            normRel[i] = '/';
    decodedFiles[decodedFilesCount++] = strdup(normRel);
}

static void writeDecodedList(const char* outDir) {
    if (decodedFilesCount == 0) return;

    char listPath[PATH_MAX];
    snprintf(listPath, sizeof(listPath), "%s/.decoded", outDir);

    FILE* f = fopen(listPath, "w");
    if (!f) return;

    for (int i = 0; i < decodedFilesCount; i++)
        fprintf(f, "%s\n", decodedFiles[i]);
    fclose(f);
}

static void loadDecodedList(const char* inDir) {
    char listPath[PATH_MAX];
    snprintf(listPath, sizeof(listPath), "%s/.decoded", inDir);

    FILE* f = fopen(listPath, "r");
    if (!f) {
        hasDecodedList = 0;
        return;
    }
    hasDecodedList = 1;

    char line[PATH_MAX];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }
        if (len > 0)
            addDecodedFile(line);
    }
    fclose(f);
}

static int isFileInDecodedList(const char* relPath) {
    if (!hasDecodedList) return 1;

    char normRel[PATH_MAX];
    snprintf(normRel, sizeof(normRel), "%s", relPath);
    for (int i = 0; normRel[i]; i++)
        if (normRel[i] == '\\')
            normRel[i] = '/';

    for (int i = 0; i < decodedFilesCount; i++)
        if (strcasecmp(decodedFiles[i], normRel) == 0)
            return 1;

    return 0;
}

static void deleteDecodedListFile(const char* inDir) {
    char listPath[PATH_MAX];
    snprintf(listPath, sizeof(listPath), "%s/.decoded", inDir);
    unlink(listPath);
}

void processDirectory(const char* inDir, const char* outDir, int decodeMode) {
    DIR* dir = opendir(inDir);
    if (!dir)
        return;
    char realOutDir[PATH_MAX] = {0};
    realpath(outDir, realOutDir);

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (strcmp(entry->d_name, ".decoded") == 0)
            continue;
        char subIn[PATH_MAX];
        char subOut[PATH_MAX];
        snprintf(subIn, sizeof(subIn), "%s/%s", inDir, entry->d_name);

        char realSubIn[PATH_MAX];
        if (realpath(subIn, realSubIn)) {
            if (realOutDir[0] == '\0')
                realpath(outDir, realOutDir);
            if (realOutDir[0] != '\0' && strcmp(realSubIn, realOutDir) == 0)
                continue;
        } else if (strcmp(subIn, outDir) == 0) {
            continue;
        }

        snprintf(subOut, sizeof(subOut), "%s/%s", outDir, entry->d_name);
        processPath(subIn, subOut, decodeMode);
    }
    closedir(dir);
}

void processPath(const char* inPath, const char* outPath, int decodeMode) {
    if (isDirectory(inPath))
        processDirectory(inPath, outPath, decodeMode);
    else
        processFile(inPath, outPath, decodeMode);
}

void processFile(const char* inFile, const char* outFile, int decodeMode) {
    char inFilePath[PATH_MAX];
    if (!realpath(inFile, inFilePath))
        snprintf(inFilePath, sizeof(inFilePath), "%s", inFile);

    char outFilePath[PATH_MAX];
    if (!realpath(outFile, outFilePath))
        snprintf(outFilePath, sizeof(outFilePath), "%s", outFile);

    char relPath[PATH_MAX] = {0};
    int hasRelPath = 0;
    if (rootInDir)
        hasRelPath = getRelativePath(rootInDir, inFile, relPath, sizeof(relPath));

    if (!decodeMode && hasDecodedList) {
        if (hasRelPath && !isFileInDecodedList(relPath)) {
            printf("Skipping %s: not in decoded files list\n", inFilePath);
            return;
        }
    }

    int isEncoded = isFileEncoded(inFile);
    if (decodeMode && !isEncoded) {
        printf("Skipping %s: already decoded.\n", inFilePath);
        return;
    }
    if (!decodeMode && isEncoded) {
        printf("Skipping %s: already encoded.\n", inFilePath);
        return;
    }

    size_t inLen = 0;
    unsigned char* inData = readFile(inFile, &inLen);
    if (!inData) {
        fprintf(stderr, "Failed to read input file: %s\n", inFilePath);
        return;
    }

    unsigned char* outData = NULL;
    size_t outLen = 0;

    if (decodeMode) {
        decodeBuffer(inData, inLen);
        outData = decompressGzip(inData, inLen, &outLen);
        free(inData);
        if (!outData)
            return;
    } else {
        unsigned char* compressed = compressGzip(inData, inLen, &outLen);
        free(inData);
        if (!compressed) {
            fprintf(stderr, "Failed to compress gzip data for: %s\n", inFilePath);
            return;
        }
        encodeBuffer(compressed, outLen);
        outData = compressed;
    }

    if (writeFile(outFile, outData, outLen)) {
        printf("Output %s: success\n", outFilePath);
        if (decodeMode && rootInDir && hasRelPath)
            addDecodedFile(relPath);
    } else {
        fprintf(stderr, "Failed to write output file: %s\n", outFilePath);
    }
    free(outData);
}

void getDefaultOutput(const char* inputPath, char* outputPath, size_t maxLen) {
    char parent[PATH_MAX];
    getParentDirectory(inputPath, parent);

    char temp[PATH_MAX];
    snprintf(temp, sizeof(temp), "%s", inputPath);
    for (int i = 0; temp[i]; i++)
        if (temp[i] == '\\')
            temp[i] = '/';

    const char* filename = strrchr(temp, '/');
    if (filename)
        filename++;
    else
        filename = temp;

    if (strlen(parent) > 0)
        snprintf(outputPath, maxLen, "%s/out_%s", parent, filename);
    else
        snprintf(outputPath, maxLen, "out_%s", filename);
}

void printVersion() {
    printf("OmcTextDecoder version %s\n", VERSION);
}

void printHelp(const char* progName) {
    printf("usage: %s -i cscfeature.xml -o cscfeature_decoded.xml\n", progName);
    printf(" -h, --help           print this message\n");
    printf(" -v, --version        print version\n");
    printf(" -e, --encode         encode omc file\n");
    printf(" -d, --decode         decode omc file\n");
    printf(" -i, --input <arg>    input file or dir\n");
    printf(" -o, --output <arg>   output file or dir\n");
    printf(" -p, --in-place       edit file(s) in place\n");
}

int main(int argc, char* argv[]) {
    int decodeMode = 1;
    char* inputPath = NULL;
    char* outputPath = NULL;
    char defaultOut[PATH_MAX] = {0};
    int inPlace = 0;

    static struct option longOptions[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"encode", no_argument, 0, 'e'},
        {"decode", no_argument, 0, 'd'},
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"in-place", no_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    int opt;
    int optionIndex = 0;
    while ((opt = getopt_long(argc, argv, "hvedi:o:p", longOptions, &optionIndex)) != -1) {
        switch (opt) {
            case 'h':
                printHelp(argv[0]);
                return 0;
            case 'v':
                printVersion();
                return 0;
            case 'e':
                decodeMode = 0;
                break;
            case 'd':
                decodeMode = 1;
                break;
            case 'i':
                inputPath = optarg;
                break;
            case 'o':
                outputPath = optarg;
                break;
            case 'p':
                inPlace = 1;
                break;
            default:
                printHelp(argv[0]);
                return 1;
        }
    }

    if (!inputPath) {
        printHelp(argv[0]);
        return 1;
    }

    if (inPlace && outputPath) {
        fprintf(stderr, "Error: Cannot use both --in-place (-p) and --output (-o)\n");
        return 1;
    }

    if (inPlace) {
        outputPath = inputPath;
    } else if (!outputPath) {
        getDefaultOutput(inputPath, defaultOut, sizeof(defaultOut));
        outputPath = defaultOut;
    }

    int inputIsDir = isDirectory(inputPath);
    if (inputIsDir)
        rootInDir = inputPath;

    if (!decodeMode && inputIsDir)
        loadDecodedList(inputPath);

    processPath(inputPath, outputPath, decodeMode);

    if (decodeMode && inputIsDir)
        writeDecodedList(outputPath);

    if (!decodeMode && inputIsDir && hasDecodedList)
        deleteDecodedListFile(inputPath);

    for (int i = 0; i < decodedFilesCount; i++)
        free(decodedFiles[i]);
    free(decodedFiles);

    return 0;
}
