/**
 * Models a Host or Scope parameter, including the handling
 * of its ValueTree and various values
 *
 *  (C) Copyright 2014 bcmodular (http://www.bcmodular.co.uk/)
 *
 * This file is part of ScopeSync.
 *
 * ScopeSync is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * ScopeSync is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ScopeSync.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contributors:
 *  Simon Russell
 *  Will Ellis
 *  Jessica Brandt
 */

#ifndef BCMPARAMETER_H_INCLUDED
#define BCMPARAMETER_H_INCLUDED

#include <JuceHeader.h>
class BCMParameterController;

#ifdef __DLL_EFFECT__
class ScopeSyncAsync;
#endif // __DLL_EFFECT__

class BCMParameter : public Value::Listener,
					 public Timer,
					 private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>
{
public:
    /* ============================ Enumerations ============================== */
    enum ParameterType         {none, scope, local, feedback, fixedBiDir, fixedInputOnly, preset};
    enum ParameterValueType    {continuous, discrete}; // Possible types of Parameter Value
	enum ParameterUpdateSource {internalUpdate, hostUpdate, guiUpdate, oscUpdate, midiUpdate, asyncUpdate};

	#ifdef __DLL_EFFECT__
		BCMParameter(ValueTree parameterDefinition, ParameterType parameterType, BCMParameterController& pc, ScopeSyncAsync& ssa, bool oscAble);
	#else
		BCMParameter(ValueTree parameterDefinition, ParameterType parameterType, BCMParameterController& pc, bool oscAble);
	#endif // __DLL_EFFECT__
    
	void initialise();
    ~BCMParameter();

    void beginParameterChangeGesture();
    void endParameterChangeGesture();
    
    void          mapToUIValue(Value& valueToMapTo) const;
    void          setAffectedByUI(bool isAffected);
    bool          isAffectedByUI() const;
    String        getName() const;
	void          setHostIdx(int newIndex) { hostIdx = newIndex; }
    int           getHostIdx() const { return hostIdx; }
    int           getScopeCodeId() const;
    String        getScopeCode() const;
    ParameterType getParameterType() const { return type; }
    ValueTree&    getDefinition() { return definition; }
    void          getSettings(ValueTree& settings) const;
    void          getDescriptions(String& shortDesc, String& fullDesc) const;
    void          getUIRanges(double& rangeMin, double& rangeMax, double& rangeInt, String& uiSuffix) const;
    void          getScopeRanges(int& min, int& max) const;
    double        getUIResetValue() const;
    double        getUISkewFactor() const;

    void          getUITextValue(String& textValue) const;
    int           getUIValue() const { return uiValue.getValue(); };

    float         getHostValue() const;
    int           getScopeIntValue() const;

    bool          isDiscrete() const;
    bool          isScopeInputOnly() const;
    
    void          setHostValue(float newValue);
    void          setScopeIntValue(int newValue);
    void          setUIValue(float newValue, bool updateHost = true);
	void          setOSCValue(float newValue);

	void          registerOSCListener();
	String        getOSCPath() const;
	void          sendOSCParameterUpdate() const;

private:
    WeakReference<BCMParameter>::Master masterReference;
    friend class WeakReference<BCMParameter>;

	void oscMessageReceived (const OSCMessage& message) override;

    /* ====================== Private Parameter Methods ======================= */
    // Either on initialisation, or after ranges have changed, this method will
    // ensure the UI and LinearNormalised values are within the ranges
    void   putValuesInRange(bool initialise);
    void   setNumDecimalPlaces();
    float  skewHostValue(float hostValue, bool invert) const;
    double dbSkew(double valueToSkew, double ref, double uiMinValue, double uiMaxValue, bool invert) const;
    double convertLinearNormalisedToUIValue(double lnValue) const;
	double convertUIToLinearNormalisedValue(double newValue) const;

    void  setParameterValues(ParameterUpdateSource updateSource, double newLinearNormalisedValue, double newUIValue, bool updateHost = true);
    int   findNearestParameterSetting(int value) const;

	void  valueChanged(Value& valueThatChanged) override;
	void  timerCallback() override;

	void  sendToScopeSyncAsync() const;

	void  decDeadTimes();
    
    /* ===================== Private member variables ========================= */
	BCMParameterController& parameterController;
#ifdef __DLL_EFFECT__
	ScopeSyncAsync&  scopeSyncAsync;
#endif // __DLL_EFFECT__

    bool             oscEnabled;
    ParameterType    type;
    ValueTree        definition;
    Value            uiValue;
    Value            linearNormalisedValue;
	Value            oscUID;
    bool             affectedByUI;
    int              hostIdx;
    int              scopeCodeId;
	int              numDecimalPlaces;
	
	int oscDeadTimeCounter;
	int asyncDeadTimeCounter;

	static const int deadTimeTimerInterval;
	static const int maxOSCDeadTime;
	static const int maxAsyncDeadTime;

};

#endif  // BCMPARAMETER_H_INCLUDED
