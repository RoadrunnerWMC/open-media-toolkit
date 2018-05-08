#include <string.h>
#include <GL/gl.h>
#include <GL/wglext.h>

#define CHECK_MEMORY(ptr) \
    if (NULL == ptr) { \
		printf("Error allocating memory in file %s, line %d\n", __FILE__, __LINE__); \
		exit(-1); \
	}

#ifdef GLH_EXT_SINGLE_FILE
    #include "glh_genext.h"
	static char *unsupportedExts = NULL;
#endif

static int ExtensionExists(const char* extName, const char* sysExts)
{
    char *padExtName = (char*)malloc(strlen(extName) + 2);
    strcat(strcpy(padExtName, extName), " ");

    if (strstr(sysExts, padExtName)) {
		free(padExtName);
        return TRUE;
    } else {
		free(padExtName);
        return FALSE;
    }
}

static const char* EatWhiteSpace(const char *str)
{
	for (; *str && (' ' == *str || '\t' == *str || '\n' == *str); str++);
	return str;
}

static const char* EatNonWhiteSpace(const char *str)
{
	for (; *str && (' ' != *str && '\t' != *str && '\n' != *str); str++);
	return str;
}

int InitExtensions(const char *origReqExts)
{
	// Length of requested extensions string
	unsigned reqExtsLen;
	char *reqExts;
	// Ptr for individual extensions within reqExts
	char *reqExt;
	int success = TRUE;

	// build space-padded extension string
	static char *sysExts = NULL;
	if (NULL == sysExts) {
		const char *extensions = (const char*)glGetString(GL_EXTENSIONS);
		int sysExtsLen = strlen(extensions);
		// Add 2 bytes, one for padding space, one for terminating NULL
		sysExts = (char*)malloc(sysExtsLen + 2);
		CHECK_MEMORY(sysExts);
		strcpy(sysExts, extensions);
		sysExts[sysExtsLen] = ' ';
		sysExts[sysExtsLen + 1] = 0;
	}

	if (NULL == origReqExts)
		return TRUE;
	reqExts = strdup(origReqExts);
	reqExtsLen = strlen(reqExts);
	if (NULL == unsupportedExts) {
		unsupportedExts = malloc(reqExtsLen + 1);
	} else if (reqExtsLen > strlen(unsupportedExts)) {
		unsupportedExts = realloc(unsupportedExts, reqExtsLen + 1);
	}
	CHECK_MEMORY(unsupportedExts);
	*unsupportedExts = 0;

	// Parse requested extension list
	for (reqExt = reqExts;
		(reqExt = (char*)EatWhiteSpace(reqExt)) && *reqExt;
		reqExt = (char*)EatNonWhiteSpace(reqExt))
	{
		char *extEnd = (char*)EatNonWhiteSpace(reqExt);
		char saveChar = *extEnd;
		*extEnd = (char)0;
		if (!ExtensionExists(reqExt, sysExts) ||
			!InitExtension(reqExt)) {
			// add reqExt to end of unsupportedExts
			strcat(unsupportedExts, reqExt);
			strcat(unsupportedExts, " ");
			success = FALSE;
		}
		*extEnd = saveChar;
	}
	free(reqExts);
	return success;
}

const char* GetUnsupportedExtensions()
{
	return (const char*)unsupportedExts;
}
