//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.5
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstparameters.h
// Created by  : Steinberg, 03/2008
// Description : VST Parameter Implementation
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2015, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// This Software Development Kit may not be distributed in parts or its entirety
// without prior written agreement by Steinberg Media Technologies GmbH.
// This SDK must not be used to re-engineer or manipulate any technology used
// in any Steinberg or Third-party application or software module,
// unless permitted by law.
// Neither the name of the Steinberg Media Technologies nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SDK IS PROVIDED BY STEINBERG MEDIA TECHNOLOGIES GMBH "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL STEINBERG MEDIA TECHNOLOGIES GMBH BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#pragma once

#include "base/source/tarray.h"
#include "base/source/tdictionary.h"
#include "base/source/fobject.h"
#include "base/source/fcontainer.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstunits.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {


//------------------------------------------------------------------------
/** Description of a Parameter.
\ingroup vstClasses */
//------------------------------------------------------------------------
class Parameter: public FObject
{
public:
//------------------------------------------------------------------------
	Parameter ();
	Parameter (const ParameterInfo&);
	Parameter (const TChar* title, ParamID tag, const TChar* units = 0,
			   ParamValue defaultValueNormalized = 0., int32 stepCount = 0,
			   int32 flags = ParameterInfo::kCanAutomate, UnitID unitID = kRootUnitId);
	virtual ~Parameter ();

	/** Returns its readonly info. */
	const ParameterInfo& getInfo () const {return info;}

	/** Returns its writeable info. */
	ParameterInfo& getInfo () {return info;}

	/** Sets its associated UnitId. */
	void setUnitID (UnitID id) {info.unitId = id;}
	/** Gets its associated UnitId. */
	UnitID getUnitID () {return info.unitId;}

	/** Gets its normalized value [0.0, 1.0]. */
	ParamValue getNormalized () const { return valueNormalized; }
	/** Sets its normalized value [0.0, 1.0]. */
	virtual bool setNormalized (ParamValue v);

	/** Converts a normalized value to a string. */
	virtual void toString (ParamValue valueNormalized, String128 string) const;
	/** Converts a string to a normalized value. */
	virtual bool fromString (const TChar* string, ParamValue& valueNormalized) const;

	/** Converts a normalized value to plain value (e.g. 0.5 to 10000.0Hz). */
	virtual ParamValue toPlain (ParamValue valueNormalized) const;
	/** Converts a plain value to a normalized value (e.g. 10000 to 0.5). */
	virtual ParamValue toNormalized (ParamValue plainValue) const;

	/** Gets the current precision (used for string representation of float). */
	int32 getPrecision () const { return precision;}
	/** Sets the precision for string representation of float value (for example 4.34 with 2 as precision). */
	void setPrecision (int32 val) { precision = val;}

	OBJ_METHODS (Parameter, FObject)
//------------------------------------------------------------------------
protected:
	ParameterInfo info;
	ParamValue valueNormalized;
	int32 precision;
};

//------------------------------------------------------------------------
/** Description of a RangeParameter.
\ingroup vstClasses */
//------------------------------------------------------------------------
class RangeParameter : public Parameter
{
public:
//------------------------------------------------------------------------
	RangeParameter (const ParameterInfo& paramInfo, ParamValue min, ParamValue max);
	RangeParameter (const TChar* title, ParamID tag, const TChar* units = 0,
					ParamValue minPlain = 0., ParamValue maxPlain = 1., ParamValue defaultValuePlain = 0.,
					int32 stepCount = 0, int32 flags = ParameterInfo::kCanAutomate, UnitID unitID = kRootUnitId);

	/** Gets the minimum plain value, same as toPlain (0). */
	virtual ParamValue getMin () const {return minPlain;}
	/** Sets the minimum plain value. */
	virtual void setMin (ParamValue value) {minPlain = value;}
	/** Gets the maximum plain value, same as toPlain (1). */
	virtual ParamValue getMax () const {return maxPlain;}
	/** Sets the maximum plain value. */
	virtual void setMax (ParamValue value) {maxPlain = value;}

	/** Converts a normalized value to a string. */
	virtual void toString (ParamValue _valueNormalized, String128 string) const;
	/** Converts a string to a normalized value. */
	virtual bool fromString (const TChar* string, ParamValue& _valueNormalized) const;

	/** Converts a normalized value to plain value (e.g. 0.5 to 10000.0Hz). */
	virtual ParamValue toPlain (ParamValue _valueNormalized) const;
	/** Converts a plain value to a normalized value (e.g. 10000 to 0.5). */
	virtual ParamValue toNormalized (ParamValue plainValue) const;

	OBJ_METHODS (RangeParameter, Parameter)
//------------------------------------------------------------------------
protected:
	RangeParameter ();

	ParamValue minPlain;
	ParamValue maxPlain;
};

//------------------------------------------------------------------------
/** Description of a StringListParameter.
\ingroup vstClasses */
//------------------------------------------------------------------------
class StringListParameter : public Parameter
{
public:
//------------------------------------------------------------------------
	StringListParameter (const ParameterInfo& paramInfo);
	StringListParameter (const TChar* title, ParamID tag, const TChar* units = 0,
						 int32 flags = ParameterInfo::kCanAutomate | ParameterInfo::kIsList, UnitID unitID = kRootUnitId);
	virtual ~StringListParameter ();

	/** Appends a string and increases the stepCount. */
	virtual void appendString (const String128 string);
	/** Replaces the string at index. Index must be between 0 and stepCount+1 */
	virtual bool replaceString (int32 index, const String128 string);

	/** Converts a normalized value to a string. */
	virtual void toString (ParamValue _valueNormalized, String128 string) const;
	/** Converts a string to a normalized value. */
	virtual bool fromString (const TChar* string, ParamValue& _valueNormalized) const;

	/** Converts a normalized value to plain value (e.g. 0.5 to 10000.0Hz). */
	virtual ParamValue toPlain (ParamValue _valueNormalized) const;
	/** Converts a plain value to a normalized value (e.g. 10000 to 0.5). */
	virtual ParamValue toNormalized (ParamValue plainValue) const;

	OBJ_METHODS (StringListParameter, Parameter)
//------------------------------------------------------------------------
protected:
	TArray<TChar*> strings;
};

//------------------------------------------------------------------------
/** Collection of parameters.
\ingroup vstClasses */
//------------------------------------------------------------------------
class ParameterContainer
{
public:
//------------------------------------------------------------------------
	ParameterContainer ();
	~ParameterContainer ();

	/** Init param array. */
	void init (int32 initialSize = 10, int32 resizeDelta = 100);

	/** Creates and adds a new parameter from a ParameterInfo. */
	Parameter* addParameter (const ParameterInfo& info);

	/** Creates and adds a new parameter with given properties. */
	Parameter* addParameter (const TChar* title, const TChar* units = 0,
							 int32 stepCount = 0,
							 ParamValue defaultValueNormalized = 0.,
							 int32 flags = ParameterInfo::kCanAutomate,
							 int32 tag = -1,
							 UnitID unitID = kRootUnitId);

	/** Adds a given parameter. */
	Parameter* addParameter (Parameter* p);

	/** Returns the count of parameters. */
	int32 getParameterCount () const { return params ? params->total () : 0; }

	/** Gets parameter by index. */
	Parameter* getParameterByIndex (int32 index) { return params ? params->at (index) : 0; }

	/** Removes all parameters. */
	void removeAll () { if (params) params->removeAll (); id2index.removeAll (); }

	/** Gets parameter by ID. */
	Parameter* getParameter (ParamID tag);

//------------------------------------------------------------------------
protected:
	TArray<IPtr<Parameter> >* params;
	THashDictionary<ParamID, int32> id2index;
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
