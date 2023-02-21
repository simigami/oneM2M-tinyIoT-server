#ifndef __ONEM2M_TYPES_H__
#define __ONEM2M_TYPES_H__

#include <stdbool.h>


typedef enum {
    RSC_ACCEPTED									= 1000,
	RSC_ACCEPTED_NONBLOCKING_REQUEST_SYNCH			= 1001,
	RSC_ACCEPTED_NONBLOCKING_REQUEST_ASYNCH			= 1002,
	RSC_OK											= 2000,
	RSC_CREATED 									= 2001,
	RSC_DELETED 									= 2002,
	RSC_UPDATED										= 2004,
	RSC_BAD_REQUEST									= 4000,
	RSC_RELEASE_VERSION_NOT_SUPPORTED				= 4001,
	RSC_NOT_FOUND 									= 4004,
	RSC_OPERATION_NOT_ALLOWED						= 4005,
	RSC_REQUEST_TIMEOUT 							= 4008,
	RSC_UNSUPPORTED_MEDIATYPE						= 4015,
	RSC_SUBSCRIPTION_CREATOR_HAS_NO_PRIVILEGE		= 4101,
	RSC_CONTENTS_UNACCEPTABLE						= 4102,
	RSC_ORIGINATOR_HAS_NO_PRIVILEGE					= 4103,
	RSC_CONFLICT									= 4105,
	RSC_SECURITY_ASSOCIATION_REQUIRED				= 4107,
	RSC_INVALID_CHILD_RESOURCETYPE					= 4108,
	RSC_GROUPMEMBER_TYPE_INCONSISTENT				= 4110,
	RSC_ORIGINATOR_HAS_ALREADY_REGISTERD			= 4117,
	RSC_APP_RULE_VALIDATION_FAILED					= 4126,
	RSC_OPERATION_DENIED_BY_REMOTE_ENTITY			= 4127,
	RSC_SERVICE_SUBSCRIPTION_NOT_ESTABLISHED		= 4128,
	RSC_INVALID_SPARQL_QUERY						= 4143,
	RSC_INTERNAL_SERVER_ERROR						= 5000,
	RSC_NOT_IMPLEMENTED								= 5001,
	RSC_TARGET_NOT_REACHABLE						= 5103,
	RSC_RECEIVER_HAS_NO_PRIVILEGES 					= 5105,
	RSC_ALREADY_EXISTS								= 5106,
	RSC_REMOTE_ENTITY_NOT_REACHABLE					= 5107,
	RSC_TARGET_NOT_SUBSCRIBABLE						= 5203,
	RSC_SUBSCRIPTION_VERIFICATION_INITIATION_FAILED	= 5204,
	RSC_SUBSCRIPTION_HOST_HAS_NO_PRIVILEGE			= 5205,
	RSC_NOT_ACCEPTABLE 								= 5207,
	RSC_CROSS_RESOURCE_OPERATION_FAILURE			= 5221,
	RSC_MAX_NUMBER_OF_MEMBER_EXCEEDED				= 6010,
	RSC_INVALID_ARGUMENTS							= 6023,
	RSC_INSUFFICIENT_ARGUMENTS						= 6024,
}ResponseStatusCode;


typedef enum {
	RT_MIXED = 0,
	RT_ACP = 1,
	RT_AE = 2,
	RT_CNT = 3,
	RT_CIN = 4,
	RT_CSE_BASE = 5,
	RT_GRP = 9,
	RT_MGMTOBJ = 13,
	RT_NOD = 14,
	RT_PCH = 15,
	RT_CSR = 16,
	RT_REQ = 17,
	RT_SUB = 23,
	RT_SMD = 24,
	RT_FCNT = 28,
	RT_TS = 29,
	RT_TSI = 30,
	RT_CRS = 48,
	RT_FCI = 58,
	RT_TSB = 60,
	RT_ACTR = 63,
} ResourceType;

#endif