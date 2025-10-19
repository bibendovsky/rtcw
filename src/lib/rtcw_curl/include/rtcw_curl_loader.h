#ifndef RTCW_CURL_LOADER_INCLUDED
#define RTCW_CURL_LOADER_INCLUDED

namespace rtcw {

class CurlLoader
{
public:
	CurlLoader() {}
	virtual ~CurlLoader() {}

	bool load_library();
	void unload_library();
	void* get_proc_address(const char* symbol_name);

private:
	virtual bool do_load_library() = 0;
	virtual void do_unload_library() = 0;
	virtual void* do_get_proc_address(const char* symbol_name) = 0;
};

// ======================================

extern CurlLoader* global_curl_loader;

} // namespace rtcw

#endif // RTCW_CURL_LOADER_INCLUDED
