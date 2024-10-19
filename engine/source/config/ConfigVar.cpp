#include "ConfigVar.h"

#include "ConfigSystem.h"

template<typename T>
inline ConfigVar<T>::ConfigVar(std::string_view _path, const T& defaultVal)
	: path(_path)
	, cachedVal(defaultVal)
{
}

template<typename T>
const T& ConfigVar<T>::QueryVal() const
{
	if (isCached) {
		return cachedVal;
	}
	ConfigSystem* cfgSys = ConfigSystem::Instance();
	assert(cfgSys);
	cachedVal = cfgSys->GetValue(path, cachedVal);
	isCached = true;
	return cachedVal;
}

#define ALLOW_CFG_VAR_TYPE(TYPE)    \
	template class ConfigVar<TYPE>; \
	template class ConfigVar<std::vector<TYPE>>

ALLOW_CFG_VAR_TYPE(bool);
ALLOW_CFG_VAR_TYPE(int);
ALLOW_CFG_VAR_TYPE(float);
ALLOW_CFG_VAR_TYPE(std::string_view);
