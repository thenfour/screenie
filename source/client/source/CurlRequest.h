#ifndef CURLREQUEST_H
#define CURLREQUEST_H

#include <string>

#include <curl/curl.h>
#include <curl/easy.h>

#include <assert.h>

namespace Curl
{

	class CurlRequest
	{
	public:
		CurlRequest();
		virtual ~CurlRequest() { }

		void SetURL(const std::string& url) { m_url = url; }
		std::string GetURL() const { return m_url; }

		int GetErrorCode() const { return m_errorcode; }
		std::string GetErrorText() const { return m_errortext; }

		void SetUsername(const std::string& username) { m_username = username; }
		std::string GetUsername() const { return m_username; }

		void SetPassword(const std::string& password) { m_password = password; }
		std::string GetPassword() const { return m_password; }

		size_t GetUploadSize() const { return m_uploadSize; }
		double GetUploadSpeed() const { return m_uploadSpeed; }

		bool SetCancel()
		{
			assert(m_cancelstate == CancelStateNone);
			m_cancelstate = CancelStatePending;	
		}

		bool Perform();
	protected:
		enum CancelState
		{
			CancelStateNone,            // when there is no canceling going on
			CancelStatePending,         // when the user sets the cancel var
			CancelStateCanceling,       // when the progress function returns false
			CancelStateCanceled         // when the cancellation is complete
		};

		enum PerformState
		{
			PerformStateNone,           // the library is inactive
			PerformStatePerforming,     // the library is currently performing a request
			PerformStateSuccess,        // the library has performed a request
			PerformStateFailed          // the request failed
		};

		virtual bool OnPrePerform(CURL* curl) { return true; }
		virtual void OnPostPerform(CURL* curl, bool success) { }

		virtual long OnReadData(unsigned char* data, size_t length) = 0;
		virtual void OnWriteData(unsigned char* data, size_t length) = 0;

		virtual bool OnProgress(double dltotal, double dlnow, double ultotal, double ulnow, bool canceled) = 0;

		CancelState GetCancelState() const { return m_cancelstate; }
		PerformState GetPerformState() const { return m_performstate; }
	private:
		std::string m_url;
		std::string m_username;
		std::string m_password;
		CancelState m_cancelstate;
		PerformState m_performstate;
		int m_errorcode;
		std::string m_errortext;
		size_t m_uploadSize;
		double m_uploadSpeed;
	private:
		static size_t ReadCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
		static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
		static int ProgressCallback(void* userdata, double dltotal, double dlnow, double ultotal, double ulnow);
	};

} // namespace Curl

#endif