#include <stdio.h>
#include "plugin.h"

std::string g_sEndingMapName;
bool g_bUseHostname;

MMSPlugin g_ThisPlugin;
PLUGIN_EXPOSE(MMSPlugin, g_ThisPlugin);

MMSPlugin::MMSPlugin() : m_Serialize(&INetworkMessages::Serialize, this, &MMSPlugin::Hook_NetSerialize, nullptr)
{
}

bool MMSPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkMessages, INetworkMessages, NETWORKMESSAGES_INTERFACE_VERSION);

	g_SMAPI->AddListener( this, this );

	m_Serialize.Add(g_pNetworkMessages);

	const char* pConfigPath = "addons/configs/change_scoreboard_mapname.ini";

	KeyValues::AutoDelete pKeyValues("Change Scoreboard Mapname");
	if (!pKeyValues->LoadFromFile(g_pFullFileSystem, pConfigPath))
	{
		g_SMAPI->Format(error, maxlen, "[%s] Failed to load %s\n", GetLogTag(), pConfigPath);
		return false;
	}

	g_bUseHostname = pKeyValues->GetBool("use_hostname");
	if (!g_bUseHostname)
		g_sEndingMapName = pKeyValues->GetString("map_ending");

	return true;
}

KHook::Return<bool> MMSPlugin::Hook_NetSerialize(INetworkMessages *, bf_write &pBuf, const CNetMessage *pData)
{
	NetMessageInfo_t *pMessageInfo = pData->GetNetMessage()->GetNetMessageInfo();

	if (pMessageInfo->m_MessageId == svc_ClearAllStringTables)
	{
		CNetMessagePB<CSVCMsg_ClearAllStringTables> *pMsg = const_cast<CNetMessage*>(pData)->ToPB<CSVCMsg_ClearAllStringTables>();
		
		if (g_bUseHostname)
			pMsg->set_mapname(pMsg->mapname() + " | " + GetHostname());
		else
			pMsg->set_mapname(pMsg->mapname() + g_sEndingMapName);
	}

	return { KHook::Action::Ignore };
}


std::string MMSPlugin::GetHostname()
{
	static ConVarRefAbstract cvHostname("hostname");
	if (cvHostname.IsValidRef())
		return cvHostname.GetString().String();
	return "";
}

bool MMSPlugin::Unload(char *error, size_t maxlen)
{
	m_Serialize.Remove(g_pNetworkMessages);

	return true;
}