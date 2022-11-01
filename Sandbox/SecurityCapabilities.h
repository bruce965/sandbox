#pragma once

#include <memory>

#include <Windows.h>

class SecurityCapabilities
{
public:
	static const size_t size = sizeof(SECURITY_CAPABILITIES);

private:
	SECURITY_CAPABILITIES capabilities;
	std::unique_ptr<SID_AND_ATTRIBUTES[]> sidAndAttributes;
	std::unique_ptr<int8_t[]> sids;

public:
	SecurityCapabilities(PSID containerSid, std::initializer_list<WELL_KNOWN_SID_TYPE> sidTypes);
	virtual ~SecurityCapabilities();

	LPSECURITY_CAPABILITIES get();
};
