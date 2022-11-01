#include "SecurityCapabilities.h"

#include "SandboxException.h"

SecurityCapabilities::SecurityCapabilities(PSID containerSid, std::initializer_list<WELL_KNOWN_SID_TYPE> sidTypes)
{
	this->sidAndAttributes = std::unique_ptr<SID_AND_ATTRIBUTES[]>(new SID_AND_ATTRIBUTES[sidTypes.size()]());
	this->sids = std::unique_ptr<int8_t[]>(new int8_t[sidTypes.size() * SECURITY_MAX_SID_SIZE]());

	int index = 0;
	for (auto sidType : sidTypes)
	{
		auto sidAndAttribute = &this->sidAndAttributes[index];
		sidAndAttribute->Sid = &this->sids[index * SECURITY_MAX_SID_SIZE];
		sidAndAttribute->Attributes = SE_GROUP_ENABLED;

		index++;

		DWORD sidSize = SECURITY_MAX_SID_SIZE;
		if (!CreateWellKnownSid(sidType, NULL, sidAndAttribute->Sid, &sidSize))
		{
			throw new SandboxException("Failed to create well-known SID", GetLastError());
		}
	}

	this->capabilities = { 0 };
	this->capabilities.AppContainerSid = containerSid;
	this->capabilities.Capabilities = this->sidAndAttributes.get();
	this->capabilities.CapabilityCount = sidTypes.size();
}

SecurityCapabilities::~SecurityCapabilities()
{
	this->capabilities = { 0 };
}

LPSECURITY_CAPABILITIES SecurityCapabilities::get()
{
	return &this->capabilities;
}
