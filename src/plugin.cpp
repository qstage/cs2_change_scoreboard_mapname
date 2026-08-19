#include <stdio.h>
#include "plugin.h"

#include "funchook.h"

std::string g_sEndingMapName;

typedef bool (FASTCALL *NetworkMessageSerialize_t)(INetworkMessages*, bf_write&, const CNetMessage*);

NetworkMessageSerialize_t g_pfnNetSerialize = nullptr;
funchook_t *g_pNetSerialize = nullptr;
constexpr int g_iNetSerializeOffset = 3;

bool FASTCALL Hook_NetSerialize(INetworkMessages *pNetworkMessages, bf_write &pBuf, const CNetMessage *pData)
{
	NetMessageInfo_t *pMessageInfo = pData->GetNetMessage()->GetNetMessageInfo();

	if (pMessageInfo->m_MessageId == svc_ClearAllStringTables)
	{
		CNetMessagePB<CSVCMsg_ClearAllStringTables> *pMsg = const_cast<CNetMessage*>(pData)->ToPB<CSVCMsg_ClearAllStringTables>();
		if (pMsg) pMsg->set_mapname(pMsg->mapname() + g_sEndingMapName.c_str());
	}

	return g_pfnNetSerialize(pNetworkMessages, pBuf, pData);
}


MMSPlugin g_ThisPlugin;
PLUGIN_EXPOSE(MMSPlugin, g_ThisPlugin);

bool MMSPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkMessages, INetworkMessages, NETWORKMESSAGES_INTERFACE_VERSION);

	void** pNetworkMessagesVtable = *(void***)g_pNetworkMessages;
	g_pfnNetSerialize = (NetworkMessageSerialize_t)pNetworkMessagesVtable[g_iNetSerializeOffset];

	g_pNetSerialize = funchook_create();
	funchook_prepare(g_pNetSerialize, (void **)&g_pfnNetSerialize, (void *)Hook_NetSerialize);
	funchook_install(g_pNetSerialize, 0);

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

bool MMSPlugin::Unload(char *error, size_t maxlen)
{
	if (g_pNetSerialize)
	{
		funchook_uninstall(g_pNetSerialize, 0);
		funchook_destroy(g_pNetSerialize);
	}

	return true;
}