#ifndef LIBGRUMBLE_SNARL_H
#define LIBGRUMBLE_SNARL_H

namespace Snarl
{

	typedef long MessageID;

	//
	// ShowMessage()
	// Show a notification message.
	//
	// title - the message title
	// text - the message text (the long one)
	// icon - path to an icon for this message. ICO, PNG, JPG, GIF, or BMP formats are supported by both Snarl and Grumble.
	// timeout - the number of seconds to leave the message on screen. Grumble ignores this.
	// hWndReply - the window to send uReplyMsg to after the message popup has been clicked or it has timed out
	// uReplyMsg - the message to send to hWndReply; see above
	// soundpath - path to a sound for this message; use NULL for none. Grumble ignores this.
	// msgclass - the 'class' of your notification; use NULL for none. the same value you would pass to RegisterAlert()
	//
	// Returns the unique integer ID of the popup message.
	//

	MessageID ShowMessage(const wchar_t* title, const wchar_t* text, long timeout, const wchar_t* path, HWND hWndReply, UINT uReplyMsg,
		const wchar_t* soundpath = 0, const wchar_t* msgclass = 0);

	//
	// UpdateMessage()
	// Change the properties of an existing popup message.
	//
	// id - the ID of the existing message
	// title - 
	// text - 
	// iconpath - 
	//
	// Returns success.
	//

	bool UpdateMessage(MessageID id, const wchar_t* title, const wchar_t* text, const wchar_t* iconpath);

	//
	// HideMessage()
	// Hides an existing popup message.
	//
	// id - the ID of the existing message
	//
	// Returns success.
	//

	bool HideMessage(MessageID id);

	//
	// HideMessage()
	// Hides an existing popup message.
	//
	// id - the ID of the existing message
	//
	// Returns success.
	//

	bool IsMessageVisible(MessageID id);

	//
	// RegisterConfig()
	// Register your application with Snarl/Grumble.
	//
	// hWndApp - the window handle of *your* application. If you don't have one, use NULL.
	// appname - the name of your application
	// uReplyMsg - the message to be sent to hWndApp
	// icon - path to an icon for your application
	//
	// Returns success.
	//

	bool RegisterConfig(HWND hWndApp, const wchar_t* appname, UINT uReplyMsg, const wchar_t* icon);

	//
	// RevokeConfig()
	// Properly unregisters your application with Snarl/Grumble.
	//
	// hWndApp - the window handle of your application. should be the same value passed to RegisterConfig
	//
	// Returns success.
	//

	bool RevokeConfig(HWND hWndApp);

	//
	// RegisterAlert()
	// Registers an alert with Snarl/Grumble. This way, the user can selectively enable/disable the alerts.
	//
	// appname - the name of your application. This should be the same as in RegisterConfig()
	// msgclass - a description of the 'type' of this message
	//
	// Returns success.
	//

	bool RegisterAlert(const wchar_t* appname, const wchar_t* msgclass);

	//
	// GetSnarlWindow()
	// Gets the handle of the Snarl (or Grumble) window.
	//
	// Returns the window handle.
	//

	HWND GetSnarlWindow();

	//////////////////////////////////////////////////////////////////////////

	enum ServiceType
	{
		ServiceTypeNone,
		ServiceTypeSnarl,
		ServiceTypeGrumble
	};

	//
	// GetCurrentServiceType()
	// Determines whether Snarl, Grumble, or neither, is running.
	//
	// Returns one of ServiceType
	//

	ServiceType GetCurrentServiceType();

}

#endif