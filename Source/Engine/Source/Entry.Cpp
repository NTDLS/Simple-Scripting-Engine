#ifndef _ENTRY_CPP_
#define _ENTRY_CPP_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <ConIO.H>
#include <IO.H>
#include <fcntl.h>

#include "CEngine/CEngine.H"
#include "../Dialogs/NetLogoDlg.H"
#include "../Resources/Resource.H"
#include "../../../Shared/NSWFL/NSWFL.h"

#include "Entry.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::File;
using namespace NSWFL::String;
using namespace NSWFL::CommandLine;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char gsAppCopyRight[64];
char gsFileVersion[32];
char gsTitleCaption[64];
HINSTANCE ghAppInstance;

void ProcessConsole(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int __cdecl main(void)
{
	ghAppInstance = GetModuleHandle(NULL);
	CommandLineParser CmdLine;

	const int iResult = 0;
	int iNonHandledParamCount = 0;
	bool bPause = false;

	LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
	LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
	LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));


	if (CmdLine.Count() == 2 && _stricmp(CmdLine.Item(1), "/Console") == 0)
	{
		ProcessConsole();
		return 0;
	}

	CEngine Engine;

	//Parse options:
	for (int iArg = 1; iArg < CmdLine.Count(); iArg++)
	{
		if (_strcmpi(CmdLine.Item(iArg), "/Pause") == 0)
		{
			bPause = true;
		}
		else if (_strcmpi(CmdLine.Item(iArg), "/Ver") == 0)
		{
			_AboutDialogInfo ADI;

			ADI.DisplayIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
			ADI.TitleCaption = gsTitleCaption;
			ADI.FileVersion = gsFileVersion;
			ADI.BuildDate = __DATE__;
			ADI.BuildTime = __TIME__;
			ADI.CopyRight = gsAppCopyRight;
			ADI.OwnerHandle = NULL;

			NetLogo(&ADI);
		}
		else if (_strnicmp(CmdLine.Item(iArg), "/DebugNoSTDRedirect:", 10) == 0)
		{
			Engine.IntegrateWithDebugger(CmdLine.Item(iArg) + 17);
			Engine.Debug->UseSTDRedirection = false;
		}
		else if (_strnicmp(CmdLine.Item(iArg), "/Debug:", 7) == 0)
		{
			Engine.IntegrateWithDebugger(CmdLine.Item(iArg) + 7);
		}
		else if (_strcmpi(CmdLine.Item(iArg), "/Debug") == 0)
		{
			Engine.IntegrateWithDebugger();
		}
		else if (_strcmpi(CmdLine.Item(iArg), "/NoValidate") == 0)
		{
			//This will significantly improve script processing time.
			Engine.ValidateCodeBeforeExecution = false;
		}
		else {
			iNonHandledParamCount++;
		}
	}

	//If no script was specified, then we will derrive it from the environment variables.
	//	This is because modern versions of IIS nolonger send us script path info via the command line.
	if (iNonHandledParamCount == 0)
	{
		char *sPathTranslated = NULL;
		char *sScriptName = NULL;
		size_t iScriptNameLen = 0;
		size_t iPathTranslatedLen = 0;

		if (_dupenv_s(&sScriptName, &iScriptNameLen, "SCRIPT_NAME") == 0 && sScriptName && strlen(sScriptName) > 0)
		{
			if (_dupenv_s(&sPathTranslated, &iPathTranslatedLen, "PATH_TRANSLATED") == 0 && sPathTranslated && strlen(sPathTranslated) > 0)
			{
				CorrectReversePath(sScriptName, iScriptNameLen);
				CorrectReversePath(sPathTranslated, iPathTranslatedLen);

				int iFoundPos = InStr(sScriptName, sPathTranslated);
				if (iFoundPos)
				{
					//Un-translate the "Path Translated"
					sPathTranslated[iFoundPos + strlen(sScriptName)] = '\0';
				}

				Engine.Scripts->AddFile(sPathTranslated);
				iNonHandledParamCount++;

				free(sPathTranslated);
			}

			free(sScriptName);
		}
	}

	//Parse scripts:
	if (iNonHandledParamCount > 0)
	{
		int iResult = 0;

		if (!Engine.Error->HardCount())
		{
			for (int iArg = 1; iArg < CmdLine.Count(); iArg++)
			{
				if (_strcmpi(CmdLine.Item(iArg), "/Ver") == 0)
				{
					//Skip.
				}
				else if (_strcmpi(CmdLine.Item(iArg), "/Pause") == 0)
				{
					//Skip.
				}
				else if (_strnicmp(CmdLine.Item(iArg), "/Debug", 6) == 0)
				{
					//Skip.
				}
				else if (_strcmpi(CmdLine.Item(iArg), "/NoValidate") == 0)
				{
					//Skip.
				}
				else {
					Engine.CommandLine->Push(CmdLine.Item(iArg), CmdLine.Length(iArg));

					//We only add the first script to the collection - all subsequent parameters are command line.
					if (Engine.Scripts->Collection.Count == 0)
					{
						Engine.Scripts->AddFile(CmdLine.Item(iArg));
					}
				}
			}

			//We only want to start the debugging routines if no errors have occured, but
			//	if the IDE is attached, then we need to start debugging so that we can
			//	report the errors to the attached debugger.
			if (Engine.Error->HardCount() == 0 || (Engine.Debug && Engine.Debug->WaitOnIDE))
			{
				if (Engine.Debug)
				{
					Engine.Debug->Start();
				}
			}

			if (Engine.Error->HardCount() == 0)
			{
				Engine.Process();
			}

			iResult = Engine.Error->HardCount();

			Engine.Error->Throw(Engine.ThrowSoftExceptions);
		}
	}

	if (bPause)
	{
		system("pause");
	}

	return iResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcessConsole(void)
{
	CEngine engine;

	while (true)
	{
		char sLine[255];
		fgets(sLine, sizeof(sLine) - 1, stdin);
		engine.Scripts->AddString(sLine); //Add a line to the execution pipeline.

		engine.Process(); //Run the next line in the pipeline.

		//Handle any errors that might come up:

		ErrorLevel result = engine.Error->HardCount();
		engine.Error->Throw(engine.ThrowSoftExceptions);

		if (result != ERROR_OK)
		{
			engine.AdvanceLine(); //Skip the offending line.
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
