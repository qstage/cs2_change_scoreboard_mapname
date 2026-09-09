#ifndef _INCLUDE_METAMOD_SOURCE_PLUGIN_H_
#define _INCLUDE_METAMOD_SOURCE_PLUGIN_H_

#include <ISmmPlugin.h>
#include <igameevents.h>
#include "version_gen.h"
#include "netmessages.pb.h"
#include "networksystem/inetworkmessages.h"
#include "KeyValues.h"

class MMSPlugin : public ISmmPlugin, public IMetamodListener
{
public:
	MMSPlugin();
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	KHook::Return<bool> Hook_NetSerialize(INetworkMessages *, bf_write &pBuf, const CNetMessage *pData);
	std::string GetHostname();
public:
	const char *GetAuthor() { return PLUGIN_AUTHOR; }
	const char *GetName() { return PLUGIN_DISPLAY_NAME; }
	const char *GetDescription() { return PLUGIN_DESCRIPTION; }
	const char *GetURL() { return PLUGIN_URL; }
	const char *GetLicense() { return PLUGIN_LICENSE; }
	const char *GetVersion() { return PLUGIN_FULL_VERSION; }
	const char *GetDate() { return __DATE__; }
	const char *GetLogTag() { return PLUGIN_LOGTAG; }
protected:
	KHook::Virtual<INetworkMessages, bool, bf_write&, const CNetMessage*> m_Serialize;
};

extern MMSPlugin g_ThisPlugin;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_METAMOD_SOURCE_PLUGIN_H_
