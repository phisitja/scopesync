/**
 * Wrapper class that hosts the ScopeSync object and its GUI
 * on behalf of the Scope (or Scope SDK) applications, using 
 * the ScopeFX SDK. This requires it to be derived from the Effect
 * class.
 *
 * Also operates as a Timer source for the ScopeSync and ScopeFXGUI
 * objects.
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
#include "ScopeFX.h"
#include "ScopeFXGUI.h"
#include "../../ScopeSyncShared/Resources/ImageLoader.h"
#include "../../ScopeSyncShared/Resources/Icons.h"
#include "../../ScopeSyncShared/Core/ScopeSyncApplication.h"
#include "../../ScopeSyncShared/Windows/UserSettings.h"

const int ScopeFX::initPositionX           = 100;
const int ScopeFX::initPositionY           = 100;
const int ScopeFX::windowHandlerDelayMax   = 6;
const int ScopeFX::timerFrequency          = 20;

#ifdef _WIN32
#include <Windows.h>
#endif
#include <float.h>

#ifdef _WIN32
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)  
HWND scopeWindow;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM /* lParam */)
{
    HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);

    if((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE) == hinst && IsWindowVisible(hwnd))
    {
        scopeWindow = hwnd;
        return FALSE;
    }
    else
        return TRUE;
}
#endif

using namespace ScopeFXParameterDefinitions;

ScopeFX::ScopeFX() : Effect(&effectDescription)
{
    initValues();
    
    if (ScopeSync::getNumScopeSyncInstances() == 0)
    {
#ifdef _WIN32
        Process::setCurrentModuleInstanceHandle(HINST_THISCOMPONENT);
#endif
        initialiseJuce_GUI();
    }

    scopeSync = new ScopeSync(this);

    DBG("ScopeFX::ScopeFX - Number of module instances: " + String(ScopeSync::getNumScopeSyncInstances()));

    startTimer(timerFrequency);
}

ScopeFX::~ScopeFX()
{
    stopTimer();
    
    scopeFXGUI = nullptr;
    scopeSync->unload();
    scopeSync = nullptr;

    DBG("ScopeFX::~ScopeFX - Number of module instances: " + String(ScopeSync::getNumScopeSyncInstances()));
    
    ScopeSync::shutDownIfLastInstance();
}
   
void ScopeFX::initValues()
{
	positionX           = initPositionX;
	positionY           = initPositionY;
    requestWindowShow   = false;
    windowShown         = false;
    windowHandlerDelay  = 0;
    configurationUID    = 0;
	oscUID              = 0;
	tmpPerfModeOffCount = 0;
}

void ScopeFX::timerCallback()
{
    if (scopeFXGUI)
        scopeFXGUI->refreshWindow();

    if (windowHandlerDelay == 0)
    {   
		if ((requestWindowShow || scopeSync->getSystemError().toString().isNotEmpty()) && !windowShown)
        {
            DBG("ScopeFX::timerCallback - Request to show window");
            showWindow();
        }
        else if (!requestWindowShow && windowShown)
        {
            DBG("ScopeFX::timerCallback - Request to hide window");
            hideWindow();
        }

        //DBG("ScopeFX::timerCallback: " + String(positionX) + ", " + String(positionY) + ", " + String(scopeFXGUI->getScreenPosition().getX()) + ", " + String(scopeFXGUI->getScreenPosition().getY()));
        if (scopeFXGUI != nullptr
            && (positionX != scopeFXGUI->getScreenPosition().getX()
            ||  positionY != scopeFXGUI->getScreenPosition().getY()))
        {
            scopeFXGUI->setTopLeftPosition(positionX, positionY);
        }
    }
    else
    {
        DBG("ScopeFX::timerCallback - Ignoring values: " + String(windowHandlerDelay));
        windowHandlerDelay--;
    }
    
    if (!(scopeSync->processConfigurationChange()))
    {
        scopeSync->receiveUpdates();
    }
}

void ScopeFX::showWindow()
{
    DBG("ScopeFX::showWindow");
    scopeFXGUI = new ScopeFXGUI(this);

#ifdef _WIN32
    if (scopeWindow == nullptr)
    {
        EnumWindows(EnumWindowsProc, 0);
    }
#else
    // If Scope ever ends up on non-Windows, we'll
    // probably want to implement something here
    void* scopeWindow = nullptr;
#endif
    scopeFXGUI->setOpaque(true);
    scopeFXGUI->setVisible(true);
    scopeFXGUI->setName("ScopeSync");

    scopeFXGUI->setTopLeftPosition(positionX, positionY);
    scopeFXGUI->addToDesktop(ComponentPeer::windowHasTitleBar | ComponentPeer::windowHasCloseButton | ComponentPeer::windowHasDropShadow, scopeWindow);
    windowShown = true;
    windowHandlerDelay = windowHandlerDelayMax;
}

void ScopeFX::positionChanged(int newPosX, int newPosY)
{
    DBG("ScopeFX::positionChanged - moving window to: " + String(newPosX) + "," + String(newPosY));
    positionX = newPosX;
    positionY = newPosY;
    windowHandlerDelay = windowHandlerDelayMax;
}

void ScopeFX::setGUIEnabled(bool shouldBeEnabled)
{
    if (scopeFXGUI != nullptr)
        scopeFXGUI->setEnabled(shouldBeEnabled);
}

void ScopeFX::hideWindow()
{
    scopeFXGUI = nullptr;
    windowShown = false;
    windowHandlerDelay = windowHandlerDelayMax;
}

int ScopeFX::async(PadData** asyncIn,  PadData* /*syncIn*/,
                   PadData*  asyncOut, PadData* /*syncOut*/)
{
	// If the Preset Name has changed, set Performance Mode off for this async cycle
	String newPresetName = asyncIn[INPAD_PRESET_NAME]->str;
	
	if (newPresetName != presetName)
	{
		presetName = newPresetName;
		ScopeSyncApplication::setPerformanceMode(0);
		tmpPerfModeOffCount = 5;
	}

	int* parameterArray = (int*)asyncIn[INPAD_PARAMS]->itg;
    int* localArray     = (int*)asyncIn[INPAD_LOCALS]->itg;

	int asyncValues[ScopeSyncApplication::numScopeSyncParameters + ScopeSyncApplication::numScopeLocalParameters];

	// Grab ScopeSync values from input
	if (tmpPerfModeOffCount == 0 && parameterArray != nullptr)
	{
		for (int i = 0; i < ScopeSyncApplication::numScopeSyncParameters; i++)
			asyncValues[i] = parameterArray[i];
	}
	else
	{
		for (int i = 0; i < ScopeSyncApplication::numScopeSyncParameters; i++)
            asyncValues[i] = 0;
	}

	// Grab ScopeLocal values from input
	if (localArray != nullptr)
	{
		for (int i = 0; i < ScopeSyncApplication::numScopeLocalParameters; i++)
			asyncValues[i + ScopeSyncApplication::numScopeSyncParameters] = localArray[i];
	}
	else
	{
        for (int i = 0; i < ScopeSyncApplication::numScopeLocalParameters; i++)
            asyncValues[i + ScopeSyncApplication::numScopeSyncParameters] = 0;
	}

	// Get ScopeSync to process the inputs and pass on changes from the SS system
	scopeSync->handleScopeSyncAsyncUpdate(asyncValues);

	// Write to the async outputs for the ScopeSync and ScopeLocal parameters
	for (int i = 0; i < ScopeSyncApplication::numScopeSyncParameters + ScopeSyncApplication::numScopeLocalParameters; i++)
		asyncOut[i].itg = asyncValues[i];

	// Deal with showing/hiding the Control Panel window
    if (asyncIn[INPAD_SHOW]->itg == 0)
        requestWindowShow = false;
    else
        requestWindowShow = true;

	// Handle configuration changes (project/preset load)
    int newConfigurationUID = asyncIn[INPAD_CONFIGUID]->itg;

    if (newConfigurationUID != configurationUID)
    {
        scopeSync->changeConfiguration(newConfigurationUID);
        configurationUID = newConfigurationUID;
    }

	// Handle OSC UID updates
    int newOSCUID = asyncIn[INPAD_OSCUID]->itg;

    if (newOSCUID != oscUID)
    {
        scopeSync->setOSCUID(newOSCUID);
        oscUID = newOSCUID;
    }

	if (tmpPerfModeOffCount > 1)
	{
		DBG("ScopeFX::async POINT 1, performance mode set to: " + String(ScopeSyncApplication::getPerformanceMode()));

		String asyncValueString = String(asyncValues[0]);

		for (int i = 1; i < 128; i++)
			asyncValueString += ", " + String(asyncValues[i]);

		DBG("ScopeFX::async POINT 1, input values: " + asyncValueString);

		// Leave the performance mode alone for now
		asyncOut[OUTPAD_PERFORMANCE_MODE].itg = performanceMode;
		tmpPerfModeOffCount--;
	}
	else if (tmpPerfModeOffCount == 1)
	{
		// Set performance mode back to what it was before
		ScopeSyncApplication::setPerformanceMode(performanceMode);
		asyncOut[OUTPAD_PERFORMANCE_MODE].itg = performanceMode;
		tmpPerfModeOffCount--;
		scopeSync->reinitialiseScopeParameters();
		DBG("ScopeFX::async POINT 2, performance mode set to: " + String(ScopeSyncApplication::getPerformanceMode()));
	}
	else
	{
		// Switch Performance Mode on/off
		int newPMSetting = asyncIn[INPAD_PERFORMANCE_MODE]->itg;
		
		if (newPMSetting != performanceMode)
		{
			ScopeSyncApplication::setPerformanceMode(newPMSetting);
			performanceMode = newPMSetting;
			DBG("ScopeFX::async POINT 3, performance mode set to: " + String(ScopeSyncApplication::getPerformanceMode()));
		}

		asyncOut[OUTPAD_PERFORMANCE_MODE].itg = (scopeSync  != nullptr) ? ScopeSyncApplication::getPerformanceMode() : performanceMode;
	}
	
	// Handle window position updates
	positionX = asyncIn[INPAD_X]->itg;
    positionY = asyncIn[INPAD_Y]->itg;
    
    asyncOut[OUTPAD_SHOW].itg             = windowShown ? 1 : 0;
    asyncOut[OUTPAD_X].itg                = (scopeFXGUI != nullptr) ? scopeFXGUI->getScreenPosition().getX()     : positionX;
    asyncOut[OUTPAD_Y].itg                = (scopeFXGUI != nullptr) ? scopeFXGUI->getScreenPosition().getY()     : positionY;
    asyncOut[OUTPAD_CONFIGUID].itg        = (scopeSync  != nullptr) ? scopeSync->getConfigurationUID()           : configurationUID;
	asyncOut[OUTPAD_OSCUID].itg           = (scopeSync  != nullptr) ? scopeSync->getOSCUID()                     : oscUID;
      
    return 0;
}

int ScopeFX::syncBlock(PadData** /*asyncIn*/, PadData* /*syncIn*/,
                       PadData* /*asyncOut*/, PadData* /*syncOut*/, 
                       int /*off*/,
                       int /*cnt*/)
{
	return -1;
}

Effect *newEffect()
{
	return new ScopeFX();
};

void deleteEffect (Effect *eff)
{
   delete eff;
}

EffectDescription *descEffect ()
{
    return &effectDescription;
}

extern "C"
::uint32 ioctlEffect (
					  ::uint32  /* dwService */, ::uint32  /* dwDDB */, ::uint32  /* hDevice */,
					  void * /* lpvInBuffer */, ::uint32 /* cbInBuffer */, 
					  void * /* lpvOutBuffer */, ::uint32 /* cbOutBuffer */,
					  ::uint32 * lpcbBytesReturned )
{
   *lpcbBytesReturned = 0;
   return ERROR_NOT_SUPPORTED;
}
