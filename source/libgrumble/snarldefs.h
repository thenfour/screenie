#ifndef LIBGRUMBLE_SNARLDEFS_H
#define LIBGRUMBLE_SNARLDEFS_H

namespace Snarl
{

	static const int SNARL_STRING_LENGTH = 1024;

	enum SNARL_COMMANDS
	{
		SNARL_SHOW = 1,
		SNARL_HIDE,
		SNARL_UPDATE,
		SNARL_IS_VISIBLE,
		SNARL_GET_VERSION,
		SNARL_REGISTER_CONFIG_WINDOW,
		SNARL_REVOKE_CONFIG_WINDOW,
		SNARL_REGISTER_ALERT,
		SNARL_REVOKE_ALERT,
		SNARL_REGISTER_CONFIG_WINDOW_2,
		SNARL_GET_VERSION_EX,
		SNARL_EX_SHOW = 0x20,

		// Grumble stuff
		SNARL_GET_GRUMBLE_INFO = 0x573
	};

	struct SNARLSTRUCT
	{
		SNARL_COMMANDS cmd;
		long id;
		long timeout;
		long lngData2;
		char title[SNARL_STRING_LENGTH];
		char text[SNARL_STRING_LENGTH];
		char icon[SNARL_STRING_LENGTH];
	};

	struct SNARLSTRUCTEX
	{
		SNARL_COMMANDS cmd;
		long id;
		long timeout;
		long lngData2;
		char title[SNARL_STRING_LENGTH];
		char text[SNARL_STRING_LENGTH];
		char icon[SNARL_STRING_LENGTH];
		char snarlClass[SNARL_STRING_LENGTH];
		char extra[SNARL_STRING_LENGTH];
		char extra2[SNARL_STRING_LENGTH];
		long reserved1;
		long reserved2;
	};

}

#endif