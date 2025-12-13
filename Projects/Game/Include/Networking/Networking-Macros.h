#pragma once

#define DO_REP(Actor, Member) {OutReplicatedProperties.push_back({this, offsetof(Actor, Member), sizeof(Member)});}