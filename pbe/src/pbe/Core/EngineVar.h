#pragma once

class EngineVar
{
public:

    virtual ~EngineVar() {}

protected:
    EngineVar( void ) {}
    EngineVar( const std::string& path ) {}
};

class BoolVar : public EngineVar
{
public:
    BoolVar( const std::string& path, bool val ) : EngineVar(path), m_Flag(val) {}
    BoolVar& operator=( bool val ) { m_Flag = val; return *this; }
    operator bool() const { return m_Flag; }

private:
    bool m_Flag;
};

class NumVar : public EngineVar
{
public:
    NumVar( const std::string& path, float val, float minValue = -FLT_MAX, float maxValue = FLT_MAX, float stepSize = 1.0f ) : EngineVar(path) {}
    NumVar& operator=( float val ) { m_Value = Clamp(val); return *this; }
    operator float() const { return m_Value; }


protected:
    float Clamp( float val ) { return val > m_MaxValue ? m_MaxValue : val < m_MinValue ? m_MinValue : val; }

    float m_Value;
    float m_MinValue;
    float m_MaxValue;
    float m_StepSize;
};

class ExpVar : public NumVar
{
public:
    ExpVar( const std::string& path, float val, float minExp = -FLT_MAX, float maxExp = FLT_MAX, float expStepSize = 1.0f ) : NumVar(path, 0, 0, 0) {}
    ExpVar& operator=( float val );    // m_Value = log2(val)
    operator float() const;            // returns exp2(m_Value)
};

class IntVar : public EngineVar
{
public:
    IntVar( const std::string& path, int32_t val, int32_t minValue = 0, int32_t maxValue = (1 << 24) - 1, int32_t stepSize = 1 ) : EngineVar(path) {}
    IntVar& operator=( int32_t val ) { m_Value = Clamp(val); return *this; }
    operator int32_t() const { return m_Value; }

protected:
    int32_t Clamp( int32_t val ) { return val > m_MaxValue ? m_MaxValue : val < m_MinValue ? m_MinValue : val; }

    int32_t m_Value;
    int32_t m_MinValue;
    int32_t m_MaxValue;
    int32_t m_StepSize;
};

class EnumVar : public EngineVar
{
public:
    EnumVar( const std::string& path, int32_t initialVal, int32_t listLength, const char** listLabels ) : EngineVar(path) {}
    EnumVar& operator=( int32_t val ) { m_Value = Clamp(val); return *this; }
    operator int32_t() const { return m_Value; }

    void SetListLength(int32_t listLength) { m_EnumLength = listLength; m_Value = Clamp(m_Value); }

private:
    int32_t Clamp( int32_t val ) { return val < 0 ? 0 : val >= m_EnumLength ? m_EnumLength - 1 : val; }

    int32_t m_Value;
    int32_t m_EnumLength;
    const char** m_EnumLabels;
};

class CallbackTrigger : public EngineVar
{
public:
    CallbackTrigger( const std::string& path, std::function<void (void*)> callback, void* args = nullptr ) : EngineVar(path) {}

private:
    std::function<void (void*)> m_Callback;
    void* m_Arguments;
    mutable uint32_t m_BangDisplay;
};

