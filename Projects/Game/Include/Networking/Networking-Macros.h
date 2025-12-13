#pragma once

#define DO_REP(Class, Member) {OutReplicatedProperties.push_back({this, offsetof(Class, Member), sizeof(Member)});}

#define REGISTER_RPC(Class, FunctionName)\
OutRpcProps.push_back({#FunctionName, [](Actor* Actor)\
{\
	if (Class* ClassPtr = dynamic_cast<Class*>(Actor))\
	{\
		ClassPtr->FunctionName();\
	}\
}});