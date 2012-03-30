#include "DebugTools.h"

DebugTools::DebugTools(void)
{
}

DebugTools::~DebugTools(void)
{
}

void DebugTools::ShowError(RakString errorString,bool pause, unsigned int lineNum,const char *fileName)
{

	char pauseChar;
	fflush(stdin);

	printf("%s\nFile:%s \nLine: %i\n",errorString.C_String(),fileName,lineNum);

	if (pause)
	{
		printf("Press enter to continue \n");
		pauseChar=getchar();
	}
}
