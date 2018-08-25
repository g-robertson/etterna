#include "global.h"
#include "Difficulty.h"
#include "GameInput.h"
#include "MsdFile.h"
#include "NoteData.h"
#include "NotesLoader.h"
#include "NotesLoaderOSU.h"
#include "PrefsManager.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageUtil_CharConversions.h"
#include "Song.h"
#include "Steps.h"

#include <map>


void OsuLoader::ParseFileString(string fileContents, Song &out)
{
	vector<string> tags;
	vector<vector<string>> contents;

	SeparateTagsAndContents(fileContents, tags, contents);
}

void OsuLoader::SeparateTagsAndContents(string fileContents, vector<string> &tagsOut, vector<vector<string>> &contentsOut)
{
	char lastByte = '\0';
	bool isComment = false;
	bool isTag = false;
	bool isContent = false;
	string tag = "";
	string content = "";

	int tagIndex = 0;

	for (int i = 0; i < (int)fileContents.length(); ++i)
	{
		char currentByte = fileContents[i];

		if (isComment)
		{
			if (currentByte == '\n')
			{
				isComment = false;
			}
		}
		else if (currentByte == '/')
		{
			if (lastByte == '/')
			{
				isComment = true;
			}
		}
		else if (currentByte == '[')
		{
			tag = "";
			isTag = true;
		}
		else if (isTag)
		{
			if (currentByte == ']')
			{
				++tagIndex;
				tagsOut[tagIndex] = tag;
				isTag = false;
				content = "";
				contentsOut[tagIndex] = vector<string>();
				isContent = true;
			}
			else
			{
				tag = tag + currentByte;
			}
		}
		else if (isContent)
		{
			if (currentByte == '[' || i == (int)fileContents.length())
			{
				contentsOut[tagIndex].emplace_back(content);
				content = "";
				isContent = false;
				tag = "";
				isTag = true;
			}
			else if (currentByte == '\n')
			{
				contentsOut[tagIndex].emplace_back(content);
				content = "";
			}
			else
			{
				content = content + currentByte;
			}
		}
		lastByte = currentByte;
	}
}

void OsuLoader::GetApplicableFiles(const RString &sPath, vector<RString> &out)
{
	GetDirListing(sPath + RString("*.osu"), out);
}

bool OsuLoader::LoadNoteDataFromSimfile(const RString &path, Steps &out)
{
	RageFile f;
	if (!f.Open(path))
	{
		LOG->UserLog("Song file", path, "couldn't be opened: %s", f.GetError().c_str());
		return false;
	}

	RString FileRStr;
	FileRStr.reserve(f.GetFileSize());

	int iBytesRead = f.Read(FileRStr);
	string FileStr = FileRStr.c_str();







	return false;
}

bool OsuLoader::LoadFromDir(const RString &sPath_, Song &out)
{
	vector<RString> aFileNames;
	GetApplicableFiles(sPath_, aFileNames);

	if (aFileNames.size() > 1)
	{
		LOG->UserLog("Song", sPath_, "has more than one OSU file. There should be only one!");
		return false;
	}

	/* We should have exactly one; if we had none, we shouldn't have been called to begin with. */
	ASSERT(aFileNames.size() == 1);
	const RString sPath = sPath_ + aFileNames[0];

	LOG->Trace("Song::LoadFromDWIFile(%s)", sPath.c_str()); //osu

	RageFile f;
	if (!f.Open(sPath))
	{
		LOG->UserLog("Song file", sPath, "couldn't be opened: %s", f.GetError().c_str());
		return false;
	}

	RString fileStr;
	f.Read(fileStr, -1);


	ParseFileString(fileStr.c_str(), out);

	out.m_sSongFileName = sPath;



	out.m_sMusicFile = "asdf";

	out.m_sMainTitle = "maintitestasdf'";
	out.m_sSubTitle = "subtitest";
	out.m_sArtist = "artest";
	out.m_sGenre = "genretest";
	out.m_sCDTitleFile = "cdtitest";
	out.m_SongTiming.AddSegment(BPMSegment(0, 100));
	out.m_DisplayBPMType = DISPLAY_BPM_SPECIFIED;
	out.m_fSpecifiedBPMMin = 1;
	out.m_fSpecifiedBPMMax = 100;
	out.m_SongTiming.m_fBeat0OffsetInSeconds = -StringToInt("000") / 1000.0f;
	out.m_fMusicSampleStartSeconds = 1.0;
	out.m_fMusicSampleLengthSeconds = 100;
	out.m_SongTiming.AddSegment(StopSegment(0, 0));
	out.m_SongTiming.AddSegment(BPMSegment(10, 200));

	auto chart = out.CreateSteps();

	chart->SetFilename(sPath);

	chart->m_StepsType = StepsType_dance_single;

	chart->SetMeter(StringToInt("69"));

	chart->SetDifficulty(Difficulty_Beginner);


	NoteData nd;
	nd.Init();
	nd.SetNumTracks(4);
	nd.SetTapNote(0, 0, TAP_ORIGINAL_TAP);
	nd.SetTapNote(0, 20, TAP_ORIGINAL_TAP);
	chart->SetNoteData(nd);

	chart->TidyUpData();

	chart->SetSavedToDisk(true);

	out.m_SongTiming.AddSegment(BPMSegment(0, 160.0));
	chart->m_Timing.AddSegment(BPMSegment(0, 160.0));

	out.AddSteps(chart);


	ConvertString(out.m_sMainTitle, "utf-8,english");
	ConvertString(out.m_sSubTitle, "utf-8,english");

	return true;
}
