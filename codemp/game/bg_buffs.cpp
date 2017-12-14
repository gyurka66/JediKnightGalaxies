#include "bg_buffs.h"
#ifdef _GAME
#include "g_local.h"
#elif defined (IN_UI)
#include "ui/ui_local.h"
#else
#include "cgame/cg_local.h"
#endif
#include "json/cJSON.h"

#define MAX_FILEBUFFER_SIZE	32768	// 32kb
#define MAX_BUFFER_ERRORLEN	1024	// 1kb
#define BUFF_FILENAME	"ext_data/tables/buffs.json"

jkgBuff_t buffLookupTable[MAX_BUFF_COUNT] = { 0 };
static int lastBuffUsed = 0;

// Converts a string to a PMOVE_TYPE
pmtype_t JKG_StringToPMoveType(const char* string)
{
	if (!Q_stricmp(string, "PM_NORMAL"))
	{
		return PM_NORMAL;
	}
	else if (!Q_stricmp(string, "PM_JETPACK"))
	{
		return PM_JETPACK;
	}
	else if (!Q_stricmp(string, "PM_FLOAT"))
	{
		return PM_FLOAT;
	}
	else if (!Q_stricmp(string, "PM_NOCLIP"))
	{
		return PM_NOCLIP;
	}
	else if (!Q_stricmp(string, "PM_SPECTATOR"))
	{
		return PM_SPECTATOR;
	}
	else if (!Q_stricmp(string, "PM_DEAD"))
	{
		return PM_DEAD;
	}
	else if (!Q_stricmp(string, "PM_FREEZE"))
	{
		return PM_FREEZE;
	}
	else if (!Q_stricmp(string, "PM_INTERMISSION"))
	{
		return PM_INTERMISSION;
	}
	else if (!Q_stricmp(string, "PM_SPINTERMISSION"))
	{
		return PM_SPINTERMISSION;
	}
	else if (!Q_stricmp(string, "PM_LOCK"))
	{
		return PM_LOCK;
	}
	else if (!Q_stricmp(string, "PM_NOMOVE"))
	{
		return PM_NOMOVE;
	}
	return PM_NORMAL;
}

// Parses a single buff in the file
void JKG_ParseBuff(const char* name, cJSON* root)
{
	jkgBuff_t* buff = &buffLookupTable[lastBuffUsed++];
	Q_strncpyz(buff->internalName, name, sizeof(buff->internalName));

	cJSON* jsonNode = cJSON_GetObjectItem(root, "category");
	cJSON* child;
	if (jsonNode == nullptr)
	{
		buff->category[0] = '\0';
	}
	else
	{
		Q_strncpyz(buff->category, cJSON_ToString(jsonNode), sizeof(buff->category));
	}

	jsonNode = cJSON_GetObjectItem(root, "damage");
	if (jsonNode)
	{
		child = cJSON_GetObjectItem(jsonNode, "damage");
		buff->damage.damage = cJSON_ToIntegerOpt(child, 0);

		child = cJSON_GetObjectItem(jsonNode, "tick");
		buff->damage.tick = cJSON_ToIntegerOpt(child, 500);

		child = cJSON_GetObjectItem(jsonNode, "means");
		buff->damage.means = JKG_GetMeansOfDamageIndex(cJSON_ToStringOpt(child, "MOD_UNKNOWN"));
	}

	jsonNode = cJSON_GetObjectItem(root, "visuals");
	if (jsonNode)
	{
		child = cJSON_GetObjectItem(jsonNode, "screenShader");
		Q_strncpyz(buff->visuals.screenShader, cJSON_ToStringOpt(child, ""), MAX_QPATH);

		child = cJSON_GetObjectItem(jsonNode, "applySound");
		Q_strncpyz(buff->visuals.applySound, cJSON_ToStringOpt(child, ""), MAX_QPATH);

		child = cJSON_GetObjectItem(jsonNode, "cancelSound");
		Q_strncpyz(buff->visuals.cancelSound, cJSON_ToStringOpt(child, ""), MAX_QPATH);

		child = cJSON_GetObjectItem(jsonNode, "fadeSound");
		Q_strncpyz(buff->visuals.fadeSound, cJSON_ToStringOpt(child, ""), MAX_QPATH);

		child = cJSON_GetObjectItem(jsonNode, "bodyShader");
		Q_strncpyz(buff->visuals.bodyShader, cJSON_ToStringOpt(child, ""), MAX_QPATH);

		child = cJSON_GetObjectItem(jsonNode, "boltedOnEffects");
		if (child)
		{
			for (int i = 0; i < cJSON_GetArraySize(child); i++)
			{
				cJSON* arrayItem = cJSON_GetArrayItem(child, i);
				cJSON* arrayChild = cJSON_GetObjectItem(arrayItem, "effectName");
				jkgBuffEffectData_t effect;

				Q_strncpyz(effect.effectName, cJSON_ToStringOpt(arrayChild, ""), MAX_QPATH);
				
				arrayChild = cJSON_GetObjectItem(arrayItem, "effectBolt");
				Q_strncpyz(effect.effectBolt, cJSON_ToStringOpt(arrayChild, ""), MAX_QPATH);

				arrayChild = cJSON_GetObjectItem(arrayItem, "tick");
				effect.tick = cJSON_ToIntegerOpt(arrayChild, 100);
			}
		}
	}

	jsonNode = cJSON_GetObjectItem(root, "canceling");
	if (jsonNode)
	{
		child = cJSON_GetObjectItem(jsonNode, "noStack");
		buff->canceling.noBuffStack = cJSON_ToBooleanOpt(child, qfalse);

		child = cJSON_GetObjectItem(jsonNode, "priority");
		buff->canceling.priority = cJSON_ToIntegerOpt(child, 0);

		child = cJSON_GetObjectItem(jsonNode, "extraCancel");
		if (child)
		{
			for (int i = 0; i < cJSON_GetArraySize(child); i++)
			{
				cJSON* arrayItem = cJSON_GetArrayItem(child, i);
				cJSON* arrayChild = cJSON_GetObjectItem(arrayItem, "category");
				jkgBuffCancelCategory_t category;

				Q_strncpyz(category.category, cJSON_ToStringOpt(arrayChild, ""), sizeof(category.category));

				arrayChild = cJSON_GetObjectItem(arrayItem, "priority");
				category.priority = cJSON_ToIntegerOpt(arrayChild, 0);

				buff->canceling.extraCancel.push_back(category);
			}
		}
	}

	jsonNode = cJSON_GetObjectItem(root, "passive");
	if (jsonNode)
	{
		child = cJSON_GetObjectItem(jsonNode, "movementModifier");
		buff->passive.movementSpeedModifier = cJSON_ToNumberOpt(child, 1.0);

		child = cJSON_GetObjectItem(jsonNode, "pmoveType");
		const char* pmoveType = cJSON_ToStringOpt(child, "PM_NORMAL");
		buff->passive.pmoveType = JKG_StringToPMoveType(pmoveType);

		child = cJSON_GetObjectItem(jsonNode, "meansModifiers");
		if (child)
		{
			for (cJSON* it = cJSON_GetFirstItem(child); it; it = cJSON_GetNextItem(it))
			{
				const char* name = cJSON_GetItemKey(it);
				jkgBuffMeanModifier_t modifier;

				modifier.means = JKG_GetMeansOfDamageIndex(name);
				modifier.modifier = cJSON_ToNumberOpt(it, 1.0);

				buff->passive.meansModifiers.push_back(modifier);
			}
		}
	}
}

// Parses the buffs file
qboolean JKG_ParseBuffs(char* buffBuffer)
{
	char error[MAX_BUFFER_ERRORLEN]{ 0 };
	cJSON* json = cJSON_ParsePooled(buffBuffer, error, MAX_BUFFER_ERRORLEN);
	cJSON* it;

	if (!json)
	{
		Com_Printf(S_COLOR_RED "Couldn't parse " BUFF_FILENAME " - %s\n", error);
		return qfalse;
	}

	for (it = cJSON_GetFirstItem(json); it; it = cJSON_GetNextItem(it))
	{
		const char* name = cJSON_GetItemKey(it);
		JKG_ParseBuff(name, it);
	}

	return qtrue;
}

// Loads all of the buffs from the buffs.json
void JKG_InitBuffs()
{
	fileHandle_t f;
	int fileLen = trap->FS_Open(BUFF_FILENAME, &f, FS_READ);
	char buffer[MAX_FILEBUFFER_SIZE]{ 0 };

	if (!f || f == -1 || fileLen == 0 || fileLen == -1)
	{
		Com_Printf(S_COLOR_RED "Could not open " BUFF_FILENAME " for reading");
		return;
	}
	else if (fileLen >= MAX_FILEBUFFER_SIZE)
	{
		trap->FS_Close(f);
		Com_Printf(S_COLOR_RED "The buff file is too large (%i > %i).", fileLen, MAX_FILEBUFFER_SIZE);
		return;
	}

	trap->FS_Read(buffer, fileLen, f);
	buffer[fileLen] = '\0';
	trap->FS_Close(f);

	if (!JKG_ParseBuffs(buffer))
	{
		Com_Printf("Couldn't parse " BUFF_FILENAME);
	}
}