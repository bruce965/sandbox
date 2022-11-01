#include "AclHelper.h"

#include <string>

#include <Windows.h>
#include <AccCtrl.h>
#include <Aclapi.h>

bool AclHelper::GrantPermissions(PSID sid, std::wstring objectName, SE_OBJECT_TYPE objectType, DWORD permissions)
{
	PACL oldAcl = NULL;
	PACL newAcl = NULL;
	PSECURITY_DESCRIPTOR securityDescriptor = NULL;

	bool success = false;

	do
	{
		EXPLICIT_ACCESS_W explicitAccess = { 0 };
		explicitAccess.grfAccessMode = GRANT_ACCESS;
		explicitAccess.grfAccessPermissions = permissions;
		explicitAccess.grfInheritance = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
		explicitAccess.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
		explicitAccess.Trustee.pMultipleTrustee = NULL;
		explicitAccess.Trustee.ptstrName = (LPWCH)sid;
		explicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		explicitAccess.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;

		DWORD status = GetNamedSecurityInfoW(objectName.c_str(), objectType, DACL_SECURITY_INFORMATION, NULL, NULL, &oldAcl, NULL, &securityDescriptor);
		if (status != ERROR_SUCCESS)
		{
			//printf("GetNamedSecurityInfoW() failed for %wZ, error: %d\n", objectName.c_str(), status);
			break;
		}

		status = SetEntriesInAclW(1, &explicitAccess, oldAcl, &newAcl);
		if (status != ERROR_SUCCESS)
		{
			//printf("SetEntriesInAclW() failed for %wZ, error: %d\n", objectName.c_str(), status);
			break;
		}

		status = SetNamedSecurityInfoW((LPWSTR)objectName.data(), objectType, DACL_SECURITY_INFORMATION, NULL, NULL, newAcl, NULL);
		if (status != ERROR_SUCCESS)
		{
			//printf("SetNamedSecurityInfoW() failed for %wZ, error: %d\n", objectName.c_str(), status);
			break;
		}

		success = true;

	} while (false);

	if (oldAcl)
		LocalFree(oldAcl);

	if (newAcl)
		LocalFree(newAcl);

	if (securityDescriptor)
		LocalFree(securityDescriptor);

	return success;
}
