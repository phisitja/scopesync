/**
 * ScopeSync Class for interfacing with the Sonic|Core Scope Async
 * messaging system. Used within the scopefx AppContext.
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

#ifndef SCOPESYNCASYNC_H_INCLUDED
#define SCOPESYNCASYNC_H_INCLUDED

class ScopeSync;

#include <JuceHeader.h>
#include "../ScopeFX/ScopeFXParameterDefinitions.h"
#include <atomic>

class ScopeSyncAsync {

public:
    
    ScopeSyncAsync();
    ~ScopeSyncAsync();

    // Processes an incoming array of Async values coming from Scope and passes
    // on any updates from the ScopeSync system
    void handleUpdate(int* asyncValues, int* prevValues);
    
    // Passes on the contents of the queue of updates received from Scope
    void getAsyncUpdates(HashMap<int, int, DefaultHashFunctions, CriticalSection>& targetHashMap);

	// Grab snapshot of current values into the hashmap
	void snapshot();
	
    // Add a new control value change to the queue for processing in the next
    // batch of Async updates
    void setValue(int scopeCodeId, int newValue);
    void setValue(const String& scopeCode, int newValue);

    void toggleUpdateProcessing(bool processThem);
    
	static void setScopeInputEnablement(bool newValue) { enableScopeInputs = newValue; }

private:
    bool initialiseScopeParameters; // All Scope Parameters are set from Async the first time we receive an update

    std::atomic_int  currentValues[ScopeFXParameterDefinitions::numParameters];
    std::atomic_bool processUpdates;

	static bool enableScopeInputs;

    HashMap<int, int, DefaultHashFunctions, CriticalSection> asyncUpdates;     // Updates received from the async input

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopeSyncAsync);
};

#endif  // SCOPESYNCASYNC_H_INCLUDED
