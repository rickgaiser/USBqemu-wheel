#ifndef DEVICEPROXY_H
#define DEVICEPROXY_H
#include "configuration.h"
#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <iterator>
//#include <memory>
#include "helpers.h"
#include "proxybase.h"
#include "qemu-usb/USBinternal.h"

// also map key/array index
enum DeviceType
{
	DEVTYPE_NONE = -1,
	DEVTYPE_PAD = 0,
	DEVTYPE_MSD,
	DEVTYPE_SINGSTAR,
	DEVTYPE_LOGITECH_MIC,
	DEVTYPE_LOGITECH_HEADSET,
	DEVTYPE_HIDKBD,
	DEVTYPE_HIDMOUSE,
	DEVTYPE_RBKIT,
};

struct SelectDeviceName {
	template <typename S>
	std::string operator()(const std::pair<const DeviceType, S> &x) const { return x.second->TypeName(); }
};

class DeviceError : public std::runtime_error
{
	public:
	DeviceError(const char* msg) : std::runtime_error(msg) {}
	virtual ~DeviceError() throw () {}
};

class Device
{
	public:
	virtual ~Device() {}
};

class DeviceProxyBase
{
	public:
	DeviceProxyBase(DeviceType key);
	virtual ~DeviceProxyBase() {}
	virtual USBDevice* CreateDevice(int port) = 0;
	virtual const TCHAR* Name() const = 0;
	virtual const char* TypeName() const = 0;
	virtual int Configure(int port, const std::string& api, void *data) = 0;
	virtual std::list<std::string> ListAPIs() = 0;
	virtual const TCHAR* LongAPIName(const std::string& name) = 0;
	virtual int Freeze(int mode, USBDevice *dev, void *data) = 0;

	virtual bool IsValidAPI(const std::string& api)
	{
		const std::list<std::string>& apis = ListAPIs();
		auto it = std::find(apis.begin(), apis.end(), api);
		if (it != apis.end())
			return true;
		return false;
	}
};

template <class T>
class DeviceProxy : public DeviceProxyBase
{
	public:
	DeviceProxy(DeviceType key): DeviceProxyBase(key) {}
	virtual ~DeviceProxy() {}
	virtual USBDevice* CreateDevice(int port)
	{
		return T::CreateDevice(port);
	}
	virtual const TCHAR* Name() const
	{
		return T::Name();
	}
	virtual const char* TypeName() const
	{
		return T::TypeName();
	}
	virtual int Configure(int port, const std::string& api, void *data)
	{
		return T::Configure(port, api, data);
	}
	virtual std::list<std::string> ListAPIs()
	{
		return T::ListAPIs();
	}
	virtual const TCHAR* LongAPIName(const std::string& name)
	{
		return T::LongAPIName(name);
	}
	virtual int Freeze(int mode, USBDevice *dev, void *data)
	{
		return T::Freeze(mode, dev, data);
	}
};

class RegisterDevice
{
	RegisterDevice(const RegisterDevice&) = delete;
	RegisterDevice() {}

	public:
	typedef std::map<DeviceType, DeviceProxyBase* > RegisterDeviceMap;
	static RegisterDevice& instance() {
		static RegisterDevice registerDevice;
		return registerDevice;
	}

	~RegisterDevice() {}

	void Add(DeviceType key, DeviceProxyBase* creator)
	{
		registerDeviceMap[key] = creator;
	}

	DeviceProxyBase* Device(const std::string& name)
	{
		//return registerDeviceMap[name];
		/*for (auto& k : registerDeviceMap)
			if(k.first.name == name)
				return k.second;
		return nullptr;*/
		auto proxy = std::find_if(registerDeviceMap.begin(),
			registerDeviceMap.end(),
			[&name](RegisterDeviceMap::value_type val) -> bool
		{
			return val.second->TypeName() == name;
		});
		if (proxy != registerDeviceMap.end())
			return proxy->second;
		return nullptr;
	}

	DeviceProxyBase* Device(int index)
	{
		auto it = registerDeviceMap.begin();
		std::advance(it, index);
		if (it != registerDeviceMap.end())
			return it->second;
		return nullptr;
	}

	DeviceType Index(const std::string& name)
	{
		auto proxy = std::find_if(registerDeviceMap.begin(),
			registerDeviceMap.end(),
			[&name](RegisterDeviceMap::value_type val) -> bool
		{
			return val.second->TypeName() == name;
		});
		if (proxy != registerDeviceMap.end())
			return proxy->first;
		return DEVTYPE_NONE;
	}

	std::list<std::string> Names() const
	{
		std::list<std::string> nameList;
		std::transform(
			registerDeviceMap.begin(), registerDeviceMap.end(),
			std::back_inserter(nameList),
			SelectDeviceName());
		return nameList;
	}

	std::string Name(int index) const
	{
		auto it = registerDeviceMap.begin();
		std::advance(it, index);
		if (it != registerDeviceMap.end())
			return it->second->TypeName();
		return std::string();
	}

	const RegisterDeviceMap& Map() const
	{
		return registerDeviceMap;
	}

	private:
	RegisterDeviceMap registerDeviceMap;
};

#define REGISTER_DEVICE(idx,cls) DeviceProxy<cls> g##cls##Proxy(idx)
//#define REGISTER_DEVICE(idx,name,cls) static std::unique_ptr< DeviceProxy<cls> > g##cls##Proxy(new DeviceProxy<cls>(DeviceKey(idx, name)))
#endif
