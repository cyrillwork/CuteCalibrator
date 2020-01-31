#include "calibratorbuilder.hh"

CalibratorBuilder::CalibratorBuilder(const CalibratorBuilder& builder)
{
    device_name = builder.getDevice_name();
    axys = builder.getAxys();
    thr_misclick = builder.getThrMisclick();
    thr_doubleclick = builder.getThrDoubleclick();
    output_type = builder.getOutput_type();
    geometry = builder.getGeometry();
    use_timeout = builder.getUse_timeout();
    output_filename = builder.getOutput_filename();
    testMode = builder.getTestMode();
    lang = builder.getLang();
    small = builder.getSmall();
    touchID = builder.getTouchID();
    timeout = builder.getTimeout();
}

CalibratorBuilder* CalibratorBuilder::setThrMisclick(int value)
{
    this->thr_misclick = value;
    return this;
}

CalibratorBuilder* CalibratorBuilder::setThrDoubleclick(int value)
{
    this->thr_doubleclick = value;
    return this;
}

XYinfo CalibratorBuilder::getAxys() const
{
    return axys;
}

CalibratorBuilder* CalibratorBuilder::setAxys(const XYinfo&value)
{
    this->axys = value;
    return this;
}

const char*CalibratorBuilder::getDevice_name() const
{
    return device_name;
}

OutputType CalibratorBuilder::getOutput_type() const
{
    return output_type;
}

void CalibratorBuilder::setOutput_type(const OutputType&value)
{
    output_type = value;
}

char*CalibratorBuilder::getGeometry() const
{
    return geometry;
}

void CalibratorBuilder::setGeometry(char*value)
{
    geometry = value;
}

bool CalibratorBuilder::getUse_timeout() const
{
    return use_timeout;
}

CalibratorBuilder* CalibratorBuilder::setUse_timeout(bool value)
{
    this->use_timeout = value;
    return this;
}

char*CalibratorBuilder::getOutput_filename() const
{
    return output_filename;
}

void CalibratorBuilder::setOutput_filename(char*value)
{
    output_filename = value;
}

bool CalibratorBuilder::getTestMode() const
{
    return testMode;
}

void CalibratorBuilder::setTestMode(bool value)
{
    testMode = value;
}

Lang CalibratorBuilder::getLang() const
{
    return lang;
}

void CalibratorBuilder::setLang(const Lang&value)
{
    lang = value;
}

bool CalibratorBuilder::getSmall() const
{
    return small;
}

void CalibratorBuilder::setSmall(bool value)
{
    small = value;
}

void CalibratorBuilder::setDevice_name(const char*value)
{
    device_name = value;
}

XID CalibratorBuilder::getDevice_id() const
{
    return device_id;
}

void CalibratorBuilder::setDevice_id(const XID&value)
{
    device_id = value;
}

bool CalibratorBuilder::getTouchID() const
{
    return touchID;
}

void CalibratorBuilder::setTouchID(bool value)
{
    touchID = value;
}

int CalibratorBuilder::getTimeout() const
{
    return timeout;
}

void CalibratorBuilder::setTimeout(int value)
{
    timeout = value;
}

int CalibratorBuilder::getThrMisclick() const
{
    return thr_misclick;
}

int CalibratorBuilder::getThrDoubleclick() const
{
    return thr_doubleclick;
}
