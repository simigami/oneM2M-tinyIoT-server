#ifndef __ONEM2M_H__
#define __ONEM2M_H__
#endif
#include <stdbool.h>


typedef enum {
    accepted									= 1000,
	acceptedNonBlockingRequestSynch				= 1001,
	acceptedNonBlockingRequestAsynch			= 1002,
	OK											= 2000,
	created 									= 2001,
	deleted 									= 2002,
	updated										= 2004,
	badRequest									= 4000,
	releaseVersionNotSupported					= 4001,
	notFound 									= 4004,
	operationNotAllowed							= 4005,
	requestTimeout 								= 4008,
	unsupportedMediaType						= 4015,
	subscriptionCreatorHasNoPrivilege			= 4101,
	contentsUnacceptable						= 4102,
	originatorHasNoPrivilege					= 4103,
	conflict									= 4105,
	securityAssociationRequired					= 4107,
	invalidChildResourceType					= 4108,
	groupMemberTypeInconsistent					= 4110,
	originatorHasAlreadyRegistered				= 4117,
	appRuleValidationFailed						= 4126,
	operationDeniedByRemoteEntity				= 4127,
	serviceSubscriptionNotEstablished			= 4128,
	invalidSPARQLQuery 							= 4143,
	internalServerError							= 5000,
	notImplemented								= 5001,
	targetNotReachable 							= 5103,
	receiverHasNoPrivileges						= 5105,
	alreadyExists								= 5106,
	remoteEntityNotReachable					= 5107,
	targetNotSubscribable						= 5203,
	subscriptionVerificationInitiationFailed	= 5204,
	subscriptionHostHasNoPrivilege				= 5205,
	notAcceptable 								= 5207,
	crossResourceOperationFailure 				= 5221,
	maxNumberOfMemberExceeded					= 6010,
	invalidArguments							= 6023,
	insufficientArguments						= 6024,
}ResponseStatusCode;