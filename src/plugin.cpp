#include <stdio.h>
#include "plugin.h"

SH_DECL_HOOK2(INetworkMessages, Serialize, SH_NOATTRIB, 0, bool, bf_write &, const CNetMessage*);

MMSPlugin g_ThisPlugin;
PLUGIN_EXPOSE(MMSPlugin, g_ThisPlugin);

std::string g_sEndingMapName;

bool MMSPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkMessages, INetworkMessages, NETWORKMESSAGES_INTERFACE_VERSION);

	SH_ADD_HOOK(INetworkMessages, Serialize, g_pNetworkMessages, SH_MEMBER(this, &MMSPlugin::Serialize), false);

	g_SMAPI->AddListener( this, this );

	const char* pConfigPath = "addons/configs/change_scoreboard_mapname.ini";

	KeyValues::AutoDelete pKeyValues("Change Scoreboard Mapname");
	if (!pKeyValues->LoadFromFile(g_pFullFileSystem, pConfigPath))
	{
		g_SMAPI->Format(error, maxlen, "[%s] Failed to load %s\n", GetLogTag(), pConfigPath);
		return false;
	}

	g_sEndingMapName = pKeyValues->GetString("map_ending");

	return true;
}

bool MMSPlugin::Serialize(bf_write &pBuf, const CNetMessage *pData)
{
	if (pData == nullptr)
		RETURN_META_VALUE(MRES_IGNORED, false);

	NetMessageInfo_t *pMessageInfo = pData->GetNetMessage()->GetNetMessageInfo();

	if (pMessageInfo && pMessageInfo->m_MessageId == svc_ClearAllStringTables)
	{
		CNetMessagePB<CSVCMsg_ClearAllStringTables> *pMsg = const_cast<CNetMessage*>(pData)->ToPB<CSVCMsg_ClearAllStringTables>();

		if (pMsg)
			pMsg->set_mapname(pMsg->mapname() + g_sEndingMapName.c_str());
	}

	RETURN_META_VALUE(MRES_IGNORED, true);
}

bool MMSPlugin::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(INetworkMessages, Serialize, g_pNetworkMessages, SH_MEMBER(this, &MMSPlugin::Serialize), false);

	return true;
}