#ifndef GRUMBLESUPPORT_H
#define GRUMBLESUPPORT_H

#include "..\libgrumble\snarlevent.h"

class GrumbleClient : public Snarl::SimpleClient
{
public:
	GrumbleClient() { }
protected:
	void OnMessageClicked(Snarl::MessageID id, ClickType clicktype) { }
	void OnMessageTimeout(Snarl::MessageID id) { }
};

extern GrumbleClient Grumble;

#endif